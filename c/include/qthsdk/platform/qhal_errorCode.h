/**
 * @version 0.0.1
 * @date 2023.03.03
 * @brief the define of error code for hal level, range: -0x8000 ~ -0xbfff
 * @copyright Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 */
#ifndef __QHAL_ERRORCODE_H__
#define __QHAL_ERRORCODE_H__


/****************************************************************************
            the error code marco define for module GLOBAL
****************************************************************************/
#define OPRT_OK                                 (-0x0000)  // Operation succeeded
#define OPRT_COM_ERROR                          (-0x8000)  // Common error
#define OPRT_INVALID_PARM                       (-0x8001)  // Invalid parameter
#define OPRT_MALLOC_FAILED                      (-0x8002)  // Memory allocation failed
#define OPRT_NOT_SUPPORTED                      (-0x8003)  // Not supported
#define OPRT_NOT_FOUND                          (-0x8004)  // Object not found
#define OPRT_INDEX_OUT_OF_BOUND                 (-0x8005)  // Index out of bounds
#define OPRT_EXCEED_UPPER_LIMIT                 (-0x8006)  // Exceed upper limit
#define OPRT_NOT_EXIST                          (-0x8007)  // Does not exist
#define OPRT_IS_EXIST                           (-0x8008)  // Already exists

/****************************************************************************
            the error code marco define for module HAL_SEM
****************************************************************************/
#define OPRT_SEM_CREAT_FAILED                   (-0x8100)  // Semaphore creation failed
#define OPRT_SEM_WAIT_FAILED                    (-0x8101)  // Semaphore wait failed
#define OPRT_SEM_POST_FAILED                    (-0x8102)  // Semaphore post failed
#define OPRT_SEM_RELEASE_FAILED                 (-0x8103)  // Semaphore release failed
#define OPRT_SEM_WAIT_TIMEOUT                   (-0x8104)  // Semaphore wait timeout

/****************************************************************************
            the error code marco define for module HAL_MUTEX
****************************************************************************/
#define OPRT_MUTEX_CREAT_FAILED                 (-0x8200)  // Mutex creation failed
#define OPRT_MUTEX_LOCK_FAILED                  (-0x8201)  // Mutex lock failed
#define OPRT_MUTEX_UNLOCK_FAILED                (-0x8202)  // Mutex unlock failed
#define OPRT_MUTEX_RELEASE_FAILED               (-0x8203)  // Mutex release failed

/****************************************************************************
            the error code marco define for module HAL_THRD
****************************************************************************/
#define OPRT_THRD_CREAT_FAILED                  (-0x8300)  // Thread creation failed
#define OPRT_THRD_RELEASE_FAILED                (-0x8301)  // Thread release failed
#define OPRT_THRD_JUDGE_SELF_FAILED             (-0x8302)  // Thread judge self failed

/****************************************************************************
            the error code marco define for module HAL_QUEUE
****************************************************************************/
#define OPRT_QUEUE_CREAT_FAILED                 (-0x8400)  // Queue creation failed
#define OPRT_QUEUE_SEND_FAILED                  (-0x8401)  // Queue send failed
#define OPRT_QUEUE_RECV_FAILED                  (-0x8402)  // Queue receive failed

/****************************************************************************
            the error code marco define for module HAL_FLASH
****************************************************************************/
#define OPRT_FLASH_READ_FAILED                  (-0x8500)  // Flash read failed
#define OPRT_FLASH_WRITE_FAILED                 (-0x8501)  // Flash write failed
#define OPRT_FLASH_ERASE_FAILED                 (-0x8502)  // Flash erase failed

/****************************************************************************
            the error code marco define for module HAL_OTA
****************************************************************************/
#define OPRT_OTA_START_INFORM_FAILED            (-0x8600)  // Upgrade start notification failed
#define OPRT_OTA_PKT_SIZE_FAILED                (-0x8601)  // Upgrade packet size invalid
#define OPRT_OTA_PROCESS_FAILED                 (-0x8602)  // Upgrade packet download write failed
#define OPRT_OTA_VERIFY_FAILED                  (-0x8603)  // Upgrade packet verification failed
#define OPRT_OTA_END_INFORM_FAILED              (-0x8604)  // Upgrade end notification failed
#define OPRT_OTA_DOWNLOAD_FAILED                (-0x8605)  // Upgrade packet download failed
#define OPRT_OTA_DOWNLOAD_WAITING               (-0x8606)  // Upgrade packet waiting for download
#define OPRT_OTA_DOWNLOAD_DOING                 (-0x8607)  // Upgrade packet downloading

