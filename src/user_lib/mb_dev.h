#ifndef _MB_DEV_H
#define _MB_DEV_H 1

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "mb_dev_config.h"

#define MAX_REG 1000
//--------------------------
//define all data in modbus regiter string
#define LAST_REG_DATA_IN_STR 5

typedef enum
{
  ADDRESS_IN_STR = 1,
  REG_IN_STR = 2,
  TYPE_IN_STR = 3,
  FUNC_IN_STR = 4,
  OPTION_IN_STR = 5,
} data_in_str_t;
//--------------------------

extern const char *str_with_type[];
extern const char **str_with_type1[];
//define type register in modbus regiter string
#define IO_TABLE\
  X_IO(REG_BYTE_0A,       "byte_0a",        &test_print, REG1_FLOAT_CDAB)	\
  X_IO(REG_BYTE_A0,       "byte_a0",        &test_print, REG_INT16_AB)	\
  X_IO(REG_INT16_AB,      "int16_ab",       &test_print, REG_INT16_AB)	\
  X_IO(REG_INT16_BA,      "int16_ba",       &test_print, REG_INT16_AB)	\
  X_IO(REG_INT32_ABCD,    "int32_abcd",     &test_print, REG_INT16_AB)	\
  X_IO(REG_INT32_CDAB,    "int32_cdba",     &test_print, REG_INT16_AB)	\
  X_IO(REG_FLOAT_ABCD,    "float32_abcd" ,  &test_print, REG_INT16_AB) \
  X_IO(REG_FLOAT_CDAB,    "float32_cdab" ,  &test_print, REG_INT16_AB) \

typedef enum
{
#define X_IO(a,b,c,d) a,
	IO_TABLE
#undef X_IO
	NUM_REG_TYPE
} regtype_t;
//--------------------------

//define type register in modbus regiter string
#define IO_TABLE_1\
  X_IO(REG1_BYTE_0A,       &str_with_type[REG1_BYTE_0A],    &test_print1, REG1_BYTE_A0)	\
  X_IO(REG1_BYTE_A0,       &str_with_type[REG1_BYTE_0A],    &test_print, REG_INT16_AB)	\
  X_IO(REG1_INT16_AB,      &str_with_type[REG1_BYTE_0A],    &test_print, REG_INT16_AB)	\
  X_IO(REG1_INT16_BA,      &str_with_type[REG1_BYTE_0A],    &test_print, REG_INT16_AB)	\
  X_IO(REG1_INT32_ABCD,    &str_with_type[REG1_BYTE_0A],    &test_print, REG_INT16_AB)	\
  X_IO(REG1_INT32_CDAB,    &str_with_type[REG1_BYTE_0A],    &test_print, REG_INT16_AB)	\
  X_IO(REG1FLOAT_ABCD,     &str_with_type[REG1_BYTE_0A],    &test_print, REG_INT16_AB) \
  X_IO(REG1_FLOAT_CDAB,    &str_with_type[REG1_BYTE_0A],    &test_print, REG_INT16_AB) \

typedef enum
{
#define X_IO(a,b,c,d) a,
	IO_TABLE_1
#undef X_IO
	NUM_REG_TYPE1
} regtype_1_t;


#define MB_FUNC_NONE							00
#define MB_FUNC_READ_COILS						01
#define MB_FUNC_READ_DISCRETE_INPUTS			02
#define MB_FUNC_WRITE_SINGLE_COIL				05
#define MB_FUNC_WRITE_MULTIPLE_COILS			15
#define MB_FUNC_READ_HOLDING_REGISTER			03	/* implemented now	*/
#define MB_FUNC_READ_INPUT_REGISTER				04
#define MB_FUNC_WRITE_REGISTER					06
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS		16
#define MB_FUNC_READWRITE_MULTIPLE_REGISTERS	23
#define MB_FUNC_ERROR							0x80

#define MB_SPAN_CREATE_FUNC ((1 << MB_FUNC_READ_HOLDING_REGISTER) |\
                             (1 << MB_FUNC_READ_INPUT_REGISTER))

//--------------------------
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
  uint8_t err;
  uint8_t func;
  uint32_t start_reg;
  uint32_t count_reg;
  //uint8_t read_request[8];
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
  uint32_t max_space_span;
  uint32_t reg_count;
  uint32_t request_count;
  mb_reg_t *p_reg;
  mb_request_span_t *p_span;
} mb_config_data_t;

mb_config_data_t *mb_config(char *st_config);
void test_print();
void test_print1();
#endif /* mb_dev.h */