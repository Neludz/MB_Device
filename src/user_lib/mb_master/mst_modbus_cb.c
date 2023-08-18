
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
#include "modbus_hard.h"
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
        mst_data->current_req_inst.dev_id = mst_data->dev_params[mst_data->device_number].data;
        mst_data->current_req_inst.func_id = 3;
        mst_data->current_req_inst.len = 10;
        mst_data->current_req_inst.req_flags = F_TYPE_TCP;
#ifdef USER_DEBUG
        printf("[LOG_ETH] user CB [%d] create\n", mst_data->device_number);
#endif
        return RET_OK;
    case MST_DONE:
#ifdef USER_DEBUG
        printf("[LOG_ETH] user CB [%d] done :%d, %d, %d\n",
               mst_data->device_number,
               mst_data->frame_buf[0],
               mst_data->frame_buf[1],
               mst_data->frame_buf[2]);
#endif
        if (mst_data->request_number >= 2)
            return RET_NEXT_DEVICE;

        return RET_NEXT_REQUEST;
    case MST_ERROR:
#ifdef USER_DEBUG
        printf("[LOG_ETH] user CB [%d] error\n", mst_data->device_number);
#endif
        return RET_NEXT_DEVICE;
    default:
        return RET_ERROR;
    }
}

mst_ret_t(mst_rs_user_cb)(mst_t *mst_data)
{
    switch (mst_check_state(mst_data))
    {
    case MST_CREATE_REQ:
        mst_data->current_req_inst.dev_id = mst_data->dev_params[mst_data->device_number].data;
        mst_data->current_req_inst.reg_id = 0;
        mst_data->current_req_inst.func_id = 3;
        mst_data->current_req_inst.len = 2;
        mst_data->current_req_inst.req_flags = F_TYPE_RTU;
#ifdef USER_DEBUG
        printf("[LOG_RS] user CB [%d] create\n", mst_data->device_number);
#endif
        return RET_OK;
    case MST_DONE:
#ifdef USER_DEBUG
        printf("[LOG_RS] user CB [%d] done :%d, %d, %d\n",
               mst_data->device_number,
               mst_data->frame_buf[0],
               mst_data->frame_buf[1],
               mst_data->frame_buf[2]);
#endif
        return RET_NEXT_DEVICE;
    case MST_ERROR:
#ifdef USER_DEBUG
        printf("[LOG_RS] user CB [%d] error\n", mst_data->device_number);
#endif
        return RET_NEXT_DEVICE;
    default:
        return RET_ERROR;
    }
}

mst_ret_t(mst_rs_user_write_cb)(mst_t *mst_data)
{
    switch (mst_check_state(mst_data))
    {
    case MST_CREATE_REQ:
        // if (mst_data->request_number > 2)
        // {
        //     return RET_NEXT_DEVICE;
        // }
        mst_data->current_req_inst.data_16 = mh_get_data_mb_buf(mst_data->request_number + 5);
      //  if (mst_data->current_req_inst.data_16)
       // {
            mst_data->current_req_inst.dev_id = mst_data->dev_params[mst_data->device_number].data;
            mst_data->current_req_inst.reg_id = mst_data->request_number + 5;
            mst_data->current_req_inst.func_id = 6;
            mst_data->current_req_inst.len = 2;
           // mst_data->current_req_inst.data_16 = mh_get_reset_data_mb_buf(mst_data->request_number + 5);
            mst_data->current_req_inst.req_flags = F_TYPE_RTU;
#ifdef USER_DEBUG
            printf("[LOG_WRITE] user CB [%d] create\n", mst_data->device_number);
#endif
            return RET_OK;
       // }
        //return RET_NEXT_REQUEST;
    case MST_DONE:
#ifdef USER_DEBUG
        printf("[LOG_WRITE] write CB\n");
#endif
        if (mst_data->request_number > 2)
            return RET_NEXT_DEVICE;
        return RET_NEXT_REQUEST;
    case MST_ERROR:
#ifdef USER_DEBUG
        printf("[LOG_WRITE] user CB [%d] error\n", mst_data->device_number);
#endif
        return RET_NEXT_DEVICE;
    default:
        return RET_ERROR;
    }
}