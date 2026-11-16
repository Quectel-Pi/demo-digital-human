/**
 * @file qhal_log.h
 * @version 0.0.1
 * @date 2023.03.02
 * @brief Log output HAL abstraction.
 * @details `Qhal_log_output()` provides the unified log output entry for the
 *          SDK.
 *
 * @par Goals
 * - The SDK must be able to print logs during initialization.
 * - Error paths must still be able to print diagnostic information.
 * - Platform debugging should be able to observe key runtime states.
 *
 * @par Adaptation Notes
 * - Prefer mapping this API to UART output or a platform debug console.
 * - Support `printf`-style formatting.
 * - At minimum, make sure the implementation does not fail silently.
 *
 * @copyright Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 */

#ifndef __QHAL_LOG_H__
#define __QHAL_LOG_H__

#include "qhal_types.h"
#include "qhal_os_define.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Output a formatted log message.
 * @param[in] format `printf`-style format string.
 * @param[in] ... Format arguments.
 * @note Output channel, line length limit, and thread-safety depend on the
 *       platform implementation.
 * @note When `Qhal_log_output` is already provided as a macro by the platform
 *       adaptation, this declaration is skipped.
 * @note LTE_AT(required)
 * @note LTE_Open(required)
 * @note WIFI_HEX(required)
 * @note WIFI_Open(required)
 * @note BLE_HEX(required)
 * @note BLE_Open(required)
 *
 * @par Platform Adaptation Example
 * @code{.c}
 * void Qhal_log_output(const char *format, ...)
 * {
 *     char buf[256] = {0};
 *     va_list ap;
 *
 *     va_start(ap, format);
 *     vsnprintf(buf, sizeof(buf), format, ap);
 *     va_end(ap);
 *
 *     chip_uart_print(buf);
 * }
 * @endcode
 */
#ifndef Qhal_log_output
void Qhal_log_output(const char *format, ...);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __QHAL_LOG_H__ */
