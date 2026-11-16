/*************************************************************************
 * @version : 0.0.1
 * @date : 2023.03.03
 * @brief : the implements of common tool functions
 * @copyright : Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 *************************************************************************/
#ifndef __QUOS_TOOLS_H__
#define __QUOS_TOOLS_H__

#include "qhal_types.h"
#include "qhal_sdk_config.h"
#include "qhal_file.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * __ENDIANCHANGE macro: Byte order reversal macro, used to reverse the byte order of variables.
 * This macro determines the data type of the variable based on its size and performs corresponding byte order reversal operations.
 *
 * Parameters:
 *   x - The variable whose byte order needs to be reversed.
 *
 * Return value:
 *   Returns the value of x with reversed byte order.
 *
 * Description:
 *   - If the size of x is 2 bytes, it performs a byte swap by shifting.
 *   - If the size of x is 4 bytes, it performs a byte swap by shifting and bitwise OR operations.
 *   - If the size of x is 8 bytes, it performs a byte swap by shifting and bitwise OR operations.
 *   - If the size of x does not match the above conditions, it returns x directly.
 *
 * Note:
 *   This macro uses sizeof to determine the size of x, and performs different operations based on different data types.
 *   It is only applicable to quint16_t, quint32_t and quint64_t data types.
 */
#define __ENDIANCHANGE(x) ((sizeof(x) == 2) ? (quint16_t)((x >> 8) | (x << 8)) : ((sizeof(x) == 4) ? (quint32_t)((((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) | (((x) << 8) & 0x00FF0000) | (((x) << 24) & 0xFF000000)) : ((sizeof(x) == 8) ? (quint64_t)((((quint64_t)(x) >> 56) & 0x00000000000000FFLL) | (((quint64_t)(x) >> 40) & 0x000000000000FF00LL) | (((quint64_t)(x) >> 24) & 0x0000000000FF0000LL) | (((quint64_t)(x) >> 8) & 0x00000000FF000000LL) | (((quint64_t)(x) << 8) & 0x000000FF00000000LL) | (((quint64_t)(x) << 24) & 0x0000FF0000000000LL) | (((quint64_t)(x) << 40) & 0x00FF000000000000LL) | (((quint64_t)(x) << 56) & 0xFF00000000000000LL)) : (quint64_t)(x))))

/**
 * __BYTE_TO_ALIGN macro definition
 *
 * The purpose of this macro is to calculate and convert a given size (X) to the nearest multiple of a specified alignment value (Y).
 * This operation ensures that the size is aligned to a specific byte boundary, which is crucial for hardware access or data structure alignment.
 *
 * Parameters:
 * X - The original size that needs to be aligned.
 * Y - The alignment value, i.e., the byte boundary to which X needs to be aligned.
 *
 * Return value:
 * The size after alignment to the nearest multiple of Y.
 *
 * Explanation:
 * This macro checks if X is a multiple of Y by calculating the remainder of X divided by Y (X % Y).
 * If the remainder is 0, it means X is already a multiple of Y, and no adjustment is needed, returning X directly.
 * If the remainder is not 0, it means X is not a multiple of Y, and the formula (X + Y - (X % Y)) is used to calculate the smallest number greater than X that is a multiple of Y.
 * This ensures that the result is aligned to the specified Y byte boundary.
 */
#define __BYTE_TO_ALIGN(X, Y) ((X) % (Y) ? ((X) + (Y) - (X) % (Y)) : (X))

#define __TO_UPPER(X) ((X) & (~0x20))
#define __TO_LOWER(X) ((X) | 0x20)

#define __IS_LETTER(X) (('A' <= (X) && (X) <= 'Z') || ('a' <= (X) && (X) <= 'z'))
#define __IS_DIGIT(X) ('0' <= (X) && (X) <= '9')
#define __IS_XDIGIT(X) (('0' <= (X) && (X) <= '9') || ('a' <= (X) && (X) <= 'f') || ('A' <= (X) && (X) <= 'F'))

