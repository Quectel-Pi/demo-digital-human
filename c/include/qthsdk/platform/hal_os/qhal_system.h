/**
 * @file qhal_system.h
 * @brief System HAL abstraction: runtime counter, reset control, random number,
 *        and device identity.
 * @version 0.0.1
 * @date 2023.03.02
 * @details The system APIs must at least cover the following capabilities:
 * - Provide a millisecond runtime counter.
 * - Provide a system reset entry.
 * - Provide random number generation.
 * - Provide the device type, firmware version, and unique identifier.
 *
 * @note This file defines platform-common system APIs. Concrete behavior is
 *       provided by each platform adaptation layer.
 * @copyright Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 */

#ifndef __QHAL_SYSTEM_H__
#define __QHAL_SYSTEM_H__

#include "qhal_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Platform-reported startup or reset reason.
     * @note Available values and mapping accuracy depend on the platform SDK.
     */
    typedef enum
    {
        QHAL_RESET_REASON_POWERON = 0,      /**< Power-on reset */
        QHAL_RESET_REASON_HW_WDOG = 1,      /**< Hardware watchdog reset */
        QHAL_RESET_REASON_FAULT = 2,        /**< Fault or bus error reset */
        QHAL_RESET_REASON_SW_WDOG = 3,      /**< Software watchdog reset */
        QHAL_RESET_REASON_SOFTWARE = 4,     /**< Software-triggered reset */
        QHAL_RESET_REASON_DEEPSLEEP = 5,    /**< Wakeup reset from deep sleep */
        QHAL_RESET_REASON_EXTERNAL = 6,     /**< External reset (button or pin) */
        QHAL_RESET_REASON_UNKNOWN = 7,      /**< Unknown reason */
        QHAL_RESET_REASON_FIB = 8,          /**< FIB bootloader-triggered reset */
        QHAL_RESET_REASON_BOOTLOADER = 9,   /**< Bootloader-related reset */
        QHAL_RESET_REASON_CRASH = 10,       /**< Software crash */
        QHAL_RESET_REASON_FLASH = 11,       /**< Flash fault reset */
        QHAL_RESET_REASON_FATAL = 12,       /**< Fatal unrecoverable error */
        QHAL_RESET_REASON_VINLDO_LOW = 13,  /**< Low supply voltage */
        QHAL_RESET_REASON_VINLDO_HIGH = 14, /**< High supply voltage */
        QHAL_RESET_REASON_OVERTEMP = 15,    /**< Over-temperature protection */
        QHAL_RESET_REASON_UNSUPPORT = 0xFF  /**< Unsupported on current platform */
    } Qhal_resetReason_e;

    /**
     * @brief Initialize optional system-level platform resources.
     * @return OPRT_OK on success, otherwise error code (see qhal_errorCode.h).
     * @note Some platforms may simply return `OPRT_OK` without additional setup.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Adaptation Requirements
     * - If the platform needs to initialize the system clock, random source, or
     *   cached device information first, it can be done here.
     * - If the platform does not require extra initialization, the
     *   implementation should still explicitly return `OPRT_OK`.
     *
     * @par Notes
     * - This API is typically called once during system startup.
     * - Avoid doing very slow work here or anything that may block the startup
     *   sequence.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * oprt_ret Qhal_system_init(void)
     * {
     *     chip_system_init();
     *     return OPRT_OK;
     * }
     * @endcode
     */
    oprt_ret Qhal_system_init(void);

    /**
     * @brief Get the wrap-around upper bound of the runtime counter returned by
     *        `Qhal_system_getMs()`.
     * @return Maximum counter value before wrap-around.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Adaptation Requirements
     * - The returned value must match the same underlying timer source used by
     *   `Qhal_system_getMs()`.
     * - If the platform timer will not wrap in the expected runtime window, it
     *   is also acceptable to return the theoretical upper bound of that timer.
     *
     * @par Notes
     * - This API is mainly used by upper layers to detect wrap-around and to
     *   calculate time deltas safely.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * quint64_t Qhal_system_getMsMax(void)
     * {
     *     return 0xFFFFFFFFULL;
     * }
     * @endcode
     */
    quint64_t Qhal_system_getMsMax(void);

    /**
     * @brief Get the current runtime counter value.
     * @return Milliseconds since boot according to the platform runtime counter.
     * @note Counter source and wrap behavior are platform-dependent.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(required)
     * @note BLE_Open(required)
     *
     * @par Requirements
     * - The return value must use milliseconds as the unit.
     * - It can represent uptime since boot; it does not need to be real
     *   calendar time.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * quint64_t Qhal_system_getMs(void)
     * {
     *     return chip_get_tick_ms();
     * }
     * @endcode
     */
    quint64_t Qhal_system_getMs(void);

    /**
     * @brief Delay the current execution context for the specified time.
     * @param[in] ms Sleep duration in milliseconds.
     * @note Actual sleep duration may be larger than requested due to scheduler
     *       and tick granularity.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(required)
     * @note BLE_Open(required)
     *
     * @par Requirements
     * - The delay unit is milliseconds.
     *
     * @par Notes
     * - This API is usually implemented as a blocking delay and does not need
     *   to be asynchronous.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * void Qhal_system_sleepMs(quint32_t ms)
     * {
     *     chip_delay_ms(ms);
     * }
     * @endcode
     */
    void Qhal_system_sleepMs(quint32_t ms);

    /**
     * @brief Trigger a platform reboot or reset.
     * @note Usually does not return.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(required)
     * @note BLE_Open(required)
     *
     * @par Requirements
     * - Trigger a platform reset.
     * - If the platform requires interrupts to be disabled or pending UART
     *   output to be flushed before reset, handle that here.
     *
     * @par Notes
     * - This API is commonly used after OTA completion or when recovering from
     *   severe fault conditions.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * void Qhal_system_reset(void)
     * {
     *     chip_uart_flush();
     *     chip_global_irq_disable();
     *     chip_system_reset();
     * }
     * @endcode
     */
    void Qhal_system_reset(void);

    /**
     * @brief Get the platform-reported startup or reset reason.
     * @return Value from `Qhal_resetReason_e`.
     * @note If the reason cannot be determined, implementation should return
     *       `QHAL_RESET_REASON_UNKNOWN` or `QHAL_RESET_REASON_UNSUPPORT`.
     * @note LTE_AT(optional)
     * @note LTE_Open(optional)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(required)
     * @note BLE_Open(required)
     *
     * @par Adaptation Requirements
     * - Map the platform-native reset reasons to `Qhal_resetReason_e` as
     *   completely as possible.
     * - If the platform can only distinguish a few reset causes, the returned
     *   value must still remain explicit and deterministic.
     *
     * @par Notes
     * - This API is commonly used during startup to record the cause of the
     *   previous reset.
     */
    Qhal_resetReason_e Qhal_system_getResetReason(void);

    /**
     * @brief Get a random value from the platform.
     * @return 32-bit random value.
     * @note Unless explicitly implemented with a TRNG, this value should be
     *       treated as non-cryptographic random.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(required)
     * @note BLE_Open(required)
     *
     * @par Adaptation Requirements
     * - Return a 32-bit random value.
     * - If the platform does not provide a TRNG, a PRNG or a lower-level random
     *   source can be used, but the upper layers must still receive a usable
     *   value.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * quint32_t Qhal_system_getRandom(void)
     * {
     *     return chip_get_random();
     * }
     * @endcode
     */
    quint32_t Qhal_system_getRandom(void);

    /**
     * @brief Get the device model or module type string.
     * @return Null-terminated device type string pointer.
     * @note The returned pointer is managed by the platform adaptation and must
     *       not be freed or modified by the caller.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Return a human-readable string.
     * - The value is used for configuration, logging, debugging, and
     *   upper-layer identification.
     *
     * @par Notes
     * - The returned value must remain stable and valid after the call. Do not
     *   return temporary stack storage.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * char *Qhal_system_getDevType(void)
     * {
     *     return "MY_BLE_DEVICE";
     * }
     * @endcode
     */
    char *Qhal_system_getDevType(void);

    /**
     * @brief Get the firmware version string.
     * @return Null-terminated firmware version string pointer.
     * @note The returned pointer is managed by the platform adaptation and must
     *       not be freed or modified by the caller.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Return a human-readable string.
     * - The value is used for configuration, logging, debugging, and
     *   upper-layer identification.
     *
     * @par Notes
     * - The returned value must remain stable and valid after the call. Do not
     *   return the address of a local variable.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * char *Qhal_system_getFwVersion(void)
     * {
     *     return "V1.0.0";
     * }
     * @endcode
     */
    char *Qhal_system_getFwVersion(void);

    /**
     * @brief Get the platform test or build version string.
     * @return Null-terminated test version string pointer.
     * @note The returned pointer is managed by the platform adaptation and must
     *       not be freed or modified by the caller.
     * @note LTE_AT(optional)
     * @note LTE_Open(optional)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Adaptation Requirements
     * - If the platform exposes a test version or build version string, return
     *   that value here.
     * - If it does not, return a fixed string or handle it according to the
     *   project convention.
     */
    char *Qhal_system_getTestVersion(void);

    /**
     * @brief Get the device unique identifier string used by the SDK.
     * @return Null-terminated unique ID string pointer.
     * @note The returned pointer must remain valid after the call and must not
     *       be freed by the caller.
     * @note Current upper-layer code may convert this string case in-place. To
     *       stay compatible, the platform adaptation should provide a writable
     *       persistent buffer instead of a string literal.
     * @note LTE_AT(optional)
     * @note LTE_Open(optional)
     * @note WIFI_HEX(optional)
     * @note WIFI_Open(optional)
     * @note BLE_HEX(optional)
     * @note BLE_Open(optional)
     *
     * @par Requirements
     * - Return a stable unique value.
     * - Prefer using a chip UUID, BLE MAC, or a module-unique identifier.
     * - The return value should preferably reside in a static buffer.
     *
     * @par Platform Adaptation Example
     * @code{.c}
     * char *Qhal_system_uuidGet(void)
     * {
     *     static char uuid[32] = {0};
     *     chip_get_unique_id_string(uuid, sizeof(uuid));
     *     return uuid;
     * }
     * @endcode
     */
    char *Qhal_system_uuidGet(void);

#ifdef __cplusplus
}
#endif

#endif /* __QHAL_SYSTEM_H__ */
