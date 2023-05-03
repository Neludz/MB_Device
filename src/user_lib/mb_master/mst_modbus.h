#ifndef MASTER_MODBUS_H_INCLUDED
#define MASTER_MODBUS_H_INCLUDED

#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

unsigned int mst_crc16(unsigned char *puch_msg, unsigned int data_len);

#endif /* MASTER_MODBUS_H_INCLUDED */
