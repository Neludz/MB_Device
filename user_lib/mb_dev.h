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
  reg_byte_0a,
  reg_byte_a0,
  reg_int16_ab,
  reg_int16_ba,
  reg_int32_abcd,
  reg_int32_cdab,
  reg_float_abcd,
  reg_float_cdab,
} regtype_t;

typedef enum
{
  error_no = 0,
  error_function,
  error_reg_address,
  error_value,
  error_crc,
  error_timeout,
  error_other,
  error_dev_address
} regerror_t;

typedef enum
{
  wait = 0,
  error,
  read_ok,
  send,
  time_out,
  not_connect
} regstatus_t;

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
  uint32_t value;
} mb_reg_t;

typedef struct
{
  uint32_t uart_port;
  uint32_t baudrate;
  uint32_t vtime;
  uint32_t vmin;
  uint32_t reg_count;
  uint32_t request_count;
  mb_reg_t  *p_reg;
  mb_request_span_t *p_span;
} mb_config_data_t;


int mb_config(char *st_config);

#endif /* mb_dev.h */