/* Get the maximum and minimum values */
#define __GET_MAX(x, y) (((x) > (y)) ? (x) : (y)) /* Returns the larger of two values x and y */
#define __GET_MIN(x, y) (((x) < (y)) ? (x) : (y)) /* Returns the smaller of two values x and y */

/* Get the offset of a field within a structure */
#define __GET_POS_ELEMENT(type, field) ((pointer_t) & (((type *)0)->field))

/* Get the size of a field within a structure */
#define __GET_SIZE_ELEMENT(type, field) sizeof(((type *)0)->field)

/* Get the structure from an element address */
#define __GET_STRUCT_BY_ELEMENT(ptr, type, field) ((type *)((pointer_t)ptr - __GET_POS_ELEMENT(type, field)))

/* Get the nearest lower multiple of n that is less than or equal to X */
#define __GET_SMALL_N(x, n) ((x) / (n) * (n))

/* Get the nearest higher multiple of n that is greater than or equal to X */
#define __GET_BIG_N(x, n) (((x) + (n) - 1) / (n) * (n))

/* Convert a macro to a string */
#define __MACRO2STR_1(s) #s				/* Converts a macro 's' to its string representation */
#define __MACRO2STR_2(s) __MACRO2STR_1(s) /* Indirectly converts a macro 's' to its string representation */

/* Concatenate strings */
#define __STRCAT_STR_1(A, B) A##B				/* Concatenates two strings A and B */
#define __STRCAT_STR_2(A, B) __STRCAT_STR_1(A, B) /* Indirectly concatenates two strings A and B */

/* Convert boolean to string */
#define __BOOL2STR(X) ((X) ? "TRUE" : "FALSE")

/* Convert string to boolean */
#define __STR2BOOL(X) (0 == Quos_strncasecmp(X, "TRUE", __GET_MAX(Quos_strlen(X), Quos_strlen("TRUE"))) ? TRUE : FALSE)

/* Convert NULL string to empty string */
#define __STR2PRINT(X) (NULL == X ? "" : X)

/* Extract a specific bit from data and convert it to boolean */
#define __DATA2BOOL(X, Y) (((X >> Y) & 1) ? TRUE : FALSE)

/* Define macros to convert arrays of bytes into integers of various bit lengths and byte orders */
#define _ARRAY01_U16(ARRAY) (((quint16_t)(((quint8_t *)(ARRAY))[0]) << 8) | \
							 ((quint16_t)(((quint8_t *)(ARRAY))[1]) << 0))

#define _ARRAY10_U16(ARRAY) (((quint16_t)(((quint8_t *)(ARRAY))[1]) << 8) | \
							 ((quint16_t)(((quint8_t *)(ARRAY))[0]) << 0))

#define _ARRAY012_U24(ARRAY) (((quint32_t)(((quint8_t *)(ARRAY))[0]) << 16) | \
							  ((quint32_t)(((quint8_t *)(ARRAY))[1]) << 8) |  \
							  ((quint32_t)(((quint8_t *)(ARRAY))[2]) << 0))

#define _ARRAY0123_U32(ARRAY) (((quint32_t)(((quint8_t *)(ARRAY))[0]) << 24) | \
							   ((quint32_t)(((quint8_t *)(ARRAY))[1]) << 16) | \
							   ((quint32_t)(((quint8_t *)(ARRAY))[2]) << 8) |  \
							   ((quint32_t)(((quint8_t *)(ARRAY))[3]) << 0))

#define _ARRAY1032_U32(ARRAY) (((quint32_t)(((quint8_t *)(ARRAY))[1]) << 24) | \
							   ((quint32_t)(((quint8_t *)(ARRAY))[0]) << 16) | \
							   ((quint32_t)(((quint8_t *)(ARRAY))[3]) << 8) |  \
							   ((quint32_t)(((quint8_t *)(ARRAY))[2]) << 0))

#define _ARRAY3210_U32(ARRAY) (((quint32_t)(((quint8_t *)(ARRAY))[3]) << 24) | \
							   ((quint32_t)(((quint8_t *)(ARRAY))[2]) << 16) | \
							   ((quint32_t)(((quint8_t *)(ARRAY))[1]) << 8) |  \
							   ((quint32_t)(((quint8_t *)(ARRAY))[0]) << 0))

