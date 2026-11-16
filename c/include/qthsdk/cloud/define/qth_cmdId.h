/**
 * @file qth_cmdId.h
 * @author Quectel
 * @version 0.0.1
 * @date 2023.03.02
 * @brief Command ID definitions for device protocol commands
 * @copyright Copyright (c) 2024 Quectel Wireless Solutions Co., Ltd. All rights reserved.
 */
#ifndef __QTH_CMDID_H__
#define __QTH_CMDID_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Device protocol command ID enumeration
 */
typedef enum
{
    QTH_DPCMD_TSL_REQ = 0X0011,              /*!< Request to get device TSL data */
    QTH_DPCMD_TSL_RSP = 0X0012,              /*!< Reply to the request for device TSL data */
    QTH_DPCMD_TSL_WRITE = 0X0013,            /*!< Request to set device TSL data */
    QTH_DPCMD_TSL_EVENT = 0X0014,            /*!< Report of device TSL data async */
    QTH_DPCMD_TSL_SERVER_WRITE = 0X0015,     /*!< Request to set device TSL service data */
    QTH_DPCMD_TSL_SERVER_EVENT = 0X0016,     /*!< Report of device TSL service data */
    QTH_DPCMD_TSL_WRITE_SYNC = 0x0017,       /*!< Report of device TSL data sync */
    QTH_DPCMD_TSL_WRITE_SYNC_RSP = 0x0018,   /*!< Reply to the report of device TSL data sync */
    QTH_DPCMD_TSL_SYNC_RSP = 0x001A,         /*!< Reply to the request to synchronize device TSL data */
    QTH_DPCMD_TSL_EVENT_REPORT = 0x001B,     /*!< Report of device TSL event report */
    QTH_DPCMD_PASS_WRITE = 0X0023,           /*!< Request to send transparent data to device */
    QTH_DPCMD_PASS_EVENT = 0X0024,           /*!< Report of transparent data received from device */
    QTH_DPCMD_STATUS_REQ = 0X0031,           /*!< Request to get device status */
    QTH_DPCMD_STATUS_RSP = 0X0032,           /*!< Reply to the request for device status */
    QTH_DPCMD_STATUS_EVENT = 0X0034,         /*!< Report of device status */
    QTH_DPCMD_STATUS_INFO_EVENT_INSIDE = 0X0035, /* The device proactively reports its status and module information. */
    QTH_DPCMD_STATUS_INFO_EVENT = 0X0036,        /* The device proactively reports its status and module information. */
    QTH_DPCMD_INFO_REQ = 0X0041,             /*!< Request to get device information */
    QTH_DPCMD_INFO_RSP = 0X0042,             /*!< Reply to the request for device information */
    QTH_DPCMD_INFO_EVENT = 0X0044,           /*!< Report of device information */
    QTH_DPCMD_EXCE_WRITE = 0X00A3,           /*!< Exception notification */
    QTH_DPCMD_EXCE_EVENT = 0X00A4,           /*!< Report of exception notification */
    QTH_DPCMD_DEV_MANAGER_WRITE = 0X00B3,    /*!< Device management command */
    QTH_DPCMD_DEV_CONFIG_WRITE = 0X00B4,     /*!< Device configuration command */
    QTH_DPCMD_DEV_BINDCODE_WRITE = 0X00B5,   /*!< Report device binding information */
    QTH_DPCMD_DEV_BINDCODE_RSP = 0X00B6,     /*!< Reply to the device binding information */
    QTH_DPCMD_DEV_CONFIG_RSP = 0X00B7,       /*!< Reply to the configuration management command */
    QTH_DPCMD_NTP_REQ = 0x00B8,              /*!< Request to synchronize server time and time zone */
    QTH_DPCMD_NTP_RSP = 0x00B9,              /*!< Reply to the request to synchronize server time and time zone */
    QTH_DPCMD_OTA_NOTIFY = 0X0111,           /*!< OTA upgrade task notification */
    QTH_DPCMD_OTA_COMFIRM = 0X0112,          /*!< OTA confirmation - whether to upgrade after receiving OTA upgrade task notification */
    QTH_DPCMD_OTA_FW_INFO = 0X0113,          /*!< Firmware information delivery */
    QTH_DPCMD_OTA_EVENT = 0X0114,            /*!< Report of OTA process status */
    QTH_DPCMD_OTA_REQUEST = 0X115,           /*!< OTA request */
    QTH_DPCMD_LOC_CFG_WRITE = 0X0121,        /*!< Setting location information to be reported, no longer supported */
    QTH_DPCMD_LOC_REPORT = 0X0122,           /*!< Report of location information */
    QTH_DPCMD_LOC_REQ = 0X0123,              /*!< Request to get real-time location information */
    QTH_DPCMD_LOC_RSP = 0X0124,              /*!< Reply to the request for real-time location information */
    QTH_DPCMD_SUB_AUTH = 0x00C0,             /*!< Sub-device authentication command */
    QTH_DPCMD_SUB_AUTH_RSP = 0x00C1,         /*!< Sub-device authentication response */
    QTH_DPCMD_SUB_LOGIN = 0x00C2,            /*!< Sub-device login command */
    QTH_DPCMD_SUB_LOGIN_RSP = 0x00C3,        /*!< Sub-device login response */
    QTH_DPCMD_SUB_LOGOUT = 0x00C4,           /*!< Sub-device logout command */
    QTH_DPCMD_SUB_LOGOUT_RSP = 0x00C5,       /*!< Sub-device logout response */
    QTH_DPCMD_SUB_UNAUTH_EVENT = 0x00C6,     /*!< Sub-device unauthorization event */
    QTH_DPCMD_SUB_UNAUTH_EVENT_RSP = 0x00C7, /*!< Sub-device unauthorization event response */
    QTH_DPCMD_SUB_OFFLINE_EVENT = 0x00C8,    /*!< Sub-device offline event */
    QTH_DPCMD_SUB_RESET = 0x00C9,            /*!< Sub-device reset command */
    QTH_DPCMD_SUB_RESET_ACK = 0x00CA,        /*!< Cloud reply to sub-device reset command, DS will be returned on success */
    QTH_DPCMD_SUB_RESET_ACK_ACK = 0x00CB,    /*!< Device notifies DMP that it has received information issued by the platform */
    QTH_DPCMD_LAN_DISCOVER_REQ = 0X7030,     /*!< UDP discovery of devices on the LAN */
    QTH_DPCMD_LAN_DISCOVER_RSP = 0X7031,     /*!< UDP response to LAN discovery request */
    QTH_DPCMD_LAN_RANDOM_REQ = 0x7032,
    QTH_DPCMD_LAN_RANDOM_RSP = 0x7033,
    QTH_DPCMD_BLE_APP_REG = 0x7017,
    QTH_DPCMD_BLE_APP_REG_RSP= 0x7018,
    QTH_DPCMD_LAN_APP_AUTH_REQ = 0x7034,
    QTH_DPCMD_LAN_APP_AUTH_RSP= 0x7035,
    QTH_DPCMD_LAN_TSL_WRITE_RSP = 0x7036,    /*!< LAN response to physical model distribution */
    QTH_DPCMD_LAN_HB_REQ = 0x7037,           /*!< BLE heartbeat request */
    QTH_DPCMD_LAN_HB_RSP = 0x7038,           /*!< BLE heartbeat response */
    QTH_DPCMD_LAN_HB_CONTROL = 0x7039,       /*!< BLE heartbeat control request */
    QTH_DPCMD_LAN_HB_CONTROL_RSP = 0x703A,   /*!< BLE heartbeat control response */

    QTH_DPCMD_BLE_CFG_QUE = 0x7010,          /*!< Configure router */
    QTH_DPCMD_BLE_CFG_ANS = 0x7011,          /*!< Configure router response */
    QTH_DPCMD_BLE_SSID_QUE = 0x7012,         /*!< Get router scan list */
    QTH_DPCMD_BLE_SSID_ANS = 0x7013,         /*!< Get router scan list response */
    QTH_DPCMD_BLE_CON_REQ = 0x7014,          /*!< Connection status report */

    QTH_DPCMD_BLE_OTA_DEVINFO_REQ = 0x7040,
    QTH_DPCMD_BLE_OTA_DEVINFO_RSP = 0x7041,
    QTH_DPCMD_BLE_OTA_OTAINFO_NOTIFY = 0x7042,
    QTH_DPCMD_BLE_OTA_OTADATA_CONTROL = 0x7043,
    QTH_DPCMD_BLE_OTA_OTADATA_CONTROL_RSP = 0x7044,
    QTH_DPCMD_BLE_OTA_OTADATA_WIRTE = 0x7045,
    QTH_DPCMD_BLE_OTA_OTADATA_WIRTE_RSP = 0x7046,

    QTH_DPCMD_SCAN_WIFI_LIST_REQ = 0x7051,   /*!< Scan wifi list request */
    QTH_DPCMD_SCAN_WIFI_LIST_RSP = 0x7052,   /*!< Scan wifi list response */
    QTH_DPCMD_SCAN_WIFI_LIST_REP = 0x7053,   /*!< Report scan wifi list */
    QTH_DPCMD_SCAN_WIFI_LIST_ANS = 0x7054,   /*!< Report scan wifi list answer */
    QTH_DPCMD_SWITCH_WIFI_REQ = 0x7055,      /*!< Switch wifi request */
    QTH_DPCMD_SWITCH_WIFI_RSP = 0x7056,      /*!< Switch wifi response */

    QTH_DPCMD_BLE_TIME_ALIGNED_REQ = 0x7060,
    QTH_DPCMD_BLE_TIME_ALIGNED_RSP = 0x7061,
    QTH_DPCMD_BLE_TIME_ALIGNED_NOTIFY = 0x7062,
    QTH_DPCMD_BLE_UNBINDING_REQ = 0x7063,      /*!< BLE unbinding data report */
    QTH_DPCMD_BLE_UNBINDING_REQ_RSP = 0x7064,  /*!< BLE unbinding data response */
    QTH_DPCMD_BLE_HISTORY_DATA_REQ = 0x7065,   /*!< BLE history data report */
    QTH_DPCMD_BLE_HISTORY_DATA_RSP = 0x7066,   /*!< BLE history data response */
} Qth_cmdId_e;

#ifdef __cplusplus
}
#endif
#endif /*__QTH_CMDID_H__ */
