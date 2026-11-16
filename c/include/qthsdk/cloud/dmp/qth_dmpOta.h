/**
 * @file qth_dmpOta.h
 * @author niko.huang
 * @version 0.0.1
 * @date 2023.03.20
 * @brief OTA (Over-the-Air) upgrade module interface definitions
 * @copyright Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 */

#ifndef __QTH_DMPOTA_H__
#define __QTH_DMPOTA_H__

#include "qhal_types.h" /**< Qth link layer definitions */

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup QTH_OTA_API OTA (Over-the-Air) Upgrade API
     * @{ */

    /**
     * @brief OTA upgrade type enumeration
     */
    typedef enum
    {
        QTH_OTA_TYPE_FOTA = 0, /**< Firmware over-the-air */
        QTH_OTA_TYPE_SOTA = 1, /**< Software over-the-air */
    } Qth_otaType_e;

    /**
     * @brief OTA upgrade status enumeration
     */
    typedef enum
    {
        QTH_OTA_STATUS_NOPLAN = 0,          /**< No upgrade plan */
        QTH_OTA_STATUS_REVICEPLAN = 1,      /**< Upgrade plan received */
        QTH_OTA_STATUS_SUBMITOTA = 2,       /**< Upgrade confirmed */
        QTH_OTA_STATUS_REFUSEDOTA = 3,      /**< Upgrade refused */
        QTH_OTA_STATUS_DOWNLOADWAIT = 4,    /**< Waiting for download */
        QTH_OTA_STATUS_DOWNLOADING = 5,     /**< Downloading in progress */
        QTH_OTA_STATUS_DOWNLOADERROR = 6,   /**< Download failed */
        QTH_OTA_STATUS_TOKENERROR = 7,      /**< Download token expired */
        QTH_OTA_STATUS_DOWNLOADSUCCESS = 8, /**< Download successful */
        QTH_OTA_STATUS_UPDATING = 9,        /**< Updating in progress */
        QTH_OTA_STATUS_WAITINGRSP = 10,     /**< Waiting for module response */
        QTH_OTA_STATUS_UPDATESUCCESS = 11,  /**< Update successful */
        QTH_OTA_STATUS_UPDATEERROR = 12,    /**< Update failed */
    } Qth_otaStatus_e;

    /**
     * @brief OTA plan information structure
     */
    typedef struct
    {
        char *identity;      /**< Component identifier */
        qint64_t type;       /**< OTA type (Qth_otaType_e) */
        char *srcVer;        /**< Source version */
        char *dstVer;        /**< Destination version */
        qint64_t minBattery; /**< Minimum battery level required */
        qint64_t minFlash;   /**< Minimum flash space required */
        qint64_t minSignal;  /**< Minimum signal strength value required */
    } Qth_otaPlan_t;

    /**
     * @brief OTA firmware information structure
     */
    typedef struct
    {
        char *version;     /**< Firmware version */
        char *fileMd5;     /**< Firmware MD5 checksum */
        char *fileCrc;     /**< Firmware CRC checksum */
        char *fileSha256;  /**< Firmware SHA256 checksum */
        char *fileUrl;     /**< Firmware download link */
        qint64_t fileSize; /**< Firmware size in bytes */
        char *srcVersion;  /* Firmware source version */
    } Qth_otaInfo_t;

    /**
     * @brief SOTA download type enumeration
     */
    typedef enum
    {
        QTH_SOTA_DOWN_TYPE_MODULE = 0, /**< Downloaded by the module */
        QTH_SOTA_DOWN_TYPE_MCU = 1,    /**< Downloaded by the MCU */
    } Qth_otaDownType_e;

    /**
     * @brief Device type enumeration for OTA
     */
    typedef enum
    {
        QTH_DEV_TYPE_GATEWAY = 0, /**< Gateway device */
        QTH_DEV_TYPE_SUB = 1,     /**< Sub-device */
    } Qth_otaDevType_e;

    /**
     * @brief OTA plan callback function type
     * @param[in] planList Array of OTA plan structures
     * @param[in] count Number of OTA plans in the list
     * @return TRUE to permit upgrade, FALSE to refuse upgrade
     * @note The list may contain one or more OTA plans
     */
    typedef qbool (*Qth_otaPlanCb_f)(Qth_otaPlan_t *planList, quint16_t count);

    /**
     * @brief Extended OTA plan callback function type (for asynchronous result reporting)
     * @param[in] planList Array of OTA plan structures
     * @param[in] count Number of OTA plans in the list
     * @return None
     * @note Use 'Qth_otaPlanConfirm' to accept or refuse the upgrade plan
     */
    typedef void (*Qth_otaPlanExCb_f)(Qth_otaPlan_t *planList, quint16_t count);

    /**
     * @brief OTA file download progress callback function type
     * @param[in] identity Component identifier
     * @param[in] percent Percentage of HTTP download completion (0-100)
     * @return None
     * @note The download progress is called back every 10% by default
     */
    typedef void (*Qth_otaDownloadCb_f)(char *identity, quint8_t percent);

    /**
     * @brief OTA upgrade result callback function type
     * @param[in] identity Component identifier
     * @param[in] result Result (0 for success, non-zero for error code)
     * @return None
     */
    typedef void (*Qth_otaResultCb_f)(char *identity, qbool result);

    /**
     * @brief SOTA firmware information callback function type
     * @param[in] identity Component identifier
     * @param[in] info Firmware information structure
     * @param[out] type Firmware download type (default: module download)
     * @return TRUE to permit upgrade, FALSE to refuse upgrade
     */
    typedef qbool (*Qth_sotaInfoCb_f)(char *identity, Qth_otaInfo_t *info, Qth_otaDownType_e *type);

    /**
     * @brief SOTA firmware fragment callback function type
     * @param[in] identity Component identifier
     * @param[in] offset Fragment offset position
     * @param[in] data Fragment content buffer
     * @param[in] dataLen Fragment size in bytes
     * @return TRUE if MCU flash write successful, FALSE if failed
     * @note This callback is only triggered if Qth_sotaInfoCb_f returns TRUE
     */
    typedef qbool (*Qth_sotaFileCb_f)(char *identity, quint32_t offset, quint8_t *data, quint32_t dataLen);

    /**
     * @brief OTA event callback structure
     */
    typedef struct
    {
        Qth_otaPlanCb_f planCb;         /**< Required: Shared by FOTA and SOTA (choose either planCb or planExCb) */
        Qth_otaPlanExCb_f planExCb;     /**< Required: Shared by FOTA and SOTA (choose either planCb or planExCb) */
        Qth_otaDownloadCb_f downloadCb; /**< Optional: Shared by FOTA and SOTA */
        Qth_otaResultCb_f resultCb;     /**< Required: Shared by FOTA and SOTA */
        Qth_sotaInfoCb_f sotaInfoCb;    /**< Optional: For SOTA only */
        Qth_sotaFileCb_f sotaFileCb;    /**< Optional: For SOTA only */
    } Qth_otaEventCb_t;

    /**
     * @brief Request OTA upgrade plan from Developer Center
     * @return OPRT_OK on success, error code otherwise
     * @note This function initiates the process of checking for available OTA updates
     */
    oprt_ret Qth_otaRequest(void);

    /**
     * @brief Update OTA process version information
     * @param[in] devType Device type (gateway or sub-device)
     * @param[in] identity Component identifier
     * @param[in] version Component version string
     * @return OPRT_OK on success, error code otherwise
     */
    oprt_ret Qth_otaVersionCheck(Qth_otaDevType_e devType, const char *identity, const char *version);

    /**
     * @brief Report OTA status to the server
     * @param[in] status Current OTA status
     * @return None
     */
    void Qth_otaStatusReport(Qth_otaStatus_e status);

    /**
     * @brief Accept or refuse the OTA upgrade plan
     * @param[in] action OTA upgrade operation
     *                   0: Refuse the upgrade
     *                   1: Confirm the upgrade
     * @return OPRT_OK on success, error code otherwise
     */
    oprt_ret Qth_otaPlanConfirm(quint8_t action);

    /**
     * @brief Initialize OTA module
     * @return OPRT_OK on success, error code otherwise
     * @note This function must be called before using any other OTA functions
     */
    oprt_ret Qth_dmpOtaInit(void);

    /**
     * @brief Set the MCU update timeout period
     * @param[in] timeout Timeout value in milliseconds
     * @return TRUE on success, FALSE on error
     */
    qbool Qth_otaSetMcuUpdateTimeWait(quint32_t timeout);

    /** @} */ // end of QTH_OTA_API group

#ifdef __cplusplus
}
#endif

#endif /* __QTH_DMPOTA_H__ */