#define _ARRAY2301_U32(ARRAY) (((quint32_t)(((quint8_t *)(ARRAY))[2]) << 24) | \
							   ((quint32_t)(((quint8_t *)(ARRAY))[3]) << 16) | \
							   ((quint32_t)(((quint8_t *)(ARRAY))[0]) << 8) |  \
							   ((quint32_t)(((quint8_t *)(ARRAY))[1]) << 0))

#define _ARRAY76543210_U64(ARRAY) (((quint64_t)(((quint8_t *)(ARRAY))[7]) << 56) | \
								   ((quint64_t)(((quint8_t *)(ARRAY))[6]) << 48) | \
								   ((quint64_t)(((quint8_t *)(ARRAY))[5]) << 40) | \
								   ((quint64_t)(((quint8_t *)(ARRAY))[4]) << 32) | \
								   ((quint64_t)(((quint8_t *)(ARRAY))[3]) << 24) | \
								   ((quint64_t)(((quint8_t *)(ARRAY))[2]) << 16) | \
								   ((quint64_t)(((quint8_t *)(ARRAY))[1]) << 8) |  \
								   ((quint64_t)(((quint8_t *)(ARRAY))[0]) << 0))

#define _ARRAY012345678_U64(ARRAY) (((quint64_t)(((quint8_t *)(ARRAY))[0]) << 56) | \
									((quint64_t)(((quint8_t *)(ARRAY))[1]) << 48) | \
									((quint64_t)(((quint8_t *)(ARRAY))[2]) << 40) | \
									((quint64_t)(((quint8_t *)(ARRAY))[3]) << 32) | \
									((quint64_t)(((quint8_t *)(ARRAY))[4]) << 24) | \
									((quint64_t)(((quint8_t *)(ARRAY))[5]) << 16) | \
									((quint64_t)(((quint8_t *)(ARRAY))[6]) << 8) |  \
									((quint64_t)(((quint8_t *)(ARRAY))[7]) << 0))

#define _U16_ARRAY01(INT, ARRAY) (((quint8_t *)ARRAY)[0] = ((INT) >> 8) & 0xFF, ((quint8_t *)ARRAY)[1] = (INT) & 0xFF)
#define _U16_ARRAY10(INT, ARRAY) (((quint8_t *)ARRAY)[0] = (INT) & 0xFF, ((quint8_t *)ARRAY)[1] = ((INT) >> 8) & 0xFF)
#define _U32_ARRAY0123(INT, ARRAY) (((quint8_t *)ARRAY)[0] = ((INT) >> 24) & 0xFF, ((quint8_t *)ARRAY)[1] = ((INT) >> 16) & 0xFF, ((quint8_t *)ARRAY)[2] = ((INT) >> 8) & 0xFF, ((quint8_t *)ARRAY)[3] = (INT) & 0xFF)
#define _U32_ARRAY3210(INT, ARRAY) (((quint8_t *)ARRAY)[0] = (INT) & 0xFF, ((quint8_t *)ARRAY)[1] = ((INT) >> 8) & 0xFF, ((quint8_t *)ARRAY)[2] = ((INT) >> 16) & 0xFF, ((quint8_t *)ARRAY)[3] = ((INT) >> 24) & 0xFF)
#define _U64_ARRAY01234567(INT, ARRAY) (((quint8_t *)ARRAY)[0] = ((INT) >> 56) & 0xFF, ((quint8_t *)ARRAY)[1] = ((INT) >> 48) & 0xFF, ((quint8_t *)ARRAY)[2] = ((INT) >> 40) & 0xFF, ((quint8_t *)ARRAY)[3] = ((INT) >> 32) & 0xFF, \
										((quint8_t *)ARRAY)[4] = ((INT) >> 24) & 0xFF, ((quint8_t *)ARRAY)[5] = ((INT) >> 16) & 0xFF, ((quint8_t *)ARRAY)[6] = ((INT) >> 8) & 0xFF, ((quint8_t *)ARRAY)[7] = ((INT) >> 0) & 0xFF)
