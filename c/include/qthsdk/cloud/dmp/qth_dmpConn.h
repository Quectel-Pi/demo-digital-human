/**
 * @file qth_dmpConn.h
 * @author niko.huang
 * @version 0.0.1
 * @date 2023.03.07
 * @brief The implements of DMP (Device Management Platform) data transmission.
 * @copyright Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 */

#ifndef __QTH_DMPCONN_H__
#define __QTH_DMPCONN_H__

#include "qth_cmdId.h"
#include "quos_socket.h"
#include "qhal_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup QTH_DMPCONN_API DMP Connection API
     * @brief Functions for managing device connection to the Device Management Platform.
     * @{
     */

    /**
     * @brief DMP device action types.
     */
    typedef enum
    {
        QTH_DEV_ACTION_AUTH = 0,  /**< Device authentication action */
        QTH_DEV_ACTION_CONN = 1,  /**< Device connection action */
        QTH_DEV_ACTION_RESET = 2, /**< Device reset action */
    } Qth_dmpAction_e;

    /**
     * @brief Initialize DMP connection module.
     * @return OPRT_OK on success. Others on error, please refer to qth_errorCode.h.
     * @note This function should be called before using other DMP connection functions.
     */
    oprt_ret Qth_dmpConnInit(void);

    /**
     * @brief Connect device to DMP platform.
     * @return OPRT_OK on success. Others on error, please refer to qth_errorCode.h.
     * @note After network configuration success, call this function to connect to DMP platform.
     */
    oprt_ret Qth_dmpConnect(void);

    /**
     * @brief Disconnect device from DMP platform.
     * @return OPRT_OK on success. Others on error, please refer to qth_errorCode.h.
     */
    oprt_ret Qth_dmpDisconnect(void);

    /**
     * @brief Get DMP connection status.
     * @return true if connected to DMP, false if disconnected.
     */
    qbool Qth_dmpConnGetStatus();

    /**
     * @brief Get the file descriptor of the DMP connection channel.
     * @return File descriptor for DMP connection channel, or -1 on error.
     */
    void *Qth_dmpConnGetChlFd();

    /**
     * @brief Handle DMP connection errors.
     * @param [in] errCode: Error code indicating the type of error.
     * @return void
     */
    void Qth_dmpConnErrHandle(oprt_ret errCode);

    /**
     * @brief Convert DMP error codes to local error codes.
     * @param [in] dmpCode: DMP platform error code.
     * @return Corresponding local error code.
     */
    oprt_ret Qth_dmpConnCodeConversion(quint16_t dmpCode);

#ifdef __cplusplus
}
#endif

#endif /* __QTH_DMPCONN_H__ */