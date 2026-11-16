/**
 * @file qhal_file.h
 * @version 0.0.1
 * @date 2023.03.02
 * @brief File-system HAL abstraction.
 * @details This header abstracts file-system operations used by the SDK.
 *
 * @par Recommended Fallback Behavior
 * If the current project does not rely on a file system, the recommended
 * first-pass behavior is to keep the return policy explicit:
 * - `Qhal_file_init()` returns `OPRT_NOT_SUPPORTED`.
 * - `Qhal_file_open()` returns `QHAL_FILE_INVALID`.
 * - Other file APIs return `OPRT_NOT_SUPPORTED`.
 * @note All APIs in this file are optional for platform adaptation.
 * @copyright Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 */

#ifndef __QHAL_FILE_H__
#define __QHAL_FILE_H__

#include "qhal_types.h"
#include "qhal_os_define.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief File open flags.
     * @note This is a lightweight platform HAL flag set, not a full POSIX `open()` flag set.
     */
    typedef enum
    {
        QHAL_O_RDONLY = 1,     /**< Open a file as read only */
        QHAL_O_WRONLY = 2,     /**< Open a file as write only */
        QHAL_O_RDWR = 3,       /**< Open a file as read and write */
        QHAL_O_CREAT = 0x0100, /**< Create a file if it does not exist */
        QHAL_O_TRUNC = 0x0400  /**< Truncate the existing file to zero size */
    } Qhal_openFlags_e;

    /**
     * @brief File seek reference position.
     */
    typedef enum
    {
        QHAL_SEEK_SET = 0, /**< Seek relative to an absolute position (beginning of file) */
        QHAL_SEEK_CUR,     /**< Seek relative to the current file position */
        QHAL_SEEK_END      /**< Seek relative to the end of the file */
    } Qhal_seekWhence_e;

    /**
     * @brief File access mode check flags.
     * @note Some platforms only check file existence and may ignore read/write/execute bits.
     */
    typedef enum
    {
        QHAL_ACCESS_F_OK = 0x00, /**< Check for file existence */
        QHAL_ACCESS_X_OK = 0x01, /**< Check for execute permission */
        QHAL_ACCESS_W_OK = 0x02, /**< Check for write permission */
        QHAL_ACCESS_R_OK = 0x04  /**< Check for read permission */
    } Qhal_accessMode_e;

    /**
     * @brief Initialize the file subsystem.
     * @return OPRT_OK on success, otherwise error code (see qhal_errorCode.h).
     * @note Call before other file APIs on platforms that require explicit init.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Initialize the file or storage subsystem when the target platform
     *   requires explicit setup.
     * - If the project does not use a file system, return
     *   `OPRT_NOT_SUPPORTED` explicitly.
     *
     * @par Notes
     * - Many platforms do not require explicit file-system initialization. In
     *   that case, returning `OPRT_OK` is also acceptable when file support
     *   exists.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_file_init(void)
     * {
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_file_init(void);

    /**
     * @brief Open a file.
     * @param[in] path File path.
     * @param[in] flags Open flags.
     * @return File handle on success, `QHAL_FILE_INVALID` on failure.
     * @note Supported flag combinations and path mapping are platform-dependent.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Open the specified path using the HAL open flags.
     * - Return `QHAL_FILE_INVALID` explicitly when file support is not provided
     *   by the platform adaptation.
     *
     * @par Notes
     * - Path translation and the mapping between HAL flags and platform-native
     *   flags are platform-dependent.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * Qhal_file_t Qhal_file_open(const char *path, qint32_t flags)
     * {
     *     UNUSED(path);
     *     UNUSED(flags);
     *     return QHAL_FILE_INVALID;
     * }
     * @endcode
     */
    Qhal_file_t Qhal_file_open(const char *path, qint32_t flags);

    /**
     * @brief Close a file.
     * @param[in] file File handle.
     * @return OPRT_OK on success, otherwise error code.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Close the specified file handle when file support exists.
     * - Return `OPRT_NOT_SUPPORTED` explicitly when the project does not use a
     *   file system.
     *
     * @par Notes
     * - After a successful close, the handle should no longer be used.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_file_close(Qhal_file_t file)
     * {
     *     UNUSED(file);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_file_close(Qhal_file_t file);

    /**
     * @brief Read file data.
     * @param[in] file File handle.
     * @param[out] buf Output buffer.
     * @param[in] len Bytes to read.
     * @return Non-negative bytes read on success, negative error code on failure.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Read up to `len` bytes from the file into `buf`.
     * - Return `OPRT_NOT_SUPPORTED` explicitly when file reads are unsupported
     *   on the current platform.
     *
     * @par Notes
     * - A successful read may still return fewer than `len` bytes, depending on
     *   file position and platform behavior.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_file_read(Qhal_file_t file, void *buf, quint32_t len)
     * {
     *     UNUSED(file);
     *     UNUSED(buf);
     *     UNUSED(len);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_file_read(Qhal_file_t file, void *buf, quint32_t len);

    /**
     * @brief Write file data.
     * @param[in] file File handle.
     * @param[in] buf Input buffer.
     * @param[in] len Bytes to write.
     * @return Non-negative bytes written on success, negative error code on failure.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Write up to `len` bytes from `buf` into the file.
     * - Return `OPRT_NOT_SUPPORTED` explicitly when file writes are unsupported
     *   on the current platform.
     *
     * @par Notes
     * - Partial writes may occur on some platforms, so upper layers should rely
     *   on the returned value instead of assuming `len` bytes were written.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_file_write(Qhal_file_t file, void *buf, quint32_t len)
     * {
     *     UNUSED(file);
     *     UNUSED(buf);
     *     UNUSED(len);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_file_write(Qhal_file_t file, void *buf, quint32_t len);

    /**
     * @brief Remove a file.
     * @param[in] path File path.
     * @return OPRT_OK on success, otherwise error code.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Remove the target file when the platform file system supports deletion.
     * - Return `OPRT_NOT_SUPPORTED` explicitly when remove support is absent.
     *
     * @par Notes
     * - Path syntax and whether directories are supported are platform-specific.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_file_remove(const char *path)
     * {
     *     UNUSED(path);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_file_remove(const char *path);

    /**
     * @brief Seek the file pointer.
     * @param[in] file File handle.
     * @param[in] offs Offset value.
     * @param[in] whence Seek reference (`Qhal_seekWhence_e`).
     * @return OPRT_OK on success, otherwise error code.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Move the file pointer according to `offs` and `whence`.
     * - Return `OPRT_NOT_SUPPORTED` explicitly when seek behavior is not
     *   implemented.
     *
     * @par Notes
     * - Seek semantics should remain consistent with `QHAL_SEEK_SET`,
     *   `QHAL_SEEK_CUR`, and `QHAL_SEEK_END`.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_file_seek(Qhal_file_t file, qint64_t offs, qint32_t whence)
     * {
     *     UNUSED(file);
     *     UNUSED(offs);
     *     UNUSED(whence);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_file_seek(Qhal_file_t file, qint64_t offs, qint32_t whence);

    /**
     * @brief Get the current file offset.
     * @param[in] file File handle.
     * @return Non-negative current offset on success, negative error code on failure.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Return the current file offset when file-position queries are supported.
     * - Return `OPRT_NOT_SUPPORTED` explicitly when this API is not provided by
     *   the current platform adaptation.
     *
     * @par Notes
     * - The returned offset is usually measured in bytes from the file start.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_file_tell(Qhal_file_t file)
     * {
     *     UNUSED(file);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_file_tell(Qhal_file_t file);

    /**
     * @brief Get file size.
     * @param[in] path File path.
     * @return Non-negative file size on success, negative error code on failure.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Return the file size for the specified path when supported.
     * - Return `OPRT_NOT_SUPPORTED` explicitly when the file system capability
     *   is not available.
     *
     * @par Notes
     * - Some platforms determine size by opening the file and seeking to the
     *   end, while others can query metadata directly.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_file_getSize(const char *path)
     * {
     *     UNUSED(path);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_file_getSize(const char *path);

    /**
     * @brief Check file accessibility.
     * @param[in] path File path.
     * @param[in] mode Access mode bitmask (`Qhal_accessMode_e`).
     * @return OPRT_OK if accessible, otherwise error code (see qhal_errorCode.h).
     * @note Some platforms ignore `mode` and only test whether the path exists.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Check the accessibility or existence of the specified path.
     * - Return `OPRT_NOT_SUPPORTED` explicitly when file-access checks are not
     *   implemented on the platform.
     *
     * @par Notes
     * - On simple embedded file systems, this API may only check whether the
     *   path exists and may ignore detailed access-mode bits.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_file_access(const char *path, qint32_t mode)
     * {
     *     UNUSED(path);
     *     UNUSED(mode);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_file_access(const char *path, qint32_t mode);

#ifdef __cplusplus
}
#endif

#endif /* __QHAL_FILE_H__ */