#define _U64_ARRAY76543210(INT, ARRAY) (((quint8_t*)ARRAY)[0] = (INT)&0xFF,((quint8_t*)ARRAY)[1] = ((INT)>>8)&0xFF,((quint8_t*)ARRAY)[2] = ((INT)>>16)&0xFF,((quint8_t*)ARRAY)[3] = ((INT)>>24)&0xFF, \
                                (((quint8_t*)ARRAY)[4] = ((INT)>>32)&0xFF,((quint8_t*)ARRAY)[5] = ((INT)>>40)&0xFF,((quint8_t*)ARRAY)[6] = ((INT)>>48)&0xFF,((quint8_t*)ARRAY)[7] = ((INT)>>56)&0xFF)

#define QUOS_IP2STR "%hhu.%hhu.%hhu.%hhu"
#define QUOS_IP2STR_(IP) (quint8_t)((IP) >> 24), (quint8_t)((IP) >> 16), (quint8_t)((IP) >> 8), (quint8_t)((IP) >> 0)

#define QUOS_TIME_SEC2UTC "%08u.%02u.%02u"
#define QUOS_TIME_SEC2UTC_(SEC) (quint32_t)(SEC / 3600), (quint8_t)(SEC % 3600 / 60), (quint8_t)(SEC % 60)

#ifndef INT_MAX
#define INT_MAX (2147483647)
#endif
#ifndef INT_MIN
#define INT_MIN (-INT_MAX - 1)
#endif
#ifndef DBL_EPSILON
#define DBL_EPSILON (2.2204460492503131e-016)
#endif

#define QUOS_CPL_BIT(value, bit) ((value) ^= (1 << (bit)))	// Toggle the specified bit in value
#define QUOS_SET_BIT(value, bit) ((value) |= (1 << (bit)))	// Set the specified bit in value
#define QUOS_CLR_BIT(value, bit) ((value) &= ~(1 << (bit))) // Clear the specified bit in value
#define QUOS_GET_BIT(value, bit) ((value) & (1 << (bit)))	// Get the value of the specified bit

#ifndef QUOS_DNS_HOSTNANE_MAX_LENGHT
#define QUOS_DNS_HOSTNANE_MAX_LENGHT (64) /* Maximum length for DNS hostname as per DNS regulations (63 characters max) */
#endif

