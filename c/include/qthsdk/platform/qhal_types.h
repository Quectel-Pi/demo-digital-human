#ifndef __QHAL_TYPES_H__
#define __QHAL_TYPES_H__
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "qhal_sdk_config.h"

#define UNUSED(x) (void)(x)

typedef uint8_t quint8_t;
typedef int8_t qint8_t;
typedef uint16_t quint16_t;
typedef int16_t qint16_t;
typedef uint32_t quint32_t;
typedef int32_t qint32_t;
typedef uint64_t quint64_t;
typedef int64_t qint64_t;
typedef long int pointer_t;
typedef enum{FALSE=0,TRUE=1}qbool;
typedef qint32_t oprt_ret;

#define PRINTF_FD       "%ld"

#define FUNCTION_ATTR_ROM
#define FUNCTION_ATTR_WEAK __attribute__((weak))
#define HAL_SPRINTF     sprintf
#define HAL_SNPRINTF snprintf
#define HAL_VSNPRINTF vsnprintf /* TODO:后续需要移除 */

#endif /*__QHAL_TYPES_H__*/
