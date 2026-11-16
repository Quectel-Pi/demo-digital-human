/*************************************************************************
** @version : 0.0.1
** @date : 2023.03.02
** @brief : the implements of OS dependent definitions
** @copyright : Copyright 2022-2023 Quectel Inc. All Rights Reserved.
*************************************************************************/
#ifndef __QHAL_OS_DEFINE_H__
#define __QHAL_OS_DEFINE_H__
#include "qhal_types.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*******************************************************************************
                                    qhal_socket
     *******************************************************************************/
    typedef int Qhal_socket_t;
    #define QHAL_SOCKET_INVALID -1
    #define QHAL_FD_MAX_COUNT (1024)
    #define QHAL_FDS_BITS_TYPE unsigned long
    #define QHAL_PDPCID 1 /* TODO:临时使用固定PDPCID，后续改为用户配置 */
    typedef struct
    {
        QHAL_FDS_BITS_TYPE fds_bits[QHAL_FD_MAX_COUNT / (8 * sizeof(QHAL_FDS_BITS_TYPE))];
    } Qhal_fdSet_t;

    /*******************************************************************************
                                    qhal_file
     *******************************************************************************/
    typedef FILE *Qhal_file_t;
    #define QHAL_FILE_INVALID NULL
    
    /*******************************************************************************
                                    qhal_mutex
     *******************************************************************************/
    typedef void *Qhal_mutex_t;

    /*******************************************************************************
                                    qhal_semaphore
     *******************************************************************************/
    typedef void *Qhal_sem_t;

    /*******************************************************************************
                                    qhal_thread
     *******************************************************************************/
    typedef void *Qhal_thread_t;
    typedef void *Qhal_threadRet_t;
    typedef Qhal_threadRet_t (*Qhal_threadFunc_f)(void *);
    #define QHAL_THREAD_END return NULL
    
    /*******************************************************************************
                                    qhal_log
    *******************************************************************************/
#define Qhal_log_output(format, ...) printf(format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /*__QHAL_OS_DEFINE_H__ */
