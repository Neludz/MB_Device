#ifndef MODBUS_HARD_H_INCLUDED
#define MODBUS_HARD_H_INCLUDED

#include <stdbool.h>
#include "inttypes.h"
#include <stdio.h>
#include <string.h>
#include <user_lib/configini.h>
#include <modbus.h>
#include <pthread.h>
#include <arpa/inet.h>

//config section
#define CONFIG_MB_TCP_SLAVE   "MB_TCP_SLAVE_"
#define CONFIG_MB_RTU_SLAVE   "MB_RTU_SLAVE_"
#define CONFIG_MAX_MBSLAVE_NUM      13

//config key
#define CONFIG_TCP_FRAME_TYPE   "TCP_Frame_type"
#define CONFIG_TCP_PORT         "Port"
#define DEFAULT_PORT            502
#define CONFIG_ADDR_ID          "Addr_ID"
#define ANY_MB_ADDR             0
#define DEFAULT_MB_ADDR         ANY_MB_ADDR

// typedef struct
// {
//   char uart_port[8];
//   uint32_t baudrate;
//   uint32_t parity;
//   uint32_t vtime;
//   uint32_t vmin;
// } mb_config_data_t;

typedef struct
{
  uint32_t Client_Sock_Inst;
  uint8_t Buf_Data[MB_FRAME_MAX];
  int32_t Port;
  MBStruct_t MB_Data_Inst;
  pthread_mutex_t socket_mutex;  
} mb_tcp_thread_data_t;

typedef enum
{
    NO_PARITY_1_STOP	= 0x00,
    NO_PARITY_2_STOP	= 0x01,
    EVEN_PARITY_1_STOP	= 0x02,
    ODD_PARITY_1_STOP	= 0x03,

} Parity_Stop_Bits_t;

// void mh_Write_Eeprom (void *mbb);
// void mh_Modbus_Init(void);
// void mh_USB_Init(void);
// void mh_USB_Transmit_Start (void *mbb);
// void mh_USB_Recieve(uint8_t *USB_buf, uint16_t len);
// void mh_RS485_Init(void);
// void mh_Rs485_Transmit_Start (void *mbb);
// void mh_Rs485_Recieve_Start (void *mbb);
// void rs485_timer_callback (xTimerHandle xTimer);
// void IO_Uart1_Init(void);
// void mh_task_Modbus (void *pvParameters);
// void mh_Factory (void);
// void mh_Buf_Init (void);
int32_t mh_Slave_Init(Config *cfg);
void mh_TCP_Transmit_Start (void *mbb);
void mh_Callback_TCP (void *mbb);
#endif /* MODBUS_HARD_H_INCLUDED */
