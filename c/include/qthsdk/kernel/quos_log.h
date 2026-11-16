/*************************************************************************
 * @version : 0.0.1
 * @date : 2023.03.03
 * @brief : the implements of log
 * @copyright : Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 *************************************************************************/
#ifndef __QUOS_LOG_H__
#define __QUOS_LOG_H__
#include "qhal_types.h"
#include "qhal_sdk_config.h"
#include "qhal_mutex.h"
#include "qhal_log.h"
#include "qhal_memory.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define LL_OFF 	(0X07) /* Close all logs */
#define LL_FAIL (0X06) /* An error that will cause the program to exit or more */
#define LL_ERR 	(0X05) /* An error occurs that does not cause the program to exit or more */
#define LL_WARN (0X04) /* The warning level is incorrect or higher */
#define LL_INFO (0X03) /* The coarse granularity level is log or higher */
#define LL_DBG 	(0X02) /* The fine-granularity level is log or higher */
#define LL_DUMP (0X01) /* dump data, used only for Quos_logHexDump/Quos_logHexDump16 printing */

#ifndef QUOS_LOGL
#define QUOS_LOGL LL_DBG
#endif
#ifndef QUOS_LOGHOOK_USE
#define QUOS_LOGHOOK_USE 0
#endif
#ifndef QUOS_LOGASSERT_ENABLE
#define QUOS_LOGASSERT_ENABLE 0
#endif

typedef void (*Quos_logPrintfCb)(qint8_t level, const char *typeString, const void *buff, quint32_t buffLen);

	/**
	 * @brief Initializes the Quos logging system.
	 *
	 * This function sets up the necessary resources and configurations required for the logging system to operate.
	 * It should be called before any other logging functions are used.
	 *
	 * @return oprt_ret Returns OPRT_OK on successful initialization, or an error code on failure.
	 */
	oprt_ret Quos_logInit(void);

	/**
	 * @brief Prints log information according to the specified format.
	 *
	 * This function filters logs based on the log level, acquires the mutex for thread safety, constructs the log information,
	 * and outputs it. It supports custom log output callbacks and ensures that the log output does not exceed the maximum length.
	 *
	 * @param typeStr The string representing the log type.
	 * @param level The log level.
	 * @param funName The name of the function where the log is located.
	 * @param line The line number where the log is located.
	 */
	void quos_logPrint(qint8_t level, const char *typeString, const char *funName, qint32_t line, const char *format, ...);

	/**
	 * @brief Dump log information and data in a specific format.
	 *
	 * This function prints log information and binary data in a specific format based on the log level and type.
	 * It checks if the current log level meets the output criteria, initializes the mutex if not already initialized,
	 * and then prints the log information and data in a loop, formatting the data as 8-bit or 16-bit hexadecimal values.
	 *
	 * @param dat Pointer to the binary data to be printed.
	 * @param datLen Length of the binary data.
	 * @param size Data size, currently supports 8-bit or 16-bit.
	 */
	void quos_logDump(qint8_t level, const char *typeString, const char *funName, qint32_t line, const char *head, const void *dat, quint32_t datLen, quint8_t size);

	/**
	 * @brief Define a macro for logging messages
	 *
	 * This macro simplifies the logging process by encapsulating the call to the actual logging function.
	 * The macro uses a do-while loop to ensure that it can be used as a single statement in the code,
	 * preventing potential issues with statement parsing.
	 *
	 * @param TYPE The type of log, typically used to identify the source or purpose of the log
	 * @param LEVEL The log level, indicating the importance or severity of the log
	 * @param format The format string for the log message, describing the structure of the log information
	 * @param ... Variable arguments list, containing the values for the format specifiers in the format string
	 */