/****************************************************************************
            the error code marco define for module HAL_FILE
****************************************************************************/
#define OPRT_FILE_READ_FAILED                   (-0x8700)  // File read failed
#define OPRT_FILE_WRITE_FAILED                  (-0x8701)  // File write failed
#define OPRT_FILE_OPEN_FAILED                   (-0x8702)  // File open failed
#define OPRT_FILE_CLOSE_FAILED                  (-0x8703)  // File close failed
#define OPRT_FILE_REMOVE_FAILED                 (-0x8704)  // File delete failed
#define OPRT_FILE_SEEK_FAILED                   (-0x8705)  // File seek failed
#define OPRT_FILE_ACCESS_FAILED                 (-0x8706)  // File access failed
#define OPRT_FILE_GETSIZE_FAILED                (-0x8707)  // File size retrieval failed
#define OPRT_FILE_TELL_FAILED                   (-0x8708)  // File position retrieval failed

/****************************************************************************
            the error code marco define for module HAL_SOCKET
****************************************************************************/
#define OPRT_SOCK_CREATE_FAILED                 (-0x8800)  // Socket creation failed
#define OPRT_SOCK_CONN_FAILED                   (-0x8801)  // Socket connection failed
#define OPRT_SOCK_SET_SOCK_FAILED               (-0x8802)  // Socket setting failed
#define OPRT_SOCK_SEND_FAILED                   (-0x8803)  // Socket send failed
#define OPRT_SOCK_RECV_FAILED                   (-0x8804)  // Socket receive failed
#define OPRT_SOCK_DISCONNECTED                  (-0x8805)  // Socket disconnected
#define OPRT_SOCK_FD_INVALID                    (-0x8806)  // Socket FD invalid
#define OPRT_SOCK_BIND_FAILED                   (-0x8807)  // Socket bind failed
#define OPRT_SOCK_SETOPTION_FAILED              (-0x8808)  // Socket set option failed
#define OPRT_SOCK_LISTEN_FAILED                 (-0x8809)  // Socket listen failed
#define OPRT_SOCK_CONNECT_FAILED                (-0x880A)  // Socket connect failed
#define OPRT_SOCK_SHUTDOWN_FAILED               (-0x880B)  // Socket shutdown failed
#define OPRT_SOCK_CLOSE_FAILED                  (-0x880C)  // Socket close failed
#define OPRT_SOCK_GET_IP_FAILED                 (-0x880D)  // Socket DNS resolution failed
#define OPRT_SOCK_ACCEPT_FAILED                 (-0x880E)  // Socket accept failed
#define OPRT_SOCK_SELECT_FAILED                 (-0x880F)  // Socket select failed
#define OPRT_SOCK_CONNECT_INPROGRESS            (-0x8810)  // Socket connection in progress

/****************************************************************************
            the error code marco define for module HAL_UART
****************************************************************************/
#define OPRT_UART_INIT_FAILED                   (-0x8900)  // UART initialization failed
#define OPRT_UART_DEINIT_FAILED                 (-0x8901)  // UART deinitialization failed
#define OPRT_UART_SEND_FAILED                   (-0x8902)  // UART send failed
#define OPRT_UART_READ_FAILED                   (-0x8903)  // UART receive failed
#define OPRT_UART_BAUDRATE_INVALID              (-0x8904)  // UART baudrate invalid 
#define OPRT_UART_DATABIT_INVALID               (-0x8905)  // UART data bit invalid
#define OPRT_UART_STOPBIT_INVALID               (-0x8906)  // UART stop bit invalid
#define OPRT_UART_PARITY_INVALID                (-0x8907)  // UART parity invalid
#define OPRT_UART_FLOWCTRL_INVALID              (-0x8908)  // UART flow control invalid
#define OPRT_UART_PORT_INVALID                  (-0x8909) // UART port invalid


