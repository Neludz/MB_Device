#ifndef MASTER_MODBUS_H_INCLUDED
#define MASTER_MODBUS_H_INCLUDED

#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

unsigned int mst_crc16(unsigned char *puch_msg, unsigned int data_len);

typedef struct mst_s mst_t;
typedef enum
{
    RET_OK = 0,
    RET_ERROR,
    RET_WAIT,
    RET_WAIT_MESSAGE,
    RET_NEXT_DEVICE,
    RET_NEXT_REQUEST,
    RET_REPEAT_REQUEST,
    RET_PARSE_ERROR,
    RET_PARSE_OK,
} mst_ret_t;

#endif /* MASTER_MODBUS_H_INCLUDED */
