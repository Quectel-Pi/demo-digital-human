#ifndef __QUOS_SOCKET_H__
#define __QUOS_SOCKET_H__
#include "qhal_types.h"
#include "qhal_sdk_config.h"
#include "quos_errorCode.h"
#include "quos_twll.h"
#include "quos_time.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum
{
    QUOS_SOCKET_TYPE_UDP = 0,
    QUOS_SOCKET_TYPE_TCP_CLIENT,
    QUOS_SOCKET_TYPE_TCP_SERVER,
    QUOS_SOCKET_TYPE_UDP_SSL_CLIENT,
    QUOS_SOCKET_TYPE_TCP_SSL_CLIENT,
    QUOS_SOCKET_TYPE_BLE,
    QUOS_SOCKET_TYPE_UART,
};

typedef enum
{
    QUOS_SOCKET_NODE_TYPE_ORDERED = (1 << 0),
    QUOS_SOCKET_NODE_TYPE_PRIORITY = (1 << 1),
    QUOS_SOCKET_NODE_TYPE_UNORDERED = (1 << 2),
} Quos_socketSendNodeType_e;

typedef qbool (*Quos_socketSendNodeCb_f)(void *chlFd, const void *sendData, qbool result, quint8_t sendCnt);
typedef void (*Quos_socketRecvNodeCb_f)(void *chlFd, const void *sendData, const void *recvData);

typedef struct
{
    quint16_t bufLen;
    quint8_t *Buf;
} Quos_socketRecvDataNode_t;

typedef struct
{
    Quos_twll_t head;
    quint8_t sendCnt;
    quint16_t bufLen;
    quint32_t sendTimeout;
    quint32_t pkgId;
    quint8_t *Buf;
    void *param;
    void *peer;
    qbool priority;
    Quos_socketSendNodeCb_f sendCB;
    Quos_socketRecvNodeCb_f recvCB; /* Wait for the receive to time out or succeed */
} Quos_socketSendDataNode_t;

typedef struct
{
    quint8_t temp;
    quint8_t *buf;
    quint32_t offset;
    quint32_t bufLen;
} Quos_socketTempData_t;

typedef qbool (*Quos_socketUnformCb_f)(const quint8_t *buf, quint32_t bufLen, quint32_t *offset, Quos_socketTempData_t *unformTemp);
typedef qbool (*Quos_socketRecvNotifyCb_f)(void *chlFd, const void *peer, quint32_t peerSize, Quos_socketRecvDataNode_t *recvData);
typedef qbool (*Quos_socketConnNotifyCb_f)(void *chlFd, qbool result);
typedef oprt_ret (*Quos_socketSendIOCb_f)(pointer_t sockFd, const void *peer, const void *buf, quint16_t bufLen, qbool *isSending, const void *param);
typedef void (*Quos_socketCloseIOCb_f)(pointer_t sockFd);
typedef void (*Quos_socketParamFreeCb_f)(void *param);

typedef struct
{
    Quos_twll_t head;
    pointer_t sockFd;
    quint8_t type;
    qbool valid;
    void *param;
    void **self;
    Quos_socketRecvNotifyCb_f recvDataFunc;
    Quos_socketUnformCb_f unformFunc; /* Calls to the Quos_socket class API are not allowed within this callback API, otherwise it will cause a deadlock */
    Quos_socketTempData_t unformTemp;
    Quos_socketParamFreeCb_f paramFree;
    struct
    {
        Quos_socketSendIOCb_f sendx;
        Quos_socketCloseIOCb_f closex;
    } io; /* Call the Quos_socket API is not allowed in this IO class CB, otherwise it will cause a deadlock */
    struct
    {
        Quos_socketConnNotifyCb_f notify;
        quint32_t timeout;
        void *timer;
    } conn;
    struct
    {
        quint8_t txCnt;
        quint8_t waitIoSendAck;
        qbool waitPkgAck;
        qbool current_limiting;
        quint32_t timeout;
        quint32_t sendInterVal; /* The minimum interval for sending data is used to limit the rate */
        Quos_systick_t beginTime;
        Quos_systick_t recvTime;
        Quos_twll_t *orderList;    /* Ordered send list */
        Quos_twll_t *disorderList; /* Unordered send list, must not wait for reply data; Data has precedence over the ordered send list, even if the ordered send list is still being sent */
    } send;
} Quos_socketChlInfoNode_t;

