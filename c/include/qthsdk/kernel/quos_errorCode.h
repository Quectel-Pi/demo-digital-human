/*************************************************************************
  * @version : 0.0.1
  * @date : 2023.03.03
  * @brief : the define of error code for kernel level, range: -0x4000 ~ -0x7fff
  * @copyright : Copyright 2022-2023 Quectel Inc. All Rights Reserved.
*************************************************************************/

#ifndef __QUOS_ERRORCODE_H__
#define __QUOS_ERRORCODE_H__
#include "qhal_errorCode.h"
/****************************************************************************
            the error code marco define for module QUOS_AES
****************************************************************************/
#define OPRT_AES_INIT_CONTEXT_FAILED                        (-0x4000)  /* AES context initialization failed */
#define OPRT_AES_INIT_CONTEXT_IV_FAILED                     (-0x4001)  /* AES context iv initialization failed */


/****************************************************************************
            the error code marco define for module QUOS_EVENT
****************************************************************************/
#define OPRT_EVENT_INIT_FAILED                              (-0x4100)  /* event component initialization failed */
#define OPRT_EVENT_REG_CALLBACK_FAILED                      (-0x4101)  /* event register callback failed */
#define OPRT_EVENT_POST_FAILED                              (-0x4102)  /* event post failed */


/****************************************************************************
            the error code marco define for module QUOS_SWTIMER
****************************************************************************/
#define OPRT_SWTIMER_INIT_FAILED                            (-0x4200)  /* swTimer component initialization failed */
#define OPRT_SWTIMER_START_FAILED                           (-0x4201)  /* swTimer add failed */
#define OPRT_SWTIMER_DELETE_FAILED                          (-0x4202)  /* swTimer delete failed */


/****************************************************************************
            the error code marco define for module QUOS_TIME
****************************************************************************/
#define OPRT_TIME_INIT_FAILED                               (-0x4300)  /* time component initialization failed */


/****************************************************************************
            the error code marco define for module QUOS_LOG
****************************************************************************/
#define OPRT_LOG_INIT_FAILED                                (-0x4400)  /* log component initialization failed */
#define OPRT_LOG_INVALID_LEVEL                               (-0x4401)  /* log invalid level */

/****************************************************************************
            the error code marco define for module QUOS_SOCKET
****************************************************************************/
#define OPRT_SOCKET_INIT_FAILED                             (-0x4500)  /* socket component initialization failed */
#define OPRT_SOCKET_ADDCHL_FAILED                           (-0x4501)  /* socket add channel failed */

/****************************************************************************
            the error code marco define for module QUOS_MQTT
****************************************************************************/
#define OPRT_MQTT_INSIDE_FAILED                             (-0x4600)  //-17920, mqtt internal error
#define OPRT_MQTT_NET_FAILED                                (-0x4601)  //-17921, mqtt network error
#define OPRT_MQTT_PING_FAILED                               (-0x4602)  //-17922, mqtt heartbeat error
#define OPRT_MQTT_SUBSCRI_FAILED                            (-0x4603)  //-17923, mqtt subscribe error
#define OPRT_MQTT_CONNECT_FAILED                            (-0x4604)  //-17924, mqtt connect error
#define OPRT_MQTT_UNNACCEPTABLE_PROTOCOL                    (-0x4605)  //-17925, mqtt unacceptable protocol version
#define OPRT_MQTT_CLIENTID_REJECTED                         (-0x4606)  //-17926, mqtt client ID rejected
#define OPRT_MQTT_SERVER_UNAVAILABLE                        (-0x4607)  //-17927, mqtt server unavailable
#define OPRT_MQTT_BAD_USERNAME_OR_PASSWORD                  (-0x4608)  //-17928, mqtt bad username or password
#define OPRT_MQTT_NOT_AUTHORIZED                            (-0x4609)  //-17929, mqtt not authorized
#define OPRT_MQTT_URL_ANALYZE_FAILED                        (-0x460a)  //-17930, URL analyze failed
#define OPRT_MQTT_GET_IP_FAILED                             (-0x460b)  //-17931, get IP address failed
#define OPRT_MQTT_DATA_PACKAGE_FAILED                       (-0x460c)  //-17932, data package build failed
#define OPRT_MQTT_DATA_SEND_FAILED                          (-0x460d)  //-17933, data package send failed
#define OPRT_MQTT_DATA_HEAD_INVALID                         (-0x460e)  /* MQTT data packet header invalid */
#define OPRT_MQTT_DATA_LEN_INVALID                          (-0x460f)  /* MQTT data packet length invalid */
#define OPRT_MQTT_CHLFD_INVALID                             (-0x4610)  /* MQTT channel handle invalid */
#define OPRT_MQTT_CONNECT_TIMEOUT                           (-0x4611)  /* mqtt connect timeout */
#define OPRT_MQTT_SEND_LIST_FULL                            (-0x4612)  /* mqtt send list full */
#define OPRT_MQTT_SEND_TIMEOUT                              (-0x4613)  /* mqtt send error */
#define OPRT_MQTT_PING_TIMEOUT                              (-0x4614)  /* mqtt ping timeout */
#define OPRT_MQTT_TOPIC_OVERFLOW                            (-0x4615)  /* mqtt topic overflow */
/****************************************************************************
            the error code marco define for module QUOS_WORKQ
****************************************************************************/
#define OPRT_WORKQ_INIT_FAILED                              (-0x4700)  /* work queue initialization failed */
#define OPRT_WORKQ_FULL_FAILED                              (-0x4701)  /* work queue full */