/****************************************************************************
            the error code marco define for module HAL_I2C
****************************************************************************/
#define OPRT_I2C_OPEN_FAILED                    (-0x8a00)  // I2C open failed
#define OPRT_I2C_CLOSE_FAILED                   (-0x8a01)  // I2C close failed
#define OPRT_I2C_READ_FAILED                    (-0x8a02)  // I2C read failed
#define OPRT_I2C_WRITE_FAILED                   (-0x8a03)  // I2C write failed
#define OPRT_I2C_INVALID_PARM                   (-0x8a04)  // Invalid parameter
#define OPRT_I2C_INIT_FAILED                    (-0x8a05)  // I2C initialization failed
#define OPRT_I2C_DEINIT_FAILED                  (-0x8a06)  // I2C deinitialization failed
#define OPRT_I2C_MODE_ERR                       (-0x8a07)  // I2C mode error
#define OPRT_I2C_BUSY                           (-0x8a08)  // I2C busy
#define OPRT_I2C_TIMEOUT                        (-0x8a09)  // I2C timeout
#define OPRT_I2C_MASTER_LOST_ARB                (-0x8a0a)  // I2C master arbitration lost
#define OPRT_I2C_BUS_ERR                        (-0x8a0b)  // I2C bus error
#define OPRT_I2C_TRANS_ERR                      (-0x8a0c)  // I2C transfer error
#define OPRT_I2C_ADDR_NO_ACK                    (-0x8a0d)  // I2C no ACK received
#define OPRT_I2C_IRQ_INIT_FAILED                (-0x8a0e)  // I2C interrupt initialization failed
#define OPRT_I2C_PORT_INVALID                   (-0x8a0f)  // I2C port invalid

