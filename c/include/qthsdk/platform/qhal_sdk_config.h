#ifndef __QHAL_SDK_CONFIG_H__
#define __QHAL_SDK_CONFIG_H__
#include "qthsdk_config.h"

/**************************************** quecthing function switch start *********************************************/
#ifndef QTH_ENABLE_LOCIN
#define QTH_ENABLE_LOCIN            1 /* 设备内置定位功能 */
#endif
#ifndef QTH_ENABLE_BINDCODE
#define QTH_ENABLE_BINDCODE         1 /* 设备绑定功能 */
#endif
#ifndef QTH_ENABLE_DKDS
#define QTH_ENABLE_DKDS             1 /* 本地修改dk ds功能 */
#endif
#ifndef QTH_ENABLE_RST
#define QTH_ENABLE_RST              1 /* 设备重置功能 */
#endif
#ifndef QTH_ENABLE_SESSION
#define QTH_ENABLE_SESSION          1 /* 通信数据协议层加密功能 */
#endif
#ifndef QTH_ENABLE_GATEWAY
#define QTH_ENABLE_GATEWAY          1 /* 网关功能 */
#endif
#if (!defined QTH_ENABLE_MODBUS) && (!defined QTH_ENABLE_AT)
#define QTH_ENABLE_MODBUS           1 /* MODBUS功能 */
#endif
#ifndef QTH_ENABLE_QTH_OTA
#define QTH_ENABLE_QTH_OTA          1 /* FOTA下载服务源，1：Quecthing,0：模组自身 */
#endif
#ifndef QTH_ENABLE_NTP_REQ
#define QTH_ENABLE_NTP_REQ          1 /* NTP功能 */
#endif
#ifndef QTH_ENABLE_ODOS
#define QTH_ENABLE_ODOS             1 /* 一机一密功能 */
#endif
#ifndef QTH_ENABLE_DATACOLL
#define QTH_ENABLE_DATACOLL         1 /* 数采功能 */   
#endif
#ifndef QTH_ENABLE_BOOTSTRAP
#define QTH_ENABLE_BOOTSTRAP        1 /* 域名引导功能 */
#endif
#ifndef QTH_ENABLE_TLS_MODE         
#define QTH_ENABLE_TLS_MODE         0 /*0-x509 认证 1-psk 认证*/
#endif

#ifndef CONFIG_KERNEL_ENABLE_IPCACHE         
#define CONFIG_KERNEL_ENABLE_IPCACHE   1 /* 是否支持DNS IP缓存 */
#endif



/**************************************** quecthing function switch end ***********************************************/

/************************************** quecthing base function switch *************************************************/
#define QTH_ENABLE_TSL              1   /* 物模型功能 */
#define QTH_ENABLE_PASSTRANS        1   /* 透传数据功能 */
#define QTH_ENABLE_OTA              1   /* OTA 功能 */
#define CONFIG_PLATFORM_ENABLE_BLE  0   /* BLE功能，0：禁用，1：启用 */
/*******************************************************************************************************************/

#endif/*__QHAL_SDK_CONFIG_H__*/