/****************************************************************************
            the error code marco define for module QUOS_TOOLS
****************************************************************************/
#define OPRT_URL_ANALYZE_FAILED                             (-0x4800)  /* URL analyze failed */

/****************************************************************************
            the error code marco define for module QUOS_DATASTORE
****************************************************************************/
#define OPRT_DATASTORE_INVALID_CTX                          (-0x4900)  /* datastore invalid context */
#define OPRT_DATASTORE_INVALID_HEAD                         (-0x4901)  /* datastore invalid head */
#define OPRT_DATASTORE_WRITE_FAILED                         (-0x4902)  /* datastore write failed */
#define OPRT_DATASTORE_READ_FAILED                          (-0x4903)  /* datastore read failed */
#define OPRT_DATASTORE_INVALID_CRC                          (-0x4904)  /* datastore crc check failed */
#define OPRT_DATASTORE_INVALID_MASK                         (-0x4905)  /* datastore invalid mask */

/****************************************************************************
 *            the error code marco define for module QUOS_IPCACHE
****************************************************************************/
#define OPRT_IPCACHE_HOSTNAME_NOTFOUND                      (-0x4a00)  /* ipcache hostname not found */
#define OPRT_IPCACHE_DEFAULT_NOTFOUND                       (-0x4a01)  /* ipcache default address not found */
#define OPRT_IPCACHE_IP_NOTFOUND                            (-0x4a02)  /* ipcache ip address not found */
/****************************************************************************
            the error code marco define for module QUOS_HTTP
****************************************************************************/
#define OPRT_HTTP_CHLFD_INVALID                             (-0x4b00)  /* http channel handle invalid */
#define OPRT_HTTP_ISBUSY                                    (-0x4b01)  /* http channel is busy */
#define OPRT_HTTP_REQUEST_TIMEOUT                           (-0x4b02)  /* http request timeout */
#define OPRT_HTTP_HEADER_CB_RETURN_FALSE                    (-0x4b03)  /* http header callback return false */
#define OPRT_HTTP_HEADER_LENGTH_INVALID                     (-0x4b04)  /* http header length invalid */
#define OPRT_HTTP_NET_FAILED                                (-0x4b05)  /* http network error */
#define OPRT_HTTP_MEMORY_FAILED                             (-0x4b06)  /* http memory allocation failed */
#define OPRT_HTTP_PARAM_INVALID                             (-0x4b07)  /* http parameter error */
#define OPRT_HTTP_FILE_READ_FAILED                          (-0x4b08)  /* http file read failed */
#define OPRT_HTTP_FILE_WRITE_FAILED                         (-0x4b09)  /* http file write failed */
#define OPRT_HTTP_SOCKET_FAILED                             (-0x4b0a)  /* http socket error */
#define OPRT_HTTP_CONNECT_FAILED                            (-0x4b0b)  /* http connect failed */
#define OPRT_HTTP_SSL_FAILED                                (-0x4b0c)  /* http SSL error */
#define OPRT_HTTP_PARSE_FAILED                              (-0x4b0d)  /* http parse error */
#define OPRT_HTTP_PROTOCOL_FAILED                           (-0x4b0e)  /* http protocol error */
#define OPRT_HTTP_UNKNOWN_ERROR                             (-0x4b0f)  /* http unknown error */