/**
** @brief : Initialize the resource
** @return : OPRT_OK on success. Others on error, please refer to quos_errorCode.h
**/
oprt_ret Quos_socketInit(void);
/**
** @brief : add socket channel
** @param [out] chlFdPoint: channel pointer
** @param [in] chlInfo: channel info
** @return : channel handle
** @note : none
**/
void *Quos_socketChannelAdd(void **chlFdPoint, Quos_socketChlInfoNode_t *chlInfo);
/**
** @brief : del socket channel
** @param [in] chlFd: channel handle
** @return : TRUE on success. Others on error
** @note : none
**/
qbool Quos_socketChannelDel(void *chlFd);
/**
** @brief : notifies the socket connection result
** @param [in] sockFd: socket handle
** @param [in] type: socket type
** @param [in] result: socket connect result
** @return : none
** @note : When the socket is a non-blocking connection, the upper application needs to be notified by this function when the connection is successful
**/
void Quos_socketIoConnResult(pointer_t sockFd, quint8_t type, qbool result);
/**
** @brief : process data received by the socket
** @param [in] sockFd: socket handle
** @param [in] type: socket type
** @param [in] peer: peer
** @param [in] peerSize: peer len
** @param [in] buf: recv data
** @param [in] bufLen: recv data len
** @return : none.
** @note : none
**/
void Quos_socketIoRx(pointer_t sockFd, quint8_t type, const void *peer, quint32_t peerSize, quint8_t *buf, quint32_t bufLen);
/**
** @brief : notifies the socket send result
** @param [in] sockFd: socket handle
** @param [in] type: socket type
** @return : none
** @note : When the socket is a non-blocking send, the upper-layer application needs to be notified by this function when the send is successful
**/
void Quos_socketIoTxCb(pointer_t sockFd, quint8_t type);
/**
** @brief : universal sending interface
** @param [in] chlFd: channel handle
** @param [in] peer: peer
** @param [in] sendCnt:send count
** @param [in] sendTimeout: send timeout
** @param [in] sendCB: send callback
** @param [in] recvCB: recv callback
** @param [in] pkgId: package id
** @param [in] buf: send data
** @param [in] bufLen: send data len
** @param [in] param: extra parameter
** @param [in] priority: priority send
** @return : TRUE on success. Others on error
** @note : none
**/
qbool Quos_socketTx(const void *chlFd, void *peer, quint8_t sendCnt, quint32_t sendTimeout, Quos_socketSendNodeCb_f sendCB, Quos_socketRecvNodeCb_f recvCB,
                            quint32_t pkgId, quint8_t *buf, quint16_t bufLen, const void *param, qbool priority);
