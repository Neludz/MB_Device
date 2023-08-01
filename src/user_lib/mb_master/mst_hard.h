#ifndef MASTER_HARD_H_INCLUDED
#define MASTER_HARD_H_INCLUDED
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "mst_disp.h"

 void mst_master_init();
 mst_ret_t master_main_cb(mst_t *mst_data);

#endif /* MASTER_HARD_H_INCLUDED */