/****************************************************************************
            the error code marco define for module QUOS_TXMGR
****************************************************************************/
#define OPRT_TXMGR_CHLFD_INVALID                            (-0x4c00)  /* txdata channel handle invalid */
#define OPRT_TXMGR_LABEL_NOTFOUND                           (-0x4c01)  /* txdata label not found */

/****************************************************************************
            the error code marco define for module QUOS_COAP
****************************************************************************/
#define OPRT_COAP_INIT_FAILED                               (-0x4d00)  /* CoAP connection initialization failed */
#define OPRT_COAP_DEINIT_FAILED                             (-0x4d01)  /* CoAP connection de-initialization failed */
#define OPRT_COAP_MESSAGE_FORMAT_FAILED                     (-0x4d02)  /* CoAP message serialization failed */
#define OPRT_COAP_MESSAGE_PARSE_FAILED                      (-0x4d03)  /* CoAP message deserialization failed */
#define OPRT_COAP_OPTION_SET_FAILED                         (-0x4d04)  /* CoAP option set failed */
#define OPRT_COAP_OPTION_NOT_FOUND                          (-0x4d05)  /* CoAP option not found */
#define OPRT_COAP_PAYLOAD_SIZE_EXCEEDED                     (-0x4d06)  /* CoAP payload size exceeded */
#define OPRT_COAP_URL_PATH_INVALID                          (-0x4d07)  /* CoAP URL path invalid */
#define OPRT_COAP_PARAM_INVALID                             (-0x4d08)  /* CoAP function parameter invalid */
#define OPRT_COAP_SEND_FAILED                               (-0x4d09)  /* CoAP message send failed */
#define OPRT_COAP_BUFFER_TOO_SMALL                          (-0x4d0a)  /* CoAP buffer too small */
#define OPRT_COAP_MSG_CLONE_FAILED                          (-0x4d0b)  /* CoAP message clone failed */
#define OPRT_COAP_SOCKET_TX_FAILED                          (-0x4d0c)  /* CoAP socket send failed */

/****************************************************************************
            the error code marco define for module QUOS_WEBSOCKET
****************************************************************************/
#define OPRT_WEBSOCKET_INVALID_ARG           (-0x4e00)  /* WebSocket invalid argument */
#define OPRT_WEBSOCKET_INIT_FAILED           (-0x4e01)  /* WebSocket client initialization failed */
#define OPRT_WEBSOCKET_URI_PARSE_FAILED      (-0x4e02)  /* WebSocket URI parse failed */
#define OPRT_WEBSOCKET_NO_MEM                (-0x4e03)  /* WebSocket memory allocation failed */
#define OPRT_WEBSOCKET_NOT_CONNECTED         (-0x4e04)  /* WebSocket client not connected */
#define OPRT_WEBSOCKET_CONNECT_FAILED        (-0x4e05)  /* WebSocket connect failed */
#define OPRT_WEBSOCKET_SEND_FAILED           (-0x4e06)  /* WebSocket send failed */
#define OPRT_WEBSOCKET_RECV_FAILED           (-0x4e07)  /* WebSocket receive failed */
#define OPRT_WEBSOCKET_TIMEOUT               (-0x4e08)  /* WebSocket operation timed out */
#define OPRT_WEBSOCKET_ALREADY_STARTED       (-0x4e09)  /* WebSocket client already started */
#define OPRT_WEBSOCKET_NOT_STARTED           (-0x4e0a)  /* WebSocket client not started */
#define OPRT_WEBSOCKET_DESTROY_TIMEOUT       (-0x4e0b)  /* WebSocket destroy timeout waiting task exit */
#define OPRT_WEBSOCKET_TASK_CREATE_FAILED    (-0x4e0c)  /* WebSocket internal task create failed */
#define OPRT_WEBSOCKET_HANDSHAKE_FAILED      (-0x4e0d)  /* WebSocket HTTP handshake failed */


#endif /* __QUOS_ERRORCODE_H__ */