/**
** @brief : sending interface
** @param [in] chlFd: channel handle
** @param [in] peer: peer
** @param [in] sendCnt:send count
** @param [in] sendTimeout: send timeout
** @param [in] sendCB: send callback
** @param [in] recvCB: recv callback
** @param [in] pkgId: package id
** @param [in] buf: send data
** @param [in] bufLen: send data len
** @param [in] param: extra parameter
** @return : TRUE on success. Others on error
** @note : none
**/
#define Quos_socketOrderedTx(chlFd, peer, sendCnt, sendTimeout, sendCB, recvCB, pkgId, buf, bufLen, param) Quos_socketTx(chlFd, peer, sendCnt, sendTimeout, sendCB, recvCB, pkgId, buf, bufLen, param, FALSE)
/**
** @brief : priority sending interface
** @param [in] chlFd: channel handle
** @param [in] peer: peer
** @param [in] sendCnt:send count
** @param [in] sendTimeout: send timeout
** @param [in] sendCB: send callback
** @param [in] recvCB: recv callback
** @param [in] pkgId: package id
** @param [in] buf: send data
** @param [in] bufLen: send data len
** @param [in] param: extra parameter
** @return : TRUE on success. Others on error
** @note : none
**/
#define Quos_socketPriorityTx(chlFd,peer,sendCnt,sendTimeout,sendCB,recvCB,pkgId,buf,bufLen,param) Quos_socketTx(chlFd,peer,sendCnt,sendTimeout,sendCB,recvCB,pkgId,buf,bufLen,param,TRUE)
/**
** @brief : unordered send interface
** @param [in] chlFd: channel handle
** @param [in] peer: peer
** @param [in] buf: send data
** @param [in] bufLen: send data len
** @return : TRUE on success. Others on error
** @note : none
**/
qbool Quos_socketUnorderedTx(const void *chlFd, void *peer, quint8_t *buf, quint16_t bufLen);
/**
** @brief : ack data was received
** @param [in] chlFd: channel handle
** @param [in] peer: peer
** @param [in] pkgId: package id
** @param [in] recvData: recv data
** @return : TRUE on success. Others on error
** @note : This function notifies upper-layer applications of ack data received from the peer end
**/
qbool Quos_socketTxAck(const void *chlFd, const void *peer, quint32_t pkgId, const void *recvData);
/**
** @brief : Gets the specified sending node for the specified channel
** @param [in] chlFd: channel handle
** @param [in] index: ordinal index value
** @return : send note
** @note : none
**/
Quos_socketSendDataNode_t *Quos_socketGetOrderSendNode(const void *chlFd, quint32_t index);
/**
** @brief : Gets the socket and type through the channel
** @param [in] chlFd: channel handle
** @param [out] sockFd: socket handle
** @param [out] type: socket type
** @return : TRUE on success. Others on error
** @note : none
**/
qbool Quos_socketGetSockFdType(const void *chlFd, pointer_t *sockFd, quint8_t *type);
/**
** @brief : Gets the channel through the socket and type
** @param [in] sockFd: socket handle
** @param [in] type: socket type
** @return : channel handle
** @note : none
**/
void *Quos_socketGetChlFd(pointer_t sockFd, quint8_t type);
/**
** @brief : Gets the channel list through the type and param
** @param [in] type: socket type
** @param [in] param: socket param
** @param [out] chlFd: channel list
** @param [in] maxSize: channel list max size
** @return : channel list count
** @note : none
**/
quint32_t Quos_socketGetChlFdList(quint8_t type, const void *param, void *chlFd[], quint32_t maxSize);
/**
** @brief : Check whether the channel is valid
** @param [in] chlFd: channel handle
** @return : TRUE on valid. Others on error
** @note : none
**/
qbool Quos_socketCheckChlFd(const void *chlFd);
/**
** @brief : get channel list
** @param : none
** @return : channel list
** @note : none
**/
Quos_twll_t *Quos_socketGetChlHead(void);
/**
** @brief : Change channel information
** @param [in] chlFd: channel handle
** @param [in] sendCnt: send count
** @param [in] sendTimeout: send timeout
** @param [in] recvDataFunc: recv callback
** @return : TRUE on success. Others on error
** @note : none
**/
qbool Quos_socketInfoModity(void *chlFd, quint8_t sendCnt, quint32_t sendTimeout, Quos_socketRecvNotifyCb_f recvDataFunc);
/**
** @brief : Gets the length of data to be sent
** @param [in] chlFd: channel handle
** @return : Length of data to be sent
** @note : none
**/
quint32_t Quos_socketSendDataByteSize(const void *chlFd);

/**
** @brief : socket task
** @param [in] : none
** @return : Next task arrival time, unit:ms
** @note: none
**/
quint32_t Quos_socketTask(void);

/**
** @brief : clean send node
** @param [in] chlFd: channel handle
** @param [in] type: send node type
** @return : none
** @note: none
**/
void Quos_socketClearSendNode(void *chlFd, Quos_socketSendNodeType_e type);

#ifdef __cplusplus
}
#endif
#endif /* __QUOS_SOCKET_H__ */
