/* Optional loopback-only HTTP bridge. No audio or X11 dependencies. */
#ifndef AVATAR_WEB_H
#define AVATAR_WEB_H

#include "avatar.h"

typedef struct avatar_web_s avatar_web_t;

typedef struct {
    av_state_t state;
    float openness, wide, round_, mic;
    char question[512];
    char answer[1024];
    unsigned long long seq;
} avatar_web_state_t;

/* root must name a readable directory. Failure returns NULL. */
avatar_web_t *avatar_web_start(const char *root);
/* Copies a whole frame under a mutex; seq is assigned by the bridge. */
void avatar_web_publish(avatar_web_t *web, const avatar_web_state_t *state);
/* Caller must stop publishing before stop. Interrupts sockets and joins. */
void avatar_web_stop(avatar_web_t *web);

#endif