/****************************************************************************
            the error code marco define for module HAL_BLE
****************************************************************************/
#define OPRT_BLE_HANDLE_ERROR                   (-0x8b00)  // BLE handle error
#define OPRT_BLE_BUSY                           (-0x8b01)  // BLE busy
#define OPRT_BLE_TIMEOUT                        (-0x8b02)  // BLE timeout
#define OPRT_BLE_INIT_FAILED                    (-0x8b03)  // BLE initialization failed
#define OPRT_BLE_DEINIT_FAILED                  (-0x8b04)  // BLE deinitialization failed
#define OPRT_BLE_GATT_CONN_FAILED               (-0x8b05)  // BLE GATT connection failed
#define OPRT_BLE_GATT_DISCONN_FAILED            (-0x8b06)  // BLE GATT disconnection failed
#define OPRT_BLE_ADV_START_FAILED               (-0x8b07)  // BLE advertisement start failed
#define OPRT_BLE_ADV_STOP_FAILED                (-0x8b08)  // BLE advertisement stop failed
#define OPRT_BLE_SCAN_START_FAILED              (-0x8b09)  // BLE scan start failed
#define OPRT_BLE_SCAN_STAOP_FAILED              (-0x8b0a)  // BLE scan stop failed
#define OPRT_BLE_SVC_DISC_FAILED                (-0x8b0b)  // BLE service discovery failed
#define OPRT_BLE_CHAR_DISC_FAILED               (-0x8b0c)  // BLE characteristic discovery failed
#define OPRT_BLE_DESC_DISC_FAILED               (-0x8b0d)  // BLE descriptor discovery failed
#define OPRT_BLE_NOTIFY_FAILED                  (-0x8b0e)  // BLE notify failed
#define OPRT_BLE_INDICATE_FAILED                (-0x8b0f)  // BLE indicate failed
#define OPRT_BLE_WRITE_FAILED                   (-0x8b10)  // BLE write failed
#define OPRT_BLE_READ_FAILED                    (-0x8b11)  // BLE read failed
#define OPRT_BLE_MTU_REQ_FAILED                 (-0x8b12)  // BLE MTU request failed
#define OPRT_BLE_MTU_REPLY_FAILED               (-0x8b13)  // BLE MTU reply failed
#define OPRT_BLE_CONN_PARAM_UPDATE_FAILED       (-0x8b14)  // BLE connection parameter update failed
#define OPRT_BLE_CONN_RSSI_GET_FAILED           (-0x8b15)  // BLE connection RSSI retrieval failed
#define OPRT_BLE_MESH_INVALID_OPCODE            (-0x8b16)  // BLE Mesh invalid opcode
#define OPRT_BLE_MESH_INVALID_ELEMENT           (-0x8b17)  // BLE Mesh invalid element
#define OPRT_BLE_MESH_INVALID_MODEL             (-0x8b18)  // BLE Mesh invalid model
#define OPRT_BLE_MESH_INVALID_ADDR              (-0x8b19)  // BLE Mesh invalid address
#define OPRT_BLE_MESH_INVALID_INDEX             (-0x8b1a)  // BLE Mesh invalid index
#define OPRT_BLE_MESH_NO_MEMORY                 (-0x8b1b)  // BLE Mesh memory full
#define OPRT_BLE_MESH_APPKEY_NOT_BOUND_MODEL    (-0x8b1c)  // BLE Mesh appkey not bound to model
#define OPRT_BLE_MESH_PROVISION_FAILED          (-0x8b1d)  // BLE Mesh provisioning failed
#define OPRT_BLE_MESH_COMPO_GET_FAILED          (-0x8b1e)  // BLE Mesh composition data retrieval failed
#define OPRT_BLE_MESH_MODEL_BIND_FAILED         (-0x8b1f)  // BLE Mesh model binding failed
#define OPRT_BLE_MESH_APPKEY_ADD_FAILED         (-0x8b20)  // BLE Mesh appkey add failed
#define OPRT_BLE_MESH_NETKEY_ADD_FAILED         (-0x8b21)  // BLE Mesh netkey add failed
#define OPRT_BLE_MESH_APPKEY_BIND_FAILED        (-0x8b22)  // BLE Mesh appkey bind failed
#define OPRT_BLE_MAC_GET_FAILED                 (-0x8b23)  // BLE MAC retrieval failed
#define OPRT_BLE_DISCONNECTED_BY_PEER           (-0x8b24)  // BLE connection disconnected by peer
#define OPRT_BLE_DISCONNECTED_BY_SELF           (-0x8b25)  // BLE connection disconnected by self
#define OPRT_BLE_DISCONNECTED_BY_CON_TIMEOUT    (-0x8b26)  // BLE connection disconnected due to timeout
#define OPRT_BLE_DISCONNECTED_BY_OTHER_REASON   (-0x8b27)  // BLE connection disconnected due to other reasons

/****************************************************************************
            the error code marco define for module HAL_SPI
****************************************************************************/
#define OPRT_SPI_INVALID_PARM                   (-0x8c00)  // SPI invalid parameter
#define OPRT_SPI_INIT_FAILED                    (-0x8c01)  // SPI initialization failed
#define OPRT_SPI_DEINIT_FAILED                  (-0x8c02)  // SPI deinitialization failed
#define OPRT_SPI_MODE_ERR                       (-0x8c03)  // SPI mode error
#define OPRT_SPI_BUSY                           (-0x8c04)  // SPI busy
#define OPRT_SPI_DATA_LOST                      (-0x8c05)  // SPI data lost
#define OPRT_SPI_TIMEOUT                        (-0x8c06)  // SPI timeout
#define OPRT_SPI_BUS_ERR                        (-0x8c07)  // SPI bus error
#define OPRT_SPI_TRANS_ERR                      (-0x8c08)  // SPI transfer error
#define OPRT_SPI_IRQ_ERR                        (-0x8c09)  // SPI interrupt error

/****************************************************************************
            the error code marco define for module HAL_PWM
****************************************************************************/
#define OPRT_PWM_INVALID_PARM                   (-0x8d00)  // PWM invalid parameter
#define OPRT_PWM_INIT_FAILED                    (-0x8d01)  // PWM initialization failed
#define OPRT_PWM_DEINIT_FAILED                  (-0x8d02)  // PWM deinitialization failed
#define OPRT_PWM_MODE_ERR                       (-0x8d03)  // PWM mode error
#define OPRT_PWM_BUSY                           (-0x8d04)  // PWM busy
#define OPRT_PWM_IRQ_INIT_FAILED                (-0x8d05)  // PWM interrupt initialization failed
#define OPRT_PWM_PORT_INVALID                   (-0x8d06)  // PWM port invalid

