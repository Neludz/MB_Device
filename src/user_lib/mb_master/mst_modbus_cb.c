
#include <mst_modbus_cb.h>

#include <stdbool.h>
#include "inttypes.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdint.h>
#include <ctype.h>
#include "user_lib/configini.h"
#include "mst_disp.h"
//-----------------------------------------------------------------------
// Variable
//-----------------------------------------------------------------------

// //-----------------------------------------------------------------------
// // Function
// //-----------------------------------------------------------------------
mst_ret_t(mst_user_cb)(mst_t *mst_data)
{
    switch (mst_check_state(mst_data))
    {
    case MST_CREATE_REQ:
        mst_data->current_req_inst.dev_id = 1;
        mst_data->current_req_inst.func_id = 3;
        mst_data->current_req_inst.len = 10;
        mst_data->current_req_inst.req_flags = F_TYPE_TCP;
        printf("[LOG] user CB [%d] create\n", mst_data->device_number);
        return RET_OK;
    case MST_DONE:
        printf("[LOG] user CB [%d] done :%d, %d, %d\n",
               mst_data->device_number,
               mst_data->frame_buf[0],
               mst_data->frame_buf[1],
               mst_data->frame_buf[2]);

        return RET_OK;
    case MST_ERROR:
        printf("[LOG] user CB [%d] error\n", mst_data->device_number);
        return RET_OK;
    default:
        return RET_ERROR;
    }
}
