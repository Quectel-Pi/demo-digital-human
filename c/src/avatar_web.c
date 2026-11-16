/* Single HTTP worker, enabled only by av_start's DEMO030_WEB_ROOT branch. */
#include "avatar_web.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define WEB_REQUEST_MAX 8192
#define WEB_PATH_MAX 2048
#define WEB_IO_SECONDS 2

struct avatar_web_s {
    pthread_mutex_t lock;       /* snapshot, stopping, and client fd ownership */
    pthread_t thread;
    int listener, root, wake[2], client, stopping;
    avatar_web_state_t state;
};

static double web_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static int web_stopping(avatar_web_t *w)
{
    pthread_mutex_lock(&w->lock);
    int stop = w->stopping;
    pthread_mutex_unlock(&w->lock);
    return stop;
}

static int send_all(avatar_web_t *w, int fd, const void *buf, size_t n,
                    double deadline)
{
    const char *p = buf;
    while (n) {
        if (web_stopping(w) || web_now() >= deadline) return -1;
        ssize_t sent = send(fd, p, n, MSG_NOSIGNAL);
        if (sent < 0 && errno == EINTR) continue;
        if (sent <= 0) return -1; /* includes socket write timeout */
        p += sent;
        n -= (size_t)sent;
    }
    return 0;
}

static int send_header(avatar_web_t *w, int fd, int code, const char *reason,
                       const char *mime, unsigned long long length,
                       double deadline)
{
    char header[512];
    int n = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %llu\r\n"
        "Connection: close\r\nCache-Control: no-store\r\n"
        "X-Content-Type-Options: nosniff\r\n\r\n",
        code, reason, mime, length);
    if (n < 0 || (size_t)n >= sizeof(header)) return -1;
    return send_all(w, fd, header, (size_t)n, deadline);
}

static void send_error(avatar_web_t *w, int fd, int code, const char *reason,
                       double deadline)
{
    if (send_header(w, fd, code, reason, "text/plain; charset=utf-8",
                    strlen(reason), deadline) == 0)
        send_all(w, fd, reason, strlen(reason), deadline);
}

/* Invalid/truncated UTF-8 bytes are escaped too, keeping JSON valid even if
 * the original fixed-size subtitle buffer ended in the middle of a character. */
static size_t utf8_length(const unsigned char *p)
{
    unsigned c = p[0];
    size_t n = c >= 0xc2 && c <= 0xdf ? 2 :
               c >= 0xe0 && c <= 0xef ? 3 :
               c >= 0xf0 && c <= 0xf4 ? 4 : 0;
    for (size_t i = 1; i < n; ++i)
        if (!p[i] || (p[i] & 0xc0) != 0x80) return 0;
    if (n >= 3 && ((c == 0xe0 && p[1] < 0xa0) ||
                   (c == 0xed && p[1] >= 0xa0))) return 0;
    if (n == 4 && ((c == 0xf0 && p[1] < 0x90) ||
                  (c == 0xf4 && p[1] >= 0x90))) return 0;
    return n;
}

/* dst needs at most 6 * strlen(src) + 3 bytes (quotes + terminator). */
static void json_string(char *dst, const char *src)
{
    static const char hex[] = "0123456789abcdef";
    const unsigned char *p = (const unsigned char *)src;
    *dst++ = '"';
    while (*p) {
        unsigned c = *p;
        if (c == '"' || c == '\\') {
            *dst++ = '\\';
            *dst++ = (char)*p++;
        } else if (c < 0x20 || c >= 0x80) {
            size_t n = c >= 0x80 ? utf8_length(p) : 0;
            if (n) {
                memcpy(dst, p, n);
                dst += n;
                p += n;
            } else {
                memcpy(dst, "\\u00", 4);
                dst += 4;
                *dst++ = hex[c >> 4];
                *dst++ = hex[c & 15];
                ++p;
            }
        } else {
            *dst++ = (char)*p++;
        }
    }
    *dst++ = '"';
    *dst = 0;
}

static float unit_float(float f)
{
    return !isfinite(f) || f < 0.0f ? 0.0f : f > 1.0f ? 1.0f : f;
}

