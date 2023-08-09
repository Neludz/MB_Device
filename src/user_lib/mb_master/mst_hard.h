#ifndef MASTER_HARD_H_INCLUDED
#define MASTER_HARD_H_INCLUDED
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "mst_disp.h"


//queue msg
typedef struct msg_mst {
    long mtype;       /* message type, must be > 0 */
    int data;    /* message data */
}msg_mst_t;

 void mst_master_init();
 mst_ret_t master_main_cb(mst_t *mst_data);
 mst_ret_t master_rs485_cb(mst_t *mst_data);


void write_rs(uint16_t reg_id, uint16_t data);
#endif /* MASTER_HARD_H_INCLUDED */