#define Quos_logPrintf(TYPE, LEVEL, format, ...)                                        \
	do                                                                                  \
	{                                                                                   \
		if (LEVEL >= TYPE && LEVEL >= QUOS_LOGL)                                        \
		{                                                                               \
			quos_logPrint(LEVEL, #TYPE, __FUNCTION__, __LINE__, format, ##__VA_ARGS__); \
		}                                                                               \
	} while (0)

/**
 * Quos_logHexDump macro definition
 *
 * This macro is used to print the hexadecimal dump of data in a formatted way
 * It captures the type of log, log level, function name, line number, data label, data content, and data length, and passes these parameters to the quos_logDump function for processing
 * The macro uses the do-while(0) structure to ensure that the macro definition can be safely used in any context, avoiding the expansion of the macro across multiple lines and causing syntax errors
 *
 * @param TYPE The type of log, represented as a string
 * @param LEVEL The log level
 * @param HEAD The label for the data
 * @param DAT Pointer to the data content
 * @param DATLEN The length of the data
 */
#define Quos_logHexDump(TYPE, LEVEL, HEAD, DAT, DATLEN)                               \
	do                                                                                \
	{                                                                                 \
		if (LEVEL >= TYPE && LEVEL >= QUOS_LOGL)                                      \
		{                                                                             \
			quos_logDump(LEVEL, #TYPE, __FUNCTION__, __LINE__, HEAD, DAT, DATLEN, 8); \
		}                                                                             \
	} while (0)

/**
 * @brief A macro to log data in hexadecimal format with 16-byte width.
 *
 * This macro logs the provided data in a hexadecimal format, capturing the
 * type, level, and context information along with the function name and line number
 * where the macro is invoked. It internally calls the `quos_logDump` function
 * to perform the actual logging.
 *
 * @param TYPE The type of log entry (e.g., "INFO", "DEBUG").
 * @param LEVEL The severity level of the log (e.g., LOG_ERROR, LOG_WARNING).
 * @param HEAD A header or additional context for the log message.
 * @param DAT A pointer to the data buffer that needs to be logged.
 * @param DATLEN The length of the data buffer in bytes.
 */
#define Quos_logHexDumpU16(TYPE, LEVEL, HEAD, DAT, DATLEN)                             \
	do                                                                                 \
	{                                                                                  \
		if (LEVEL >= TYPE && LEVEL >= QUOS_LOGL)                                       \
		{                                                                              \
			quos_logDump(LEVEL, #TYPE, __FUNCTION__, __LINE__, HEAD, DAT, DATLEN, 16); \
		}                                                                              \
	} while (0)

	/**
	 * Converts a 64-bit integer to a string and returns it. This function is primarily used for logging 64-bit integers.
	 *
	 * @param num The 64-bit integer to be converted to a string.
	 * @return Returns a pointer to a static character array containing the converted string.
	 *
	 * Note: This function uses a static character array to store the converted string, which may be overwritten by subsequent calls.
	 *       Therefore, it is not recommended to use this function in scenarios where multiple threads access it simultaneously.
	 */
	char *Quos_logInt64ToString(qint64_t num);

	/**
	 * Sets a custom callback function for log output.
	 * The callback function will be invoked with the log level, type string, and log message.
	 * This allows for flexible handling of log messages, such as redirecting them to different outputs or formats.
	 * @param cb The callback function to be set for log output.
	 * @note The callback function must be thread-safe and should not block or perform time-consuming operations.
	 * @note If no callback is set, the default log output method will be used.
	 * @note The callback function should not modify the log message string or its contents.
	 */
	void Quos_logSetCb(Quos_logPrintfCb cb);


	/**
	 * @brief Sets the log level for the Quos logging system.
	 *
	 * This function is used to set the global log level, which determines the types of log messages that will be shown.
	 * Log levels not within the specified range will result in an error return.
	 *
	 * @param level The log level to be set, must be within the range specified by the logging system.
	 * @return oprt_ret Returns the execution result: OPRT_OK for success, OPRT_LOG_INVALID_LEVEL for invalid log level.
	 */
	oprt_ret Quos_logSetUsrLevel(quint8_t level);

	/**
	 * @brief Get the current log level configuration.
	 *
	 * This function retrieves the current log level setting from the log configuration.
	 * The log level determines which severity of log messages will be displayed or recorded.
	 *
	 * @return quint8_t The current log level value.
	 */
	quint8_t Quos_logGetUsrLevel(void);

#if (1 == QUOS_LOGASSERT_ENABLE)
#define QUOS_ASSERT(expr, format, ...)                                                    \
	do                                                                                    \
	{                                                                                     \
		if (!(expr))                                                                      \
		{                                                                                 \
			quos_logPrint(LL_FAIL, #TYPE, __FUNCTION__, __LINE__, format, ##__VA_ARGS__); \
			while (1)                                                                     \
				;                                                                         \
		}                                                                                 \
	} while (0);
#else
#define QUOS_ASSERT(expr, format, ...) \
	do                                 \
	{                                  \
	} while (0);
#endif
#ifdef __cplusplus
}
#endif

#endif /* __QUOS_LOG_H__ */