static void send_state(avatar_web_t *w, int fd, double deadline)
{
    static const char *names[] = { "idle", "listen", "think", "speak" };
    avatar_web_state_t s;
    pthread_mutex_lock(&w->lock);
    s = w->state;
    pthread_mutex_unlock(&w->lock);
    char q[sizeof(s.question) * 6 + 3], a[sizeof(s.answer) * 6 + 3];
    char body[sizeof(q) + sizeof(a) + 512];
    json_string(q, s.question);
    json_string(a, s.answer);
    unsigned st = (unsigned)s.state;
    /* Integer whole/fraction parts avoid locale-dependent decimal commas. */
    unsigned op = (unsigned)(unit_float(s.openness) * 10000.0f);
    unsigned wd = (unsigned)(unit_float(s.wide) * 10000.0f);
    unsigned rd = (unsigned)(unit_float(s.round_) * 10000.0f);
    unsigned mic = (unsigned)(unit_float(s.mic) * 10000.0f);
    int n = snprintf(body, sizeof(body),
        "{\"state\":\"%s\",\"openness\":%u.%04u,\"wide\":%u.%04u,"
        "\"round\":%u.%04u,\"mic\":%u.%04u,\"question\":%s,"
        "\"answer\":%s,\"seq\":%llu}",
        st < 4 ? names[st] : names[0], op / 10000, op % 10000,
        wd / 10000, wd % 10000, rd / 10000, rd % 10000,
        mic / 10000, mic % 10000, q, a, s.seq);
    if (n < 0 || (size_t)n >= sizeof(body)) {
        send_error(w, fd, 500, "Internal Server Error", deadline);
        return;
    }
    if (send_header(w, fd, 200, "OK", "application/json; charset=utf-8",
                    (unsigned long long)n, deadline) == 0)
        send_all(w, fd, body, (size_t)n, deadline);
}