/****************************************************************************
            the error code marco define for module HAL_ADC
****************************************************************************/
#define OPRT_ADC_INVALID_PARM                   (-0x8e00)  // ADC invalid parameter
#define OPRT_ADC_INIT_FAILED                    (-0x8e01)  // ADC initialization failed
#define OPRT_ADC_DEINIT_FAILED                  (-0x8e02)  // ADC deinitialization failed
#define OPRT_ADC_READ_FAILED                    (-0x8e03)  // ADC read failed
#define OPRT_ADC_MODE_ERR                       (-0x8e04)  // ADC mode error
#define OPRT_ADC_BUSY                           (-0x8e05)  // ADC busy
#define OPRT_ADC_TIMEOUT                        (-0x8e06)  // ADC timeout
#define OPRT_ADC_IRQ_INIT_FAILED                (-0x8e07)  // ADC interrupt initialization failed
#define OPRT_ADC_PORT_INVALID                   (-0x8e08)  // ADC port invalid

/****************************************************************************
            the error code marco define for module HAL_DAC
****************************************************************************/
#define OPRT_DAC_INVALID_PARM                   (-0x8f00)  // DAC invalid parameter
#define OPRT_DAC_INIT_FAILED                    (-0x8f01)  // DAC initialization failed
#define OPRT_DAC_DEINIT_FAILED                  (-0x8f02)  // DAC deinitialization failed
#define OPRT_DAC_WRITE_FAILED                   (-0x8f03)  // DAC write failed
#define OPRT_DAC_MODE_ERR                       (-0x8f04)  // DAC mode error
#define OPRT_DAC_BUSY                           (-0x8f05)  // DAC busy
#define OPRT_DAC_TIMEOUT                        (-0x8f06)  // DAC timeout
#define OPRT_DAC_IRQ_INIT_FAILED                (-0x8f07)  // DAC interrupt initialization failed

/****************************************************************************
            the error code marco define for module HAL_I2S
****************************************************************************/
#define OPRT_I2S_INVALID_PARM                   (-0x9000)  // I2S invalid parameter
#define OPRT_I2S_INIT_FAILED                    (-0x9001)  // I2S initialization failed
#define OPRT_I2S_DEINIT_FAILED                  (-0x9002)  // I2S deinitialization failed
#define OPRT_I2S_WRITE_FAILED                   (-0x9003)  // I2S write failed
#define OPRT_I2S_READ_FAILED                    (-0x9004)  // I2S read failed
#define OPRT_I2S_MODE_ERR                       (-0x9005)  // I2S mode error
#define OPRT_I2S_BUSY                           (-0x9006)  // I2S busy
#define OPRT_I2S_TIMEOUT                        (-0x9007)  // I2S timeout
#define OPRT_I2S_DATA_LOST                      (-0x9008)  // I2S data lost
#define OPRT_I2S_TRANS_ERR                      (-0x9009)  // I2S transfer error
#define OPRT_I2S_IRQ_INIT_FAILED                (-0x900a)  // I2S interrupt initialization failed

/****************************************************************************
            the error code marco define for module HAL_WATCHDOG
****************************************************************************/
#define OPRT_WD_INIT_FAILED                     (-0x9100)  // Watchdog initialization failed

/****************************************************************************
            the error code marco define for module HAL_GPIO
****************************************************************************/
#define OPRT_GPIO_INOUT_SET_FAILED              (-0x9200)  // GPIO INOUT set failed
#define OPRT_GPIO_MODE_SET_FAILED               (-0x9201)  // GPIO mode set failed
#define OPRT_GPIO_WRITE_FAILED                  (-0x9202)  // GPIO write failed
#define OPRT_GPIO_IRQ_INIT_FAILED               (-0x9203)  // GPIO interrupt initialization failed
#define OPRT_GPIO_PORT_INVALID                  (-0x9204)  // GPIO port invalid
#define OPRT_GPIO_INIT_FAILED                   (-0x9205)  // GPIO initialization failed
#define OPRT_GPIO_DEINIT_FAILED                 (-0x9206)  // GPIO deinitialization failed
#define OPRT_GPIO_IRQ_DEINIT_FAILED             (-0x9207)  // GPIO interrupt deinitialization failed

