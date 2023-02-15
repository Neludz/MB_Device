#ifndef _MB_DEV_H
#define _MB_DEV_H 1

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_REG 500

typedef enum
{
  REG_BYTE_0A,
  REG_BYTE_A0,
  REG_INT16_AB,
  REG_INT16_BA,
  REG_INT32_ABCD,
  REG_INT32_CDAB,
  REG_FLOAT_ABCD,
  REG_FLOAT_CDAB,
} regtype_t;

typedef enum
{
  MB_NO_ERROR = 0,
  MB_ERROR_FUNC,
  MB_ERROR_REG_ADDR,
  MB_ERROR_VAL,
  MB_ERROR_CRC,
  MB_ERROR_TIMEOUT,
  MB_ERROR_OTHER,
  MB_ERROR_DEVICE
} regerror_t;

typedef enum
{
  MB_WAIT = 0,
  MB_ERROR,
  MB_READOK,
  MB_SEND,
  MB_TOMEOUT,
  MB_NOTCONNECTED
} regstatus_t;

typedef enum
{
  MB_NONE = 0,
  MB_ODD,
  MB_EVEN
} portparity_t;

typedef struct
{
  uint8_t addr;
  uint32_t start_reg;
  uint32_t count_reg;
  uint8_t read_request[8];
} mb_request_span_t;

typedef struct
{
  uint8_t addr;
  uint8_t err;
  uint8_t func;
  uint8_t type;
  uint32_t reg;
  uint32_t user_option;
  uint32_t value;
} mb_reg_t;

typedef struct
{
  char uart_port[8];
  uint32_t baudrate;
  uint32_t parity;
  uint32_t vtime;
  uint32_t vmin;
  uint32_t max_request_span;
  uint32_t reg_count;
  uint32_t request_count;
  mb_reg_t *p_reg;
  mb_request_span_t *p_span;
} mb_config_data_t;

int mb_config(char *st_config);

#endif /* mb_dev.h */