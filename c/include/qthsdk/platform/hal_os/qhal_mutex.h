/**
 * @file qhal_mutex.h
 * @version 0.0.1
 * @date 2023.03.02
 * @brief Mutex HAL abstraction.
 * @details This header defines the common mutex contract for platform
 * adaptation.
 *
 * @par Recommended Fallback Behavior
 * If the current project phase does not rely on mutex protection, the
 * implementation may still provide a minimal compatible behavior:
 * `Qhal_mutex_init()` can assign any non-zero dummy handle and return
 * `OPRT_OK`, while the lock, unlock, and release APIs can explicitly return
 * `OPRT_NOT_SUPPORTED`.
 * @note This header defines mutex API contracts in platform-common.
 *       Platform adaptation must provide concrete lock behavior.
 * @copyright Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 */

#ifndef __QHAL_MUTEX_H__
#define __QHAL_MUTEX_H__

#include "qhal_types.h"
#include "qhal_os_define.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Create and initialize a mutex.
     * @param[out] pMutexHandle Output mutex handle pointer.
     * @return OPRT_OK on success, otherwise error code (see qhal_errorCode.h).
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Create and initialize a platform mutex when mutex support is available.
     * - If the current project does not rely on mutexes, it is still
     *   acceptable to assign a dummy non-zero handle and return `OPRT_OK`.
     *
     * @par Notes
     * - Some upper-layer code only checks whether initialization succeeded, so a
     *   deterministic dummy handle is preferable to leaving the handle unset.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_mutex_init(Qhal_mutex_t *handle)
     * {
     *     *handle = 0x01;
     *     return OPRT_OK;
     * }
     * @endcode
     */
    oprt_ret Qhal_mutex_init(Qhal_mutex_t *pMutexHandle);

    /**
     * @brief Lock a mutex.
     * @param[in] pMutexHandle Mutex handle.
     * @return OPRT_OK on success, otherwise error code (see qhal_errorCode.h).
     * @note Lock behavior (recursive or non-recursive) depends on platform adaptation.
     * @note This API has no timeout parameter. Most adaptations block until the mutex is acquired or an internal error occurs.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Acquire the specified mutex when mutex support is implemented.
     * - If mutexes are intentionally unsupported in the current project stage,
     *   return `OPRT_NOT_SUPPORTED` explicitly.
     *
     * @par Notes
     * - The recursive or non-recursive behavior remains platform-specific.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_mutex_lock(Qhal_mutex_t handle)
     * {
     *     UNUSED(handle);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_mutex_lock(Qhal_mutex_t pMutexHandle);

    /**
     * @brief Unlock a mutex.
     * @param[in] pMutexHandle Mutex handle.
     * @return OPRT_OK on success, otherwise error code (see qhal_errorCode.h).
     * @note Caller/ownership rules depend on platform adaptation.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Release the specified mutex lock when mutex support is implemented.
     * - If the project does not use mutexes, return `OPRT_NOT_SUPPORTED`
     *   explicitly.
     *
     * @par Notes
     * - Ownership checks are platform-specific.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_mutex_unlock(Qhal_mutex_t handle)
     * {
     *     UNUSED(handle);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_mutex_unlock(Qhal_mutex_t pMutexHandle);

    /**
     * @brief Destroy and release a mutex.
     * @param[in] pMutexHandle Mutex handle.
     * @return OPRT_OK on success, otherwise error code (see qhal_errorCode.h).
     * @note After release, the handle is invalid.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Destroy the mutex and release the associated resources when supported by
     *   the platform.
     * - If mutex support is intentionally omitted, return
     *   `OPRT_NOT_SUPPORTED`.
     *
     * @par Notes
     * - After successful release, the mutex handle should be treated as invalid
     *   and must not be reused.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_mutex_release(Qhal_mutex_t handle)
     * {
     *     UNUSED(handle);
     *     return OPRT_NOT_SUPPORTED;
     * }
     * @endcode
     */
    oprt_ret Qhal_mutex_release(Qhal_mutex_t pMutexHandle);

#ifdef __cplusplus
}
#endif

#endif /* __QHAL_MUTEX_H__ */