/****************************************************************************
            the error code marco define for module HAL_CELL
****************************************************************************/
#define OPRT_CELL_IMEI_INVAILD                  (-0x9300)  // IMEI invalid
#define OPRT_CELL_CONNECT_FAILED                (-0x9301)  // CELL connection failed
#define OPRT_CELL_DISCONNECT_FAILED             (-0x9302)  // CELL disconnection failed
#define OPRT_CELL_GET_STATUS_FAILED             (-0x9303)  // CELL status retrieval failed
#define OPRT_CELL_GET_NWINFO_FAILED             (-0x9304)  // CELL network info retrieval failed
#define OPRT_CELL_GET_IMEI_FAILED               (-0x9305)  // CELL IMEI retrieval failed
#define OPRT_CELL_GET_LOCAL_IP_FAILED           (-0x9306)  // CELL local IP retrieval failed
#define OPRT_CELL_REGISTER_FAILED               (-0x9307)  // CELL network registration failed
#define OPRT_CELL_GET_APN_FAILED                (-0x9308)  // CELL APN retrieval failed

/****************************************************************************
            the error code marco define for module HAL_SIM
****************************************************************************/
#define OPRT_SIM_GET_STATUS_FAILED              (-0x9400)  // SIM card status retrieval failed
#define OPRT_SIM_GET_IMSI_FAILED                (-0x9401)  // SIM card IMSI retrieval failed
#define OPRT_SIM_GET_ICCI_FAILED                (-0x9402)  // SIM card ICCI retrieval failed
#define OPRT_SIM_GET_PHONE_NUM_FAILED           (-0x9403)  // SIM card phone number retrieval failed

/****************************************************************************
            the error code marco define for module HAL_GNSS
****************************************************************************/
#define OPRT_GNSS_GET_RAW_DATA_FAILED           (-0x9500)  // GNSS raw data retrieval failed

/****************************************************************************
            the error code marco define for module HAL_WIFI
****************************************************************************/
#define OPRT_WIFI_INIT_FAILED                   (-0x9600)  // WIFI initialization failed
#define OPRT_WIFI_CONNECT_FAILED                (-0x9601)  // WIFI connection failed
#define OPRT_WIFI_SCAN_FAILED                   (-0x9602)  // WIFI scan failed
#define OPRT_WIFI_DISCONNECT_FAILED             (-0x9603)  // WIFI disconnection failed
#define OPRT_WIFI_GET_MAC_FAILED                (-0x9604)  // WIFI MAC retrieval failed
#define OPRT_WIFI_GET_IP_FAILED                 (-0x9605)  // WIFI IP retrieval failed
#define OPRT_WIFI_GET_RSSI_FAILED               (-0x9606)  // WIFI RSSI retrieval failed
#define OPRT_WIFI_SET_COUNTRY_CODE_FAILED       (-0x9607)  // WIFI country code setting failed

/****************************************************************************
            the error code marco define for module HAL_ETH
****************************************************************************/
#define OPRT_ETH_INIT_FAILED                   (-0x9700)  // ETH initialization failed
#define OPRT_ETH_CONNECT_FAILED                (-0x9701)  // ETH connection failed
#define OPRT_ETH_DISCONNECT_FAILED             (-0x9702)  // ETH disconnection failed
#define OPRT_ETH_GET_STATUS_FAILED             (-0x9703)  // ETH status retrieval failed
#define OPRT_ETH_GET_IP_FAILED                 (-0x9704)  // ETH IP retrieval failed

/****************************************************************************
            the error code marco define for module HAL_WDG
****************************************************************************/
#define OPRT_WDG_INIT_FAILED                    (-0xA000)  // WDG initialization failed

#endif /* __QHAL_ERRORCODE_H__ */