#ifndef QUOS_IP_ADDR_MAX_LEN
#define QUOS_IP_ADDR_MAX_LEN (46) /* Maximum length for an IP address string (e.g., IPv6 can be up to 45 characters) */
#endif

	/**
	 * Quos_urlAnalyze_t is a structure for parsing and storing URL information.
	 * It contains information about the URL, including the path, hostname, security protocol, and port.
	 */
	typedef struct
	{
		// Pointer to the character string storing the path of the URL
		char *path;
		// Array of characters storing the hostname, with a maximum length defined by QUOS_DNS_HOSTNANE_MAX_LENGHT
		char domain[QUOS_DNS_HOSTNANE_MAX_LENGHT + 1];
		// Boolean value indicating whether the connection is secure (usually HTTPS)
		qbool isSecure;
		// 16-bit unsigned integer storing the port number of the URL
		quint16_t port;
	} Quos_urlAnalyze_t;

	typedef enum
	{
		QUOS_CHAR_09 = (1 << 0),
		QUOS_CHAR_af = (1 << 1),
		QUOS_CHAR_gz = (1 << 2),
		QUOS_CHAR_AF = (1 << 3),
		QUOS_CHAR_GZ = (1 << 4),
		QUOS_CHAR_DOT = (1 << 5),
		QUOS_CHAR_MINUS = (1 << 6),
	} Quos_randomType_e;

	/**
	 * Converts a 64-bit unsigned integer to a byte array.
	 *
	 * This function aims to convert a given 64-bit unsigned integer into a variable-length byte array,
	 * omitting leading zero bytes. It first converts the integer into a temporary fixed-length byte array,
	 * then removes the leading zero bytes, and finally moves the remaining bytes to the output array.
	 *
	 * @param intValue The 64-bit unsigned integer to be converted.
	 * @param array Pointer to the output byte array.
	 * @return Returns the length of the byte array after conversion.
	 */
	quint32_t Quos_uint2Array(quint64_t intValue, quint8_t *array);

	/**
	 * Converts a string to an unsigned integer.
	 *
	 * This function attempts to convert a given string into a quint32_t unsigned integer. It first checks if the input string meets basic validity conditions,
	 * then converts each character to a digit and accumulates it to form the final unsigned integer value. If the conversion is successful and a valid
	 * value pointer is provided, the result is stored at the address pointed to by value.
	 *
	 * @param src Pointer to the string to be converted.
	 * @param len Length of the string.
	 * @param value Pointer to the variable where the converted unsigned integer is stored. If not needed, it can be set to NULL.
	 * @return Returns TRUE if the conversion is successful, otherwise returns FALSE.
	 *
	 * Note: The function checks for NULL pointers, empty strings, and strings that start with a '-' as invalid input. It also checks for overflow during the conversion process.
	 */
	qbool Quos_str2Uint(const char *src, quint32_t len, quint32_t *value);

	/**
	 * Converts a string to a 32-bit integer.
	 *
	 * This function attempts to convert the string pointed to by `src` into a 32-bit integer, storing the result in `value`.
	 * It allows for leading spaces and an optional sign ('+' or '-') before the digits.
	 * The function ensures that the input string represents a valid integer within the range of a 32-bit signed integer.
	 *
	 * @param src Pointer to the string to be converted.
	 * @param len Length of the string.
	 * @param value Pointer to the integer variable where the result will be stored.
	 * @return Returns TRUE if the conversion is successful, otherwise returns FALSE.
	 */
	qbool Quos_str2Int(const char *src, quint32_t len, qint32_t *value);

	/**
	 * Converts a string of hexadecimal characters to a byte array.
	 *
	 * This function takes a string representing hexadecimal characters and converts it into a byte array.
	 * The string can include characters '0'-'9' and 'A'-'F' or 'a'-'f'. If the string contains invalid characters,
	 * the function returns 0. The function processes two hexadecimal characters at a time to form one byte,
	 * as one hexadecimal character represents 4 bits.
	 *
	 * @param srcStr Pointer to the source string of hexadecimal characters.
	 * @param retHex Pointer to the byte array where the converted result is stored.
	 * @return Returns the length of the converted byte array, or 0 if the conversion fails.
	 */
	quint32_t Quos_str2Hex(void *srcStr, quint8_t retHex[]);

	/**
	 * Converts a hexadecimal number to a string.
	 *
	 * @param hex Pointer to the hexadecimal number to be converted.
	 * @param hexLen Length of the hexadecimal number.
	 * @param retStr Pointer to the converted string.
	 * @param isUpper Flag to determine if the letters in the converted string should be uppercase.
	 * @return The length of the converted string.
	 *
	 * This function iterates through each byte of the input hexadecimal number, converting it to two characters,
	 * and appending it to the return string. The conversion uses either lowercase or uppercase letters based on the isUpper flag.
	 */
	quint32_t Quos_hex2Str(quint8_t hex[], quint32_t hexLen, char *retStr, qbool isUpper);

	/**
	 * Converts a double precision floating point number to a string representation.
	 *
	 * @param strBuf A pointer to the buffer where the converted string is stored.
	 * @param maxSize The maximum size of the buffer, to prevent buffer overflow.
	 * @param value The double precision floating point number to be converted.
	 * @return Returns the length of the converted string.
	 *
	 * This function first determines if the input value is negative, and if so, writes a '-' sign to the buffer and converts the value to its absolute value.
	 * It then calculates the number of decimal places (amp) for the fractional part and converts the integer part of the value to a string.
	 * If there are decimal places, it converts them to a string and appends them to the buffer, followed by appending the fractional part.
	 * Finally, it terminates the string with a null character and returns the length of the string.
	 */
	quint32_t Quos_double2String(char *strBuf, quint32_t maxSize, double value);

	/**
	 * Converts a 64-bit unsigned integer to a string representation.
	 *
	 * This function takes a buffer and the maximum size of the buffer, along with a 64-bit unsigned integer value,
	 * and converts the integer into a string representation, storing it in the buffer.
	 *
	 * @param strBuf Pointer to the character array where the result will be stored.
	 * @param maxSize Maximum size of the strBuf buffer.
	 * @param value The 64-bit unsigned integer value to be converted.
	 * @return Returns the length of the converted string, excluding the null terminator.
	 *
	 * Note: This function assumes that the buffer size is large enough to store the converted string and the null terminator.
	 */
	quint32_t Quos_uint2String(char *strBuf, quint32_t maxSize, quint64_t value);

	/**
	 * Calculates the CRC (Cyclic Redundancy Check) value for a given data buffer.
	 *
	 * This function is used to calculate the CRC value of a data block to ensure data integrity.
	 * It starts from an initial CRC value and adds the byte values in the data block to update the CRC value.
	 *
	 * @param crc The initial CRC value, usually starting at 0.
	 * @param buf Pointer to the data buffer, which contains the data to be used for calculating the CRC.
	 * @param len The length of the data buffer, indicating the amount of data to be processed.
	 * @return Returns the updated CRC value.
	 *
	 * Note: This function assumes that the input CRC value is correct and that the data buffer is accessible.
	 * The function does not handle data buffer access errors or other exceptions.
	 */
	qbool Quos_urlAnalyze(const char *url, Quos_urlAnalyze_t *result);

	/**
	 * Calculates the CRC (Cyclic Redundancy Check) value for a given data buffer.
	 *
	 * This function is used to calculate the CRC value of a data block to ensure data integrity.
	 * It starts from an initial CRC value and adds the byte values in the data block to update the CRC value.
	 *
	 * @param crc The initial CRC value, usually starting at 0.
	 * @param buf Pointer to the data buffer, which contains the data to be used for calculating the CRC.
	 * @param len The length of the data buffer, indicating the amount of data to be processed.
	 * @return Returns the updated CRC value.
	 *
	 * Note: This function assumes that the input CRC value is correct and that the data buffer is accessible.
	 * The function does not handle data buffer access errors or other exceptions.
	 */
	quint32_t Quos_crcCalculate(quint32_t crc, const void *buf, quint32_t len);

	/**
	 * Calculates the MPEG-2 CRC checksum for a given data block.
	 *
	 * This function implements the MPEG-2 CRC algorithm, which is used to calculate and verify the integrity of data blocks.
	 * It iterates through each byte of the input data, updates the CRC checksum using a CRC table, and finally returns the calculated CRC value.
	 *
	 * @param crcDefault The initial value of the CRC checksum, usually starting from 0xFFFFFFFF.
	 * @param data A pointer to the data block for which the CRC checksum is to be calculated.
	 * @param len The length of the data block in bytes.
	 * @return Returns the calculated MPEG-2 CRC checksum value.
	 */
	quint32_t Quos_calcCrc_MPEG2(quint32_t crcDefault, const quint8_t *data, quint32_t len);

	/**
	 * @brief Generates random data based on the specified type and length.
	 *
	 * This function generates random data of a specified type and length, placing the results in a buffer pointed to by the data parameter.
	 * The type of data generated is determined by the type parameter, which decides the range and base of the random data through bit operations.
	 *
	 * @param data Pointer to the buffer where the generated random data is stored.
	 * @param len Length of the random data to generate.
	 * @param type Type of data to generate, which determines the range and base of the random data through bit operations.
	 */
	void Quos_randomGen(void *data, quint32_t len, quint8_t type);

	/**
	 * Splits a string into an array of substrings based on a specified delimiter.
	 *
	 * This function divides the source string into multiple substrings according to the given delimiter and stores these substrings in the words array.
	 * It supports custom delimiters and can control whether to retain empty strings that appear between consecutive delimiters.
	 *
	 * @param src Pointer to the source string to be split.
	 * @param srcLen Length of the source string.
	 * @param words Pointer to the array of strings where the split substrings will be stored.
	 * @param maxSize Maximum number of substrings that can be stored in the words array.
	 * @param delim Pointer to the delimiter string.
	 * @param keepEmptyParts Flag indicating whether to retain empty strings between consecutive delimiters, TRUE to retain, FALSE otherwise.
	 * @return Returns the actual number of substrings obtained after splitting.
	 *
	 * Note: This function does not handle cases where the source string or delimiter is NULL, or where the source string length is 0, returning 0 in such cases.
	 */
	quint32_t Quos_stringSplit(char *src, quint32_t srcLen, char **words, quint32_t maxSize, const char *delim, qbool keepEmptyParts);

	/**
	 * Checks if the given string meets specific character type criteria.
	 *
	 * This function iterates through each character in the string to check if it matches the specified character types.
	 * The specified character types are represented by bit flags in the 'type' parameter. If any character does not match,
	 * the function immediately returns FALSE. If all characters match, it returns TRUE.
	 *
	 * @param data A pointer to the string to be checked. The string is considered as a binary data sequence.
	 * @param len The length of the string, in bytes.
	 * @param type A bit mask representing the allowed character types. Different bit positions represent different character types.
	 *
	 * @return Returns TRUE if all characters in the string match the specified types, otherwise returns FALSE.
	 */
	qbool Quos_stringCheck(const void *data, quint32_t len, quint8_t type);

	/**
	 * Determines if a given double-precision floating-point value is not an integer.
	 *
	 * This function checks if the given value is a non-integer. It first takes the absolute value of the input to handle negative numbers,
	 * then checks if the value is equal to its integer cast. If they are equal, it means the original value was an integer, and the function
	 * returns FALSE; otherwise, it returns TRUE.
	 *
	 * @param value The double-precision floating-point value to be checked.
	 * @return Returns a boolean value, TRUE if the value is not an integer, FALSE otherwise.
	 */
	qbool Quos_numIsDouble(double value);

	/**
	 * Calculates the number of decimal places of a given floating-point number.
	 *
	 * This function aims to determine the number of decimal places of a floating-point number as accurately as possible.
	 * It first extracts the integer part of the number, then calculates the number of digits in the integer part.
	 * After that, it calculates the number of decimal places based on the precision of the floating-point number.
	 *
	 * @param value The floating-point number whose number of decimal places is to be calculated.
	 * @return Returns the number of decimal places of the floating-point number.
	 */
	quint8_t Quos_decimalDigits(double value);

	/**
	 * Calculates the MD5 checksum of a file.
	 *
	 * This function opens a file in read-only mode, calculates its MD5 checksum in chunks, and returns the checksum as a hexadecimal string.
	 * It uses the Quos MD5 initialization, update, and completion functions to calculate the checksum, and uses the Qhal file operation functions to read the file.
	 *
	 * @param filename The name of the file for which to calculate the MD5 checksum.
	 * @param fileLen The length of the file. If the file length is known, it can be provided here; otherwise, it can be obtained internally.
	 * @param md5 An array of characters used to store the calculated MD5 checksum string.
	 * @return Returns TRUE if the operation is successful, otherwise returns FALSE.
	 */
	qbool Quos_fileMd5(const char *filename, quint32_t fileLen, char md5[]);

	/**
	 * @brief Inserts a key-value pair into a given string.
	 *
	 * This function attempts to insert a key-value pair into a source string. If the key already exists, it updates the value.
	 * If the key does not exist, it appends the key-value pair to the end of the source string.
	 *
	 * @param srcStr Pointer to the source string where the key-value pair will be inserted.
	 * @param maxLen Maximum length of the source string.
	 * @param keyword Pointer to the key string to be inserted or updated.
	 * @param separator Pointer to the separator string between the key and value.
	 * @param value Pointer to the value string to be inserted or updated.
	 * @param endStr Pointer to the end string marker, used to identify the end of a key-value pair.
	 * @return qbool Returns TRUE if the operation is successful, otherwise returns FALSE.
	 */
	qbool Quos_keyValueInsert(char *srcStr, quint32_t maxLen, const char *keyword, const char *separator, const char *value, const char *endStr);

	/**
	 * Extracts the value associated with a specific keyword from a source string.
	 *
	 * This function searches for a keyword within the source string and extracts the value associated with it.
	 * The keyword and value are separated by a specified separator. The search can be limited by specifying an end string.
	 *
	 * @param srcStr Pointer to the source string from which to extract the value.
	 * @param keyword Pointer to the keyword to search for.
	 * @param separator Pointer to the string used as a separator.
	 * @param dstStr Pointer to a pointer where the extracted value will be stored.
	 * @param endStr Pointer to the end string, used to limit the search.
	 * @return Returns the length of the extracted value if successful, -1 if an error occurs or no matching keyword is found.
	 */
	qint32_t Quos_keyValueExtract(char *srcStr, const char *keyword, const char *separator, char **dstStr, const char *endStr);

	/**
	 * Partially obscures a string by replacing part of it with asterisks.
	 * This function is used to hide part of a sensitive string for security purposes,
	 * such as hiding part of a password or personal information.
	 *
	 * @param dst The destination string buffer, which must be pre-allocated and large enough to hold the processed string.
	 * @param src The source string to be processed.
	 * @param len The length of the source string.
	 *
	 * Note:
	 * - The function divides the source string into three parts: the beginning, middle, and end.
	 * - The first quarter and the last quarter of the string remain unchanged, while the middle half is replaced with asterisks.
	 * - This approach ensures that part of the string is hidden while still allowing for some form of verification or reference.
	 */
	void Quos_string2Scrt(char *dst, const char *src, quint32_t len);

	/**
	 * Removes enclosing quotation marks from the provided string if present.
	 *
	 * @param strVal A pointer to the null-terminated string to be processed.
	 *
	 * This function checks if the first and last characters of the string are double quotes ("").
	 * If so, it shifts all characters one position to the left, effectively removing the enclosing quotes,
	 * and terminates the string with a null character in the new position.
	 *
	 * @return Returns a pointer to the modified string. If no enclosing quotes are found,
	 *         the original string is returned unchanged.
	 */
	char *Quos_stringRemoveMarks(char *strVal);

	/**
	 * Converts a number to a specified base and returns the highest digit position.
	 *
	 * This function aims to convert a given number value into a representation under a specified base,
	 * and returns the position of the highest digit in that base representation. If the base is less than
	 * or equal to 1, the function returns 0, indicating the conversion cannot be performed.
	 *
	 * @param value The number to be converted.
	 * @param base The base to convert to.
	 * @return quint8_t The number of digits needed minus 1:
	 *         - Returns 0 if base ≤ 1 (invalid input) or value = 0
	 *         - Returns count-1 for other valid inputs
	 */
	quint8_t Quos_convertToBase(quint32_t value, quint32_t base);

	/**
	 * Converts an IP address string to its integer representation.
	 *
	 * This function takes a string representing an IP address and converts it to a 32-bit integer.
	 * The purpose of this conversion is to facilitate the handling and comparison of IP addresses in network programming.
	 *
	 * @param ipStr A string representing the IP address, e.g., "192.168.1.1".
	 * @return Returns the integer representation of the IP address if the conversion is successful, otherwise returns 0.
	 *
	 * Note: The function assumes the input is a standard IPv4 address string, containing four dot-separated decimal numbers.
	 */
	quint32_t Quos_ip2Int(const char *ipStr);

    /**
     * @brief : Convert string case (uppercase or lowercase)
     * @param [in] str: String to be converted
     * @param [in] toUpperCase: TRUE to convert to uppercase, FALSE to convert to lowercase
     * @return : Pointer to the converted string
     * @note: This function converts letters in the string to uppercase or lowercase
     *        without using standard library API, and modifies the string in place.
     */
    char *Quos_stringCase(char *str, qbool toUpperCase);

#ifdef __cplusplus
}
#endif

#endif /* __QUOS_TOOLS_H__ */
