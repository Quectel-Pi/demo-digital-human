/*************************************************************************
 * @version : 0.0.1
 * @date : 2023.03.03
 * @brief : the implements of time, include timezone and summer time
 * @copyright : Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 *************************************************************************/
#ifndef __QUOS_TIME_H__
#define __QUOS_TIME_H__
#include "qhal_types.h"
#include "qhal_sdk_config.h"
#include "qhal_mutex.h"
#include "qhal_system.h"
#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct
	{
		quint32_t sec; /*  second,support 136years */
		quint32_t ms;  /* millisecond */
	} Quos_systick_t;

	typedef struct
	{
		quint8_t tm_wday;  /* Week - The value range is 0,6, Sunday to Saturday */
		quint8_t tm_sec;   /* Seconds - The value range is [0,59] */
		quint8_t tm_min;   /* Minute - The value range is [0,59] */
		quint8_t tm_hour;  /* Hour - The value range is 0,23. */
		quint8_t tm_mday;  /* Date of the month - Value range [1,31] */
		quint8_t tm_mon;   /* Month (starting from January, 0 means January) - The value range is [0,11] */
		quint16_t tm_yday; /* Day of the year - Value range [0,365] */
		quint32_t tm_year; /* Years, counting from 0, it is different from POSIX(from 1990) */
		quint8_t tm_isdst; /* Daylight Saving Time flag: >0 for DST in effect, 0 for not in DST */
	} Quos_tm_t;

	typedef struct
	{
		quint32_t start; /* 此年夏令时开始时间戳 */
		quint32_t end;	 /* 此年夏令时结束时间戳 */
	} Quos_sumtime_t;

	/**
	 * @brief : time service init
	 * @param [in] : none
	 * @return :
	 * @note: should call this function first before any other time functions
	 */
	oprt_ret Quos_timeInit(void);

	/**
	 * @brief : The time since the system started
	 * @return : sys tick, include second and msec
	 * @note: none
	 */
	Quos_systick_t Quos_timeGetSystem(void);

	/**
	 * @brief : The milliseconds since the system started
	 * @param [in] None
	 * @return : The milliseconds
	 * @note: none
	 */
	quint32_t Quos_timeGetSystemMs(void);

	/**
	 * @brief : The seconds since the system started
	 * @param [in] None
	 * @return : The seconds
	 * @note: none
	 */
	quint32_t Quos_timeGetSystemS(void);

	/**
	 * @brief : get new systick
	 * @param [in] tick: base systick
	 * @param [in] ms: millisecond offset
	 * @return : systick
	 * @note: none
	 */
	Quos_systick_t Quos_timeAddMs(Quos_systick_t tick, quint32_t ms);

	/**
	 * @brief : compare two systick
	 * @param [in] time1: systick1
	 * @param [in] time2: systick2
	 * @param [in] isMs: TRUE: interval unit is millisecond, FALSE: interval unit is second
	 * @param [out] interval: the interval of two systick, unit base on isMs
	 * @return : TRUE: systick1 > systick2, FALSE: systick1 <= systick2
	 * @note: none
	 */
	qbool Quos_timeDiff(Quos_systick_t time1, Quos_systick_t time2, qbool isMs, quint32_t *interval);

	/**
	 * Sets the summary time information.
	 *
	 * This function is used to set a batch of summary time information, which is a critical part of the system's time management.
	 * It first checks if the number of summary times exceeds the maximum limit. If it does, the function terminates immediately.
	 * Then, it copies the passed summary time information into the system's summary time table, thereby updating the system's time settings.
	 *
	 * @param sumtimes Pointer to the summary time information to be set. This is an array of summary time structures, containing detailed time information.
	 * @param num The number of summary times. Indicates how many pieces of summary time information are to be set.
	 *
	 * Note: If num exceeds the maximum limit, the operation is considered invalid, and the function exits directly.
	 * The memory operation function Quos_memmove is used here to copy the contents of the summary time structure array,
	 * which safely handles the memory copy even if the source and destination memory regions overlap.
	 */
	void Quos_timeSetSumtime(const Quos_sumtime_t *sumtimes, quint32_t num);

	/**
	 * Quos_timeGetSumtime copies the summary time information to the specified memory location.
	 *
	 * This function is designed to batch transfer summary time data to a caller-provided buffer. It first checks
	 * if there is any summary time data available, and if so, moves the data from the internal table to the caller's buffer.
	 *
	 * @param sumtimes A pointer to the buffer where the caller wants to store the summary time information. This buffer
	 *                 should be large enough to hold the number of summary time structures being copied.
	 * @param num A pointer to a variable where the number of summary time entries will be stored. This parameter
	 *            is used both as an output parameter and potentially as an input parameter indicating the maximum
	 *            number of entries the caller's buffer can hold.
	 */
	void Quos_timeGetSumtime(Quos_sumtime_t *sumtimes, quint32_t *num);

	/**
	 * Checks if the time is set.
	 *
	 * This function determines if the time has been set by checking if both the milliseconds and seconds of the Quos_timeUnix.baseUnix structure are zero.
	 * If both are zero, it indicates that the time has not been set; otherwise, the time has been set.
	 *
	 * @return qbool
	 * - Returns FALSE if the time has not been set.
	 * - Returns TRUE if the time has been set.
	 */
	qbool Quos_timeIsSet(void);

	/**
	 * Sets the current time in UTC format.
	 *
	 * This function is used to set the current time in UTC format. It acquires a mutex before setting the time to ensure thread safety.
	 * The time setting is based on a time structure, updating the system's Unix time base.
	 *
	 * @param tv A time structure containing the UTC time information.
	 */
	void Quos_timeSetUtc(Quos_systick_t tv);

	/**
	 * @brief Get the current UTC time
	 *
	 * This function retrieves the current system time and calculates the UTC time
	 * by comparing it with the Unix time base. It handles cases where the system
	 * time's milliseconds are less than the Unix base time's milliseconds.
	 *
	 * @return Quos_systick_t The current UTC time, including seconds and milliseconds
	 */
	Quos_systick_t Quos_timeGetUtc(void);

	/**
	 * Sets the system time using a Unix timestamp.
	 *
	 * This function converts a Unix timestamp (in milliseconds) into a Quos_systick_t structure,
	 * and then calls the Quos_timeSetUtc function to set the system time.
	 *
	 * @param timestamp Unix timestamp in milliseconds.
	 */
	void Quos_timeSetUnix(quint64_t timestamp);

	/**
	 * Converts the current time to Unix timestamp format.
	 *
	 * This function calls `Quos_timeGetUtc()` to obtain the current time in UTC, and then converts it to a Unix timestamp.
	 * A Unix timestamp represents the number of milliseconds that have elapsed since January 1, 1970 (00:00:00 UTC).
	 *
	 * @param timestamp Pointer to a variable where the result of the conversion, the Unix timestamp, will be stored.
	 *                  The caller must ensure that the pointer is not NULL.
	 * @return None
	 */
	void Quos_timeGetUnix(quint64_t *timestamp);

	/**
	 * Sets the timezone.
	 *
	 * This function is used to set the timezone information. The valid range of timezones is from -12 to +12 hours (excluding -12 and +12 hours),
	 * corresponding to -12 * 3600 to 12 * 3600 seconds. If the input timezone exceeds this range, the function will return FALSE, indicating failure.
	 * If the input is valid, it will update the global variable Quos_timezone with the input value and return TRUE, indicating success.
	 *
	 * @param tz The number of seconds relative to Coordinated Universal Time (UTC), representing the timezone offset.
	 * @return Returns a qbool value, TRUE indicates success, FALSE indicates failure.
	 */
	qbool Quos_timeSetTimezone(qint32_t tz);

	/**
	 * @brief Get the system timezone offset
	 *
	 * This function retrieves the timezone offset of the system in seconds.
	 * The offset is the difference between the local time and UTC.
	 *
	 * @return qint32_t The timezone offset in seconds. Positive values indicate
	 *                  time zones east of UTC, and negative values indicate time zones
	 *                  west of UTC.
	 */
	qint32_t Quos_timeGetTimezone(void);

	/**
	 * Converts a time zone string into an integer representation.
	 *
	 * This function takes a time zone string in the format "+/-HH:MM" and converts it into an integer representing the number of seconds offset from UTC.
	 * The valid time zone string format is 6 characters long, starting with a '+' or '-' sign, followed by two digits for hours and two digits for minutes, separated by a ':'.
	 *
	 * @param inStr The input time zone string, e.g., "+03:00".
	 * @param outInt Pointer to an integer where the converted time zone offset in seconds will be stored.
	 *               If the input string is invalid, the value at this pointer will not be modified.
	 * @return Returns TRUE if the conversion is successful, otherwise returns FALSE.
	 */
	qbool Quos_timezone2Int(const char *inStr, qint32_t *outInt);

	/**
	 * Converts a given timestamp to Coordinated Universal Time (UTC) in calendar time format.
	 *
	 * @param time The timestamp to be converted, representing the number of seconds since the Epoch (00:00:00 UTC, January 1, 1970).
	 * @return Returns a Quos_tm_t structure containing the calendar time information.
	 *
	 * This function calls quos_timeConvertToCalendar to convert the input timestamp into calendar time format.
	 * After conversion, it explicitly sets the tm_isdst member of the Quos_tm_t structure to 0, indicating that the converted time is in UTC.
	 */
	Quos_tm_t Quos_gmtime(quint32_t time);

	/**
	 * Converts the given time to local time.
	 *
	 * This function takes a time value in seconds and converts it to local time based on the current timezone.
	 * It also determines if daylight saving time is in effect for the given time and adjusts the time accordingly.
	 *
	 * @param time The time in seconds since the epoch (1970-01-01 00:00:00 UTC).
	 * @return A Quos_tm_t structure containing the broken-down local time.
	 *
	 * Note: This function uses the Quos_timezone global variable to adjust the time to the local timezone.
	 * It also uses the quos_timeIsDST function to determine if daylight saving time is in effect for the given time.
	 * The actual conversion to calendar time is performed by the quos_timeConvertToCalendar function.
	 */
	Quos_tm_t Quos_localtime(quint32_t time);

	/**
	 * Converts a broken-down time structure into the number of seconds since the Epoch (00:00:00 UTC, January 1, 1970).
	 * This function does not use the standard library's mktime function, but instead calculates the total seconds since the Epoch through arithmetic operations.
	 *
	 * @param tm A Quos_tm_t structure that contains the time information to be converted, including year, month, day, hour, minute, second, and whether it is a leap year.
	 * @return Returns the number of seconds since the Epoch.
	 *
	 * Note: This function takes into account leap years and daylight saving time.
	 */
	quint32_t Quos_mktime(Quos_tm_t tm);

#ifdef __cplusplus
}
#endif

#endif /* __QUOS_TIME_H__ */