static int hex_value(unsigned char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Decode exactly once, reject NUL, controls, backslashes and dot segments.
 * No further URL decoding is performed by the filesystem layer. */
static int decode_path(const char *url, char path[WEB_PATH_MAX])
{
    if (*url != '/') return -1;
    size_t n = 0;
    while (*url && *url != '?' && *url != '#') {
        unsigned char c = (unsigned char)*url++;
        if (c == '%') {
            if (!url[0] || !url[1]) return -1;
            int a = hex_value((unsigned char)url[0]);
            int b = hex_value((unsigned char)url[1]);
            if (a < 0 || b < 0) return -1;
            c = (unsigned char)((a << 4) | b);
            url += 2;
        }
        if (c < 0x20 || c == 0x7f || c == '\\' || n >= WEB_PATH_MAX - 1)
            return -1;
        path[n++] = (char)c;
    }
    path[n] = 0;
    const char *p = path;
    while (*p) {
        while (*p == '/') ++p;
        const char *begin = p;
        while (*p && *p != '/') ++p;
        size_t len = (size_t)(p - begin);
        if ((len == 1 && begin[0] == '.') ||
            (len == 2 && begin[0] == '.' && begin[1] == '.')) return -1;
    }
    return 0;
}

/* Walk from an already-open root directory; reject symlinks at EVERY level.
 * openat + O_NOFOLLOW avoids realpath/check/open TOCTOU escapes. */
static int open_static(avatar_web_t *w, const char *path)
{
    char copy[WEB_PATH_MAX];
    snprintf(copy, sizeof(copy), "%s", path);
    int dir = dup(w->root);
    if (dir < 0) return -1;
    char *save = NULL;
    char *part = strtok_r(copy, "/", &save);
    while (part) {
        char *next = strtok_r(NULL, "/", &save);
        int flags = O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK;
        if (next) flags |= O_DIRECTORY;
        int fd = openat(dir, part, flags);
        close(dir);
        if (fd < 0) return -1;
        dir = fd;
        part = next;
    }
    struct stat st;
    if (fstat(dir, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(dir);
        return -1;
    }
    return dir;
}

static const char *mime_type(const char *path)
{
    const char *e = strrchr(path, '.');
    if (!e) return "application/octet-stream";
    if (!strcasecmp(e, ".html") || !strcasecmp(e, ".htm"))
        return "text/html; charset=utf-8";
    if (!strcasecmp(e, ".js") || !strcasecmp(e, ".mjs"))
        return "text/javascript; charset=utf-8";
    if (!strcasecmp(e, ".css")) return "text/css; charset=utf-8";
    if (!strcasecmp(e, ".json") || !strcasecmp(e, ".map")) return "application/json";
    if (!strcasecmp(e, ".vrm") || !strcasecmp(e, ".glb")) return "model/gltf-binary";
    if (!strcasecmp(e, ".gltf")) return "model/gltf+json";
    if (!strcasecmp(e, ".wasm")) return "application/wasm";
    if (!strcasecmp(e, ".png")) return "image/png";
    if (!strcasecmp(e, ".jpg") || !strcasecmp(e, ".jpeg")) return "image/jpeg";
    if (!strcasecmp(e, ".webp")) return "image/webp";
    if (!strcasecmp(e, ".gif")) return "image/gif";
    if (!strcasecmp(e, ".svg")) return "image/svg+xml";
    if (!strcasecmp(e, ".ico")) return "image/x-icon";
    if (!strcasecmp(e, ".woff2")) return "font/woff2";
    if (!strcasecmp(e, ".woff")) return "font/woff";
    return "application/octet-stream";
}

static void serve_client(avatar_web_t *w, int fd)
{
    char request[WEB_REQUEST_MAX + 1];
    size_t used = 0;
    double read_deadline = web_now() + WEB_IO_SECONDS;
    /* Absolute read deadline also bounds clients trickling one byte at a time. */
    for (;;) {
        if (web_stopping(w)) return;
        if (web_now() >= read_deadline) {
            send_error(w, fd, 408, "Request Timeout", web_now() + WEB_IO_SECONDS);
            return;
        }
        ssize_t n = recv(fd, request + used, WEB_REQUEST_MAX - used, 0);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return;
        if (memchr(request + used, 0, (size_t)n)) {
            send_error(w, fd, 400, "Bad Request", web_now() + WEB_IO_SECONDS);
            return;
        }
        used += (size_t)n;
        request[used] = 0;
        if (strstr(request, "\r\n\r\n")) break;
        if (used == WEB_REQUEST_MAX) {
            send_error(w, fd, 431, "Request Header Fields Too Large",
                       web_now() + WEB_IO_SECONDS);
            return;
        }
    }
    double deadline = web_now() + 30.0; /* bounded total response time */
    char *line_end = strstr(request, "\r\n");
    if (!line_end) { send_error(w, fd, 400, "Bad Request", deadline); return; }
    *line_end = 0;
    char *url = strchr(request, ' ');
    if (!url) { send_error(w, fd, 400, "Bad Request", deadline); return; }
    *url++ = 0;
    char *version = strchr(url, ' ');
    if (!version) { send_error(w, fd, 400, "Bad Request", deadline); return; }
    *version++ = 0;
    if (strcmp(version, "HTTP/1.1") && strcmp(version, "HTTP/1.0")) {
        send_error(w, fd, 400, "Bad Request", deadline);
        return;
    }
    if (strcmp(request, "GET")) {
        send_error(w, fd, 405, "Method Not Allowed", deadline);
        return;
    }
    char path[WEB_PATH_MAX];
    if (decode_path(url, path) != 0) {
        send_error(w, fd, 403, "Forbidden", deadline);
        return;
    }
    if (!strcmp(path, "/api/state")) {
        send_state(w, fd, deadline);
        return;
    }
    size_t len = strlen(path);
    if (len && path[len - 1] == '/') {
        if (len + sizeof("index.html") > sizeof(path)) {
            send_error(w, fd, 414, "URI Too Long", deadline);
            return;
        }
        memcpy(path + len, "index.html", sizeof("index.html"));
    }
    int file = open_static(w, path);
    if (file < 0) { send_error(w, fd, 404, "Not Found", deadline); return; }
    struct stat st;
    if (fstat(file, &st) != 0 || st.st_size < 0) {
        close(file);
        send_error(w, fd, 500, "Internal Server Error", deadline);
        return;
    }
    if (send_header(w, fd, 200, "OK", mime_type(path),
                    (unsigned long long)st.st_size, deadline) == 0) {
        char chunk[16384];
        off_t left = st.st_size;
        while (left > 0 && !web_stopping(w) && web_now() < deadline) {
            size_t size = left > (off_t)sizeof(chunk) ? sizeof(chunk) : (size_t)left;
            ssize_t n = read(file, chunk, size);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0 || send_all(w, fd, chunk, (size_t)n, deadline) != 0) break;
            left -= n;
        }
    }
    close(file);
}

static void *web_thread(void *arg)
{
    avatar_web_t *w = arg;
    struct pollfd fds[2] = {
        { .fd = w->listener, .events = POLLIN },
        { .fd = w->wake[0], .events = POLLIN }
    };
    while (!web_stopping(w)) {
        int rc = poll(fds, 2, -1);
        if (rc < 0 && errno == EINTR) continue;
        if (rc < 0 || fds[1].revents) break;
        if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) break;
        if (!(fds[0].revents & POLLIN)) continue;
        int fd = accept4(w->listener, NULL, NULL, SOCK_CLOEXEC);
        if (fd < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
            fprintf(stderr, "[AVWEB] accept 失败: %s\n", strerror(errno));
            break;
        }
        struct timeval timeout = { .tv_sec = WEB_IO_SECONDS, .tv_usec = 0 };
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) ||
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout))) {
            close(fd);
            continue;
        }
        pthread_mutex_lock(&w->lock);
        int stop = w->stopping;
        if (!stop) w->client = fd;
        pthread_mutex_unlock(&w->lock);
        if (!stop) serve_client(w, fd);
        /* Stop only shutdowns; worker owns close, protected against fd reuse. */
        pthread_mutex_lock(&w->lock);
        w->client = -1;
        close(fd);
        pthread_mutex_unlock(&w->lock);
        if (stop) break;
    }
    fprintf(stderr, "[AVWEB] HTTP 线程退出\n");
    return NULL;
}

avatar_web_t *avatar_web_start(const char *root)
{
    if (!root || !*root) return NULL;
    avatar_web_t *w = calloc(1, sizeof(*w));
    if (!w) { fprintf(stderr, "[AVWEB] 内存分配失败\n"); return NULL; }
    w->listener = w->root = w->wake[0] = w->wake[1] = w->client = -1;
    int rc = pthread_mutex_init(&w->lock, NULL);
    if (rc) { errno = rc; goto fail_alloc; }
    w->root = open(root, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (w->root < 0) goto fail;
    if (pipe2(w->wake, O_CLOEXEC | O_NONBLOCK) != 0) goto fail;
    w->listener = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (w->listener < 0) goto fail;
    int one = 1;
    if (setsockopt(w->listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) goto fail;
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(8765) };
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(w->listener, (struct sockaddr *)&addr, sizeof(addr)) != 0 ||
        listen(w->listener, 8) != 0) goto fail;
    rc = pthread_create(&w->thread, NULL, web_thread, w);
    if (rc) { errno = rc; goto fail; }
    fprintf(stderr, "[AVWEB] http://127.0.0.1:8765/ root=%s\n", root);
    return w;

fail:
    {
        int saved = errno;
        if (w->listener >= 0) close(w->listener);
        if (w->root >= 0) close(w->root);
        if (w->wake[0] >= 0) close(w->wake[0]);
        if (w->wake[1] >= 0) close(w->wake[1]);
        pthread_mutex_destroy(&w->lock);
        errno = saved;
    }
fail_alloc:
    fprintf(stderr, "[AVWEB] 启动失败: %s\n", strerror(errno));
    free(w);
    return NULL;
}

void avatar_web_publish(avatar_web_t *w, const avatar_web_state_t *state)
{
    if (!w || !state) return;
    pthread_mutex_lock(&w->lock);
    unsigned long long seq = w->state.seq + 1;
    w->state = *state;
    w->state.question[sizeof(w->state.question) - 1] = 0;
    w->state.answer[sizeof(w->state.answer) - 1] = 0;
    w->state.seq = seq;
    pthread_mutex_unlock(&w->lock);
}

void avatar_web_stop(avatar_web_t *w)
{
    if (!w) return;
    pthread_mutex_lock(&w->lock);
    w->stopping = 1;
    if (w->client >= 0) shutdown(w->client, SHUT_RDWR);
    pthread_mutex_unlock(&w->lock);
    /* The pipe stays open until after join, so this write cannot raise SIGPIPE. */
    char wake = 1;
    ssize_t n;
    do { n = write(w->wake[1], &wake, 1); } while (n < 0 && errno == EINTR);
    pthread_join(w->thread, NULL);
    close(w->listener);
    close(w->root);
    close(w->wake[0]);
    close(w->wake[1]);
    pthread_mutex_destroy(&w->lock);
    free(w);
    fprintf(stderr, "[AVWEB] 已停止\n");
}