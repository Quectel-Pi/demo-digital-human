/**
 * @file qth_config.h
 * @author niko.huang
 * @version 0.0.1
 * @date 2023.03.07
 * @brief The implements of DMP config
 * @copyright Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 */

#ifndef __QTH_CONFIG_H__
#define __QTH_CONFIG_H__

#include "quos_tools.h"
#include "qhal_types.h"
#include "qhal_sdk_config.h"
#if (1 == CONFIG_CLOUD_ENABLE_DMPOTA)
#include "qth_dmpOta.h"
#endif /* CONFIG_CLOUD_ENABLE_DMPOTA */
#ifdef __cplusplus
extern "C"
{
#endif

/** @defgroup QTH_CONFIG_MACROS Configuration Macros
 *  @{ */

/** @brief Maximum size of product key */
#define QTH_PK_MAXSIZE (32)
/** @brief Maximum size of product secret */
#define QTH_PS_MAXSIZE (32)
/** @brief Minimum size of device key */
#define QTH_DK_MINSIZE (4)
/** @brief Maximum size of device key */
#define QTH_DK_MAXSIZE (32)
/** @brief Maximum size of device secret */
#define QTH_DS_MAXSIZE (32)
/** @brief Maximum size of user device key */
#define QTH_DK_USER_MAXSIZE (16)
/** @brief Size of binding secret */
#define QTH_BINDINGSECRET_SIZE (16)
/** @brief Size of binding code */
#define QTH_BINDINGCODE_SIZE (16)
/** @brief Size of binding key */
#define QTH_BINDINGKEY_SIZE (16)
/** @brief Maximum size of M2M client ID */
#define QTH_M2M_CLIENTID_MAXSIZE (2 + QTH_PK_MAXSIZE + QTH_DK_MAXSIZE)
/** @brief Maximum size of component number */
#define QTH_COMPNO_MAXSIZE (32)
/** @brief Maximum size of component version */
#define QTH_COMPVER_MAXSIZE (64)
/** @brief Maximum size of WiFi SSID */
#define QTH_WIFI_SSID_MAXSIZE (32)
/** @brief Maximum size of WiFi password */
#define QTH_WIFI_PWD_MAXSIZE (64)
/** @brief Maximum size of PSK ID */
#define QTH_PSK_ID_MAXSIZE (32)
/** @brief Maximum size of PSK secret */
#define QTH_PSK_SECRET_MAXSIZE (32)
/** @brief Size of provision api key */
#define QTH_APIKEY_MAXSIZE (64)

/** @brief Default MQTT lifetime in seconds */
#define QTH_DEFAULT_LIFETIME 120
/** @brief Default TLS server URL */
#define QTH_DEFAULT_SERVERURL_TLS "mqtts://uat-iot-south.quectelcn.com:8886"
/** @brief Default server URL */
#define QTH_DEFAULT_SERVERURL "mqtt://uat-iot-south.quectelcn.com:1883"
/** @brief Default server IP address */
#define QTH_DEFAULT_SERVERIP "139.224.51.71"
/** @brief Default WiFi network configuration enable status */
#define QTH_DEFAULT_WIFI_NETCFG_ENABLE TRUE
/** @brief Default WiFi network configuration timeout in seconds */
#define QTH_DEFAULT_WIFI_NETCFG_TIMEOUT 300
/** @brief Maximum length of BLE custom advertising data */
#define QTH_BLE_MAX_CUST_ADV_DATA_LEN (2)
/** @brief Maximum number of BLE custom configurations */
#define QTH_BLE_MAX_CUSTOM 5

    /** @} */

    /** @defgroup QTH_CONFIG_ENUMS Configuration Enumerations
     *  @{ */

    /**
     * @brief WiFi configuration states
     */
    enum
    {
        QTH_WIFI_CONFIG_ENTER = 0,   /*!< Enter WiFi configuration mode */
        QTH_WIFI_CONFIG_RECV = 1,    /*!< Receive WiFi configuration information */
        QTH_WIFI_CONFIG_CONNECT = 2, /*!< Connected to the configuration router */
        QTH_WIFI_CONFIG_BIND = 3,    /*!< User completes the device binding */
        QTH_WIFI_CONFIG_EXIT = 4,    /*!< Exit WiFi configuration mode */
    };

    /**
     * @brief Dev reset type
     */
    enum
    {
        QTH_DEV_RESET_ONLINE = 0,    /*!< When the user resets the device, the device is online and has received a result */
        QTH_DEV_RESET_OFFLINE = 1,   /*!< When the user resets the device, it is offline and will receive a result when reconnecting to the cloud platform */
    };

    /**
     * @brief Dev remove type
     */
    enum
    {
        QTH_DEV_REMOVE_ONLINE = 0,  /*!< When the app removes the device, the device is online and has received a notification */
        QTH_DEV_REMOVE_OFFLINE = 1, /*!< When the application deleted the device, it was offline and a notification was sent when the device was re added */
    };

    /**
     * @brief BLE connection events
     */
    enum
    {
        QTH_BLE_CONNECT_EVENT = 0,     /*!< Connected to BLE */
        QTH_BLE_DISCONNECT_EVENT = 1,  /*!< Disconnected from BLE */
        QTH_BLE_BINDING_EVENT = 2,     /*!< BLE binding event */
        QTH_BLE_LOGIN_EVENT = 3,       /*!< BLE login event */
        QTH_BLE_TX_DONE = 4,           /*!< BLE transmission completed */
        QTH_BLE_BINDING_DMP_EVENT = 5, /*!< User successfully bound the device through DMP, applicable when external network is available */
    };

    /**
     * @brief Device event types
     */
    typedef enum
    {
        QTH_DEV_EVENT_WIFI_CONFIG = 0,  /*!< Device configure router events */
        QTH_DEV_EVENT_WIFI_CONNECT = 1, /*!< Device connects router events */
        QTH_DEV_EVENT_DMP_CONNECT = 2,  /*!< Device connects DMP events */
        QTH_DEV_EVENT_RESET = 3,        /*!< Device reset events */
        QTH_DEV_EVENT_REMOVE = 4,       /*!< Device remove events */
        QTH_DEV_EVENT_SLIENT = 5,       /*!< Device enter silent mode */
        QTH_DEV_EVENT_BLE = 6,          /*!< Device BLE connection events */
        QTH_DEV_EVENT_ETH_CONNECT = 7,  /*!< Device connects Ethernet events */
        QTH_DEV_EVENT_LTE_CONNECT = 9,  /*!< Device connects LTE events */
    } Qth_devEvent_e;

    typedef enum
    {
        QTH_PDP_TYPE_LTE = 0,  /*!< The PDP network context type is LTE */
        QTH_PDP_TYPE_WIFI = 1, /*!< The PDP network context type is WIFI */
        QTH_PDP_TYPE_ETH = 2,  /*!< The PDP network context type is ETH */
    } Qth_pdpType_e;

    /**
     * @brief Device authentication types
     */
    typedef enum
    {
        QTH_AUTHTYPE_OTOS = 0, /*!< One type one secret authentication */
        QTH_AUTHTYPE_ODOS = 1, /*!< One device one secret authentication */
        QTH_AUTHTYPE_ODOC = 2, /*!< One device one certificate authentication */
    } Qth_authType_e;

    /** @} */

    /** @defgroup QTH_CONFIG_CALLBACKS Configuration Callbacks
     *  @{ */

    /**
     * @brief Device event notification callback
     * @param[in] event Device event type
     * @param[in] oprt_ret Result equals 0 indicates successful operation, non-zero returns the corresponding error code
     * @return None
     */
    typedef void (*Qth_devEventCb_f)(Qth_devEvent_e event, oprt_ret result);

    /**
     * @brief TTLV data receive notification callback
     * @param[in] ttlvHead TTLV request data
     * @return None
     */
    typedef void (*Qth_ttlvRecvCb_f)(void *ttlvHead);

    /**
     * @brief TTLV data read notification callback
     * @param[in] id TTLV ID list
     * @param[in] idNum TTLV ID number
     * @return TTLV response data
     */
    typedef void *(*Qth_ttlvReqCb_f)(quint16_t id[], quint32_t idNum);

    /**
     * @brief Enhanced TTLV data read notification callback
     * @param[in] id TTLV ID list
     * @param[in] idNum TTLV ID number
     * @param[in] pkgId Package ID
     * @return TTLV response data
     */
    typedef void *(*Qth_ttlvReqCb2_f)(quint16_t id[], quint32_t idNum, quint16_t pkgId);

    /**
     * @brief TTLV server data receive notification callback
     * @param[in] ttlvHead TTLV request data
     * @param[in] pkgId Package ID
     * @return TTLV response data, return NULL means no response
     */
    typedef void *(*Qth_ttlvRecvServerCb_f)(void *ttlvHead, quint16_t pkgId);

    /**
     * @brief Transparent data receive notification callback
     * @param[in] value Transparent data value
     * @param[in] valLen Transparent data length
     * @return None
     */
    typedef void (*Qth_transRecvCb_f)(quint8_t *value, quint32_t valLen);

#if (1 == CONFIG_CLOUD_ENABLE_DMPRRPC)
    /**
     * @brief TTLV data receive notification callback
     * @param[in] ttlvHead TTLV request data
     * @return None
     */
    typedef void (*Qth_ttlvRecvRrpcCb_f)(void *ttlvHead, char *msgid);

    /**
     * @brief Enhanced TTLV data read notification callback
     * @param[in] id TTLV ID list
     * @param[in] idNum TTLV ID number
     * @param[in] pkgId Package ID
     * @return TTLV response data
     */
    typedef void (*Qth_ttlvReqRrpcCb_f)(quint16_t id[], quint32_t idNum, quint16_t pkgId, char *msgid);

    /**
     * @brief TTLV server data receive notification callback
     * @param[in] ttlvHead TTLV request data
     * @param[in] pkgId Package ID
     * @return TTLV response data, return NULL means no response
     */
    typedef void (*Qth_ttlvRecvRrpcServerCb_f)(void *ttlvHead, quint16_t pkgId, char *msgid);

#endif /* CONFIG_CLOUD_ENABLE_DMPRRPC */

#if (1 == CONFIG_CLOUD_ENABLE_DMPSHADOW)
    /**
     * @brief Shadow Transparent data receive notification callback
     * @param[in] value Shadow Transparent data value
     * @param[in] valLen Shadow Transparent data length
     * @return None
     */
    typedef void (*Qth_ShadowRecvCb_f)(quint8_t *value, quint32_t valLen);

#endif /* CONFIG_CLOUD_ENABLE_DMPSHADOW */

#if (1 == CONFIG_CLOUD_ENABLE_DMPDESIRED)
    /**
     * @brief TTLV data receive notification callback
     * @param[in] ttlvHead TTLV request data
     * @return None
     */
    typedef void (*Qth_ttlvDesiredCb_f)(void *ttlvHead);
#endif /* CONFIG_CLOUD_ENABLE_DMPDESIRED */

    /** @} */

    /** @defgroup QTH_CONFIG_STRUCTS Configuration Structures
     *  @{ */

    /**
     * @brief Event callback structure
     */
    typedef struct
    {
        Qth_devEventCb_f devEventCb; /*!< Device event callback */
#if (1 == CONFIG_CLOUD_ENABLE_TSL)
        Qth_ttlvRecvCb_f ttlvRecvCb;             /*!< TTLV data receive callback */
        Qth_ttlvReqCb_f ttlvReqCb;               /*!< TTLV data request callback */
        Qth_ttlvReqCb2_f ttlvReqCb2;             /*!< Enhanced TTLV data request callback */
        Qth_ttlvRecvServerCb_f ttlvRecvServerCb; /*!< TTLV server data receive callback */
#if (1 == CONFIG_CLOUD_ENABLE_DMPDESIRED)
        Qth_ttlvDesiredCb_f desiredRecvCb;
#endif /* CONFIG_CLOUD_ENABLE_DMPDESIRED */
#if (1 == CONFIG_CLOUD_ENABLE_DMPRRPC)
        Qth_ttlvRecvRrpcCb_f ttlvRecvRrpcCb;
        Qth_ttlvReqRrpcCb_f ttlvReqRrpcCb; /*!< TTLV RRPC data request callback */
        Qth_ttlvRecvRrpcServerCb_f ttlvRecvRrpcServerCb;
#endif /* CONFIG_CLOUD_ENABLE_DMPRRPC */
#endif /* CONFIG_CLOUD_ENABLE_TSL */
#if (1 == CONFIG_CLOUD_ENABLE_DMPSHADOW)
        Qth_ShadowRecvCb_f shadowRecvCb; /*!< Shadow Transparent data receive callback */
#endif                                   /* CONFIG_CLOUD_ENABLE_DMPSHADOW */
#if (1 == CONFIG_CLOUD_ENABLE_TRANS)
        Qth_transRecvCb_f transRecvCb; /*!< Transparent data receive callback */
#endif                                 /* CONFIG_CLOUD_ENABLE_TRANS */
#if (1 == CONFIG_CLOUD_ENABLE_DMPOTA)
        Qth_otaEventCb_t otaEventCb; /*!< OTA event callback */
#endif                               /* CONFIG_CLOUD_ENABLE_DMPOTA */
    } Qth_eventCb_t;

    /**
     * @brief MCU information structure
     */
    typedef struct
    {
        char identity[QTH_COMPNO_MAXSIZE]; /*!< MCU component identity */
        char version[QTH_COMPVER_MAXSIZE]; /*!< MCU component version */
    } Qth_mcuInfo_t;

    /** @} */

    /** @defgroup QTH_CONFIG_API Configuration API Functions
     *  @{ */

    /**
     * @brief Initialize configuration, read config data from flash
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     * @note This API should be called first before any other config functions
     */
    oprt_ret Qth_configInit(void);

    /**
     * @brief Set default configuration
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetDefault(void);

    /**
     * @brief Clear all configuration
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configClear(void);

    /**
     * @brief Get device authentication type
     * @param[out] type Authentication type
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     * @note This config is set by calling Qth_configSetDeviceInfo() or Qth_configSetODOSInfo()
     */
    oprt_ret Qth_configGetAuthType(Qth_authType_e *type);

    /**
     * @brief Set product key and product secret information
     * @param[in] pk Product key
     * @param[in] ps Product secret
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetProductInfo(const char *pk, const char *ps);

    /**
     * @brief Get product key and product secret information
     * @param[out] pk Product key
     * @param[out] ps Product secret
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetProductInfo(char *pk, char *ps);

    /**
     * @brief Set device key and device secret information
     * @param[in] dk Device key
     * @param[in] ds Device secret
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     * @note Just used for OTOS type authentication
     */
    oprt_ret Qth_configSetDeviceInfo(const char *dk, const char *ds);

    /**
     * @brief Get device key and device secret information
     * @param[out] dk Device key
     * @param[out] ds Device secret
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetDeviceInfo(char *dk, char *ds);

    /**
     * @brief Set product key, product secret, device key and device secret information
     * @param[in] pk Product key
     * @param[in] ps Product secret
     * @param[in] dk Device key
     * @param[in] ds Device secret
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     * @note Just used for ODOS type authentication
     */
    oprt_ret Qth_configSetODOSInfo(const char *pk, const char *ps, const char *dk, const char *ds);

    /**
     * @brief Set server URL and IP information
     * @param[in] url Server URL
     * @param[in] ip Server IP
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetServer(const char *url, const char *ip);

    /**
     * @brief Get server URL and IP information
     * @param[out] url Server URL
     * @param[out] ip Server IP
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetServer(char *url, char *ip);

    /**
     * @brief Set MQTT lifetime information
     * @param[in] lifetime Lifetime for MQTT in seconds
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetLifetime(quint32_t lifetime);

    /**
     * @brief Get MQTT lifetime information
     * @param[out] lifetime Lifetime for MQTT in seconds
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetLifetime(quint32_t *lifetime);

    /**
     * @brief Set user event callback functions
     * @param[in] cb Callback structure containing all event callbacks
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetEventCb(Qth_eventCb_t cb);

    /**
     * @brief Get user event callback functions
     * @param[out] cb Callback structure to store all event callbacks
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetEventCb(Qth_eventCb_t *cb);

    /**
     * @brief Generate binding info
     *
     * This function generates a new binding info that can be used for device binding.
     *
     * @param[in] secret Whether to generate binding secret
     * @param[in] code Whether to generate binding code
     * @param[in] key Whether to generate binding key
     * @return Operation result
     * @retval OPRT_OK Generation successful
     * @retval Other values Generation failed, returns corresponding error
     * @note After successful generation, the binding info will be saved to flash memory
     */
    oprt_ret Qth_configGenBindingInfo(qbool secret, qbool code, qbool key);

    /**
     * @brief Set binding information
     * @param[in] secret Binding secret
     * @param[in] code Binding code
     * @param[in] key Binding key
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetBindingInfo(char *secret, char *code, quint8_t *key);

    /**
     * @brief Get binding information
     * @param[out] secret Binding secret
     * @param[out] code Binding code
     * @param[out] key Binding key
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetBindingInfo(char *secret, char *code, quint8_t *key);

    /**
     * @brief Set binding information source
     * @param[in] external Is it generated internally or set externally
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetBindingInfoSource(qbool external);

    /**
     * @brief Set binding flag
     * @param[in] bindingFlag Whether device is bound
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetBindingFlag(qbool bindingFlag);

    /**
     * @brief Get binding flag
     * @param[out] bindingFlag Whether device is bound
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetBindingFlag(qbool *bindingFlag);

    /**
     * @brief Set reset flag
     * @param[in] resetFlag Reset flag
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetResetFlag(qbool resetFlag);

    /**
     * @brief Get reset flag
     * @param[out] resetFlag Reset flag
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetResetFlag(qbool *resetFlag);

    /**
     * @brief Set module version
     * @param[in] version Module version
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetModuleVer(char *version);

    /**
     * @brief Get module version
     * @param[out] version Module version
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetModuleVer(char **version);

    /**
     * @brief Get module default version
     * @param[out] version Module default version
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetModuleDefaultVer(char **version);

    /**
     * @brief Set MCU version information
     * @param[in] mcuList MCU information list
     * @param[in] count Number of MCUs
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetMcuVer(Qth_mcuInfo_t *mcuList, quint16_t count);

    /**
     * @brief Get MCU version information
     * @param[out] mcuList MCU information list
     * @param[out] count Number of MCUs
     * @param[out] mcuVer Combined MCU version string
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetMcuVer(Qth_mcuInfo_t **mcuList, quint16_t *count, char **mcuVer);

    /**
     * @brief Set OTA information
     * @param[in] identity Component identity
     * @param[in] version Component version
     * @param[in] type Component type
     * @param[in] status OTA task status
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetOtaInfo(char *identity, char *version, quint8_t *type, quint8_t *status);

    /**
     * @brief Get OTA information
     * @param[out] identity Component identity
     * @param[out] version Component version
     * @param[out] type Component type
     * @param[out] status OTA task status
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetOtaInfo(char *identity, char *version, quint8_t *type, quint8_t *status);

    /**
     * @brief Set WiFi information
     * @param[in] ssid WiFi SSID
     * @param[in] pwd WiFi password
     * @param[in] mcc Mobile Country Code
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetWifiInfo(const char *ssid, const char *pwd, quint16_t mcc);

    /**
     * @brief Get WiFi information
     * @param[out] ssid WiFi SSID
     * @param[out] pwd WiFi password
     * @param[out] mcc Mobile Country Code
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetWifiInfo(char *ssid, char *pwd, quint16_t *mcc);

    /**
     * @brief Clear WiFi information
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configClearWifiInfo(void);

    /**
     * @brief Set WiFi network configuration mode information
     * @param[in] enable Whether to start network configuration if no WiFi info is saved
     * @param[in] timeout Network configuration timeout in seconds (180-600 or 0xffff for perpetual)
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     * @note When timeout is set to 0xffff (65535), device remains in configurable mode until network configuration is successful
     */
    oprt_ret Qth_configSetNetcfgMode(qbool enable, quint16_t timeout);

    /**
     * @brief Get WiFi network configuration mode information
     * @param[out] enable Whether to start network configuration if no WiFi info is saved
     * @param[out] timeout Network configuration timeout in seconds (180-600 or 0xffff for perpetual)
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetNetcfgMode(qbool *enable, quint16_t *timeout);

    /**
     * @brief Set PSK information
     * @param[in] id PSK ID
     * @param[in] secret PSK secret
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetPskInfo(const char *id, const char *secret);

    /**
     * @brief Get PSK information
     * @param[out] id PSK ID
     * @param[out] secret PSK secret
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetPskInfo(char *id, char *secret);

    /**
     * @brief Get module MAC address
     * @param[out] mac MAC address string
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetModuleMac(char *mac);

    /**
     * @brief Set BLE name
     * @param[in] name BLE name
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetBleName(const char *name);

    /**
     * @brief Get BLE name
     * @param[out] name BLE name
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetBleName(char *name);

    /**
     * @brief Print system configuration information
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configPrintCfgInfo(void);

    /**
     * @brief Product information temporary replacement
     * @param[out] productInfo Product information structure
     * @param[in] direct TRUE: copy current product info to productInfo; FALSE: copy productInfo to current product info
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configProductInfoReplace(void *productInfo, qbool direct);

    /**
     * @brief Get hex band product index
     * @return Hex band product index
     * @note This API is valid only in the MCU firmware
     */
    quint8_t Qth_configHexBuandProdIndexGet(void);

    /**
     * @brief Set hex band product index
     * @param[in] index Hex band product index
     * @return None
     * @note This API is valid only in the MCU firmware
     */
    void Qth_configHexBuandProdIndexSet(quint8_t index);

    /**
     * @brief Add new BLE bind code
     * @param[in] bindcode Bind code string
     * @return >=0 is bind code sort ID, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configAddBleBindcode(char *bindcode);

    /**
     * @brief Get BLE bind code by sort ID
     * @param[in] sortId Bind code sort ID
     * @return Bind code string for the specified sort ID
     */
    char *Qth_configGetBleBindcode(int sortId);

    /**
     * @brief Delete BLE bind code by sort ID
     * @param[in] sortId Bind code sort ID
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configDelBleBindcode(quint8_t sortId);

    /**
     * @brief Check if bind code buffer is empty
     * @return TRUE if empty, FALSE if not
     */
    qbool Qth_configBindCodeIsEmpty(void);

    /**
     * @brief Check if bind code buffer is full
     * @return TRUE if full, FALSE if not
     */
    qbool Qth_configBindCodeIsFull(void);

    /**
     * @brief Clear all bind codes
     * @return None
     */
    void Qth_configClearAllBindCode(void);

    /**
     * @brief Get old binding secret
     * @param[in] oldSecret Buffer to store encrypted old binding secret
     * @return Old binding secret length
     */
    quint8_t Qth_configGetOldBindingSecret(char *oldSecret);

    /**
     * @brief Set old binding secret
     * @param[in] oldSecret Old binding secret
     * @param[in] secretLen Old binding secret length
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetOldBindingSecret(char *oldSecret, quint32_t secretLen);

    /**
     * @brief Set custom BLE advertising data
     * @param[in] custAdvData Custom advertising data
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     * @note Maximum length is QTH_BLE_MAX_CUST_ADV_DATA_LEN
     */
    oprt_ret Qth_configSetCustBleAdvData(quint8_t *custAdvData);

    /**
     * @brief Get custom BLE advertising data
     * @param[in] custAdvData Buffer to store custom advertising data
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     * @note Maximum length is QTH_BLE_MAX_CUST_ADV_DATA_LEN
     */
    oprt_ret Qth_configGetCustBleAdvData(quint8_t *custAdvData);

    /**
     * @brief Set PDP context
     * @param[in] type PDP context type
     * @param[in] id PDP context ID
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetCurrPdp(Qth_pdpType_e type, quint8_t id);

    /**
     * @brief Get PDP context type
     * @return PDP context ID type
     */
    Qth_pdpType_e Qth_configGetCurrPdpType();

    /**
     * @brief Get PDP context ID
     * @return PDP context ID
     */
    quint8_t Qth_configGetCurrPdpId();

    /**
     * @brief Set provision URL
     * @param[in] url provision URL
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configSetProvisionUrl(const char *url, const char *apiKey);

        /**
     * @brief Get provision URL
     * @param[out] url Buffer to store provision URL
     * @return OPRT_OK on success, others on error. Please refer to qth_errorCode.h for error codes.
     */
    oprt_ret Qth_configGetProvisionUrl(char *url, char *apiKey);

    /** @} */

#ifdef __cplusplus
}
#endif

#endif /* __QTH_CONFIG_H__ */