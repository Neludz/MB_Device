#ifndef MASTER_MODBUS_H_INCLUDED
#define MASTER_MODBUS_H_INCLUDED

#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <mst_disp.h>

#define MST_FUNC_NONE							00
#define MST_FUNC_READ_COILS						01
#define MST_FUNC_READ_DISCRETE_INPUTS			02
#define MST_FUNC_WRITE_SINGLE_COIL				05
#define MST_FUNC_WRITE_MULTIPLE_COILS			15
#define MST_FUNC_READ_HOLDING_REGISTER			03	/* implemented now	*/
#define MST_FUNC_READ_INPUT_REGISTER            04
#define MST_FUNC_WRITE_REGISTER					06	/* implemented now	*/
#define MST_FUNC_WRITE_MULTIPLE_REGISTERS		16	/* implemented now	*/
#define MST_FUNC_READWRITE_MULTIPLE_REGISTERS	23
#define MST_FUNC_ERROR							0x80

mst_ret_t mst_fill_buff(mst_t *mst);
unsigned int mst_crc16(unsigned char *puch_msg, unsigned int data_len);
#endif /* MASTER_MODBUS_H_INCLUDED */
