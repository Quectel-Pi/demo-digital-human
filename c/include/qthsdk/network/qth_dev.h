/*************************************************************************
** @version : 0.0.1
** @date : 2026.03.31
** @brief : Device management module
** @copyright : Copyright 2022-2023 Quectel Inc. All Rights Reserved.
*************************************************************************/

#ifndef __QTH_DEV_H__
#define __QTH_DEV_H__

#include "qhal_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
    ** @brief : init all components
    ** @param [in] : none
    ** @param [out] : none
    ** @return : OPRT_OK on success. Others on error, please refer to qth_errorCode.h
    ** @note: this API should call first before any other DMP functions
    **/
    oprt_ret Qth_devInit(void);

    /**
    ** @brief : start process
    ** @param [in] : none
    ** @param [out] : none
    ** @return : OPRT_OK on success. Others on error, please refer to qth_errorCode.h
    ** @note: if exist ssid/pwd info, will connect router, otherwise, start ble adv and enter netcfg status
    **/
    oprt_ret Qth_devStart(void);

    /**
    ** @brief : stop process
    ** @param [in] : none
    ** @param [out] : none
    ** @return : OPRT_OK on success. Others on error, please refer to qth_errorCode.h
    ** @note: none
    **/
    oprt_ret Qth_devStop(void);

    /**
    ** @brief: dev reset
    ** @param [in]: none
    ** @return: OPRT_OK on success. Others on error, please refer to qth_errorCode.h
    ** @note: clean wifi info, enter netcfg status again, after netcfg success, update new ds from DMP
    **/
    oprt_ret Qth_devReset(void);

#ifdef __cplusplus
}
#endif

#endif /* __QTH_DEV_H__ */
