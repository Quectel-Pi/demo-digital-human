/**
 * @file qth_core.h
 * @author niko.huang
 * @version 0.0.1
 * @date 2023.03.08
 * @brief The implements of DMP (Device Management Platform)
 * @copyright Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 */

#ifndef __QTH_CORE_H__
#define __QTH_CORE_H__

#include "qhal_types.h"
#include "qth_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        QTH_CORE_STATUS_IDLE = 0,  /*!< core is idle */
        QTH_CORE_STATUS_INIT = 1,  /*!< core is initialized */
        QTH_CORE_STATUS_START = 2, /*!< core is started */
    } Qth_coreStatus_e;

    /** @defgroup QTH_CORE_API Core API Functions
     *  @{ */

    /**
     * @brief Initialize QTH core system
     *
     * This function is used to initialize various components and resources of the QTH (Quectel IoT Hub) core system.
     * Before using other functions of the QTH system, this function must be called first.
     *
     * @return Operation result
     * @retval OPRT_OK Initialization successful
     * @retval Other values Initialization failed, returns corresponding error code
     */
    oprt_ret Qth_coreInit(void);

    /**
     * @brief Start QTH core system
     *
     * This function is used to start the operation of the QTH core system, including starting various services and threads.
     * It must be called after successfully initializing with Qth_coreInit().
     *
     * @return Operation result
     * @retval OPRT_OK Startup successful
     * @retval Other values Startup failed, returns corresponding error code
     */
    oprt_ret Qth_coreStart(void);

    /**
     * @brief Stop QTH core system
     *
     * This function is used to stop the operation of the QTH core system, including stopping various services and threads.
     * After the system is stopped, it can be restarted by calling Qth_coreStart().
     *
     * @return Operation result
     * @retval OPRT_OK Stop successful
     * @retval Other values Stop failed, returns corresponding error code
     */
    oprt_ret Qth_coreStop(void);

    /**
     * @brief Get the current status of the QTH core system
     *
     * This function is used to retrieve the current operational status of the QTH (Quectel IoT Hub) core system.
     * - QTH_CORE_STATUS_IDLE: Idle state (initial state)
     * - QTH_CORE_STATUS_INIT: Initialization state
     * - QTH_CORE_STATUS_START: Started state
     *
     * @return Current QTH core system status
     * @retval Qth_coreStatue_e enumeration type representing the current system status
     */
    Qth_coreStatus_e Qth_coreGetStatus(void);

    /**
     * @brief Reset QTH core system
     *
     * This function is used to reset the state of the QTH core system, clear device keys and binding relationships, etc.
     * After calling this interface, the device will go offline and needs to call Qth_dmpConnect again to go back online.
     *
     * @return Operation result
     * @retval OPRT_OK Reset successful
     * @retval Other values Reset failed, returns corresponding error code
     */
    oprt_ret Qth_coreReset(void);

    /**
     * @brief Post QTH device event
     *
     * This function is used to post device events to the QTH system, notifying the system of changes in device status.
     * Event types are defined by the Qth_devEvent_e enumeration.
     *
     * @param[in] event Event type, see Qth_devEvent_e enumeration definition
     * @param[in] code Event result code, usually OPRT_OK for success, corresponding error code for failure
     *
     * @note The Qth_devEvent_e enumeration defines the following event types:
     *       - QTH_DEV_EVENT_WIFI_CONFIG (0): Device configures router event
     *       - QTH_DEV_EVENT_WIFI_CONNECT (1): Device connects to router event
     *       - QTH_DEV_EVENT_DMP_CONNECT (2): Device connects to DMP event
     *       - QTH_DEV_EVENT_RESET (3): Device reset event
     *       - QTH_DEV_EVENT_REMOVE (4): Device remove event
     *       - QTH_DEV_EVENT_SLIENT (5): Device enters silent mode event
     *       - QTH_DEV_EVENT_BLE (6): Device BLE connection event
     *       - QTH_DEV_EVENT_ETH_CONNECT (7): Device Ethernet connection event
     *       - QTH_DEV_EVENT_LTE_CONNECT (9): Device LTE connection event
     */
    void Qth_coreEventPost(Qth_devEvent_e event, oprt_ret code);

    /** @} */

#ifdef __cplusplus
}
#endif

#endif /* __QTH_CORE_H__ */