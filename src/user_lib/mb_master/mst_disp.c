
#include <mst_disp.h>

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

//-----------------------------------------------------------------------
// Variable
//-----------------------------------------------------------------------

// //-----------------------------------------------------------------------
// // Function
// //-----------------------------------------------------------------------
static mst_ret_t mst_disconnect(mst_t *mst);
static mst_ret_t mst_prepare_connect(mst_t *mst);
static mst_ret_t mst_create_request(mst_t *mst);
static mst_ret_t mst_modbus_init(mst_t *mst);
// //-----------------------------------------------------------------------
// // State machine processing
// //-----------------------------------------------------------------------
mst_ret_t mst_modbus_iteration(mst_t *mst, mst_event_t  event)
{
    switch (mst->state)
    {
    case MST_INIT:
        mst_modbus_init(mst);
        break;
    case MST_IDLE:
        mst->state = MST_DISCONNECT;
        /* fall-through */
    case MST_DISCONNECT:
        if (mst_disconnect(mst) != RET_OK)
            return RET_ERROR;
        break;
    case MST_PREPARE_CONNECT:
        if (mst_prepare_connect(mst) != RET_OK)
            return RET_ERROR;
        break;
    case MST_CREATE_REQ:
        if (mst_create_request(mst) == RET_ERROR)
            return RET_ERROR;
        break;
    default:
        break;
    }
    return RET_OK;
}

// //-----------------------------------------------------------------------
// // init
// //-----------------------------------------------------------------------
static mst_ret_t mst_modbus_init(mst_t *mst)
{
    if (mst->default_cb == NULL)
        return RET_ERROR;
    mst->request_number = 0;
    mst->device_number = 0;
    mst->state = MST_IDLE;
    memset(mst->lan_state, 0, sizeof(mst->lan_state) * mst->max_device);
    memset(mst->event, 0, sizeof(mst->event) * mst->max_device);
    return RET_OK;
}

// //-----------------------------------------------------------------------
// // disconnect func for frame like TCP/IP
// //-----------------------------------------------------------------------
static mst_ret_t mst_disconnect(mst_t *mst)
{
    mst_ret_t ret;
    ret = mst->default_cb(mst);
    if (ret == RET_OK)
    {
        mst->state = MST_PREPARE_CONNECT;
        return RET_OK;
    }
    else if (ret == RET_WAIT)
    {
        mst->state = MST_WAIT;
        return RET_OK;
    }
    mst->state = MST_ERROR;
    return RET_ERROR;
}

// //-----------------------------------------------------------------------
// // connect func for frame like TCP/IP
// //-----------------------------------------------------------------------
static mst_ret_t mst_prepare_connect(mst_t *mst)
{
    // uint8_t lan_state_temp;
    // mst_ret_t ret;
    // if (mst->state == MST_PREPARE_CONNECT || mst->connect != NULL) // prepare cb is mandatory
    // {
    //     // connect only if not lan error or there are try permission;
    //     if ((mst->lan_state[mst->device_number] & LAN_TRY_BITS) != LAN_ERROR_BIT)
    //     {
    //         mst->state = MST_CONNECT;
    //         mst->cb_data.device_number = mst->device_number;
    //         mst->cb_data.request_number = mst->request_number;
    //         ret = mst->connect(&mst->cb_data);
    //         if (ret == RET_OK)
    //             mst->state = MST_CREATE_REQ;
    //         else if (ret == RET_NEXT_DEVICE)
    //             mst->state = MST_NEXT_DEV;
    //         else if (ret != RET_WAIT)
    //         {
    //             mst->state = MST_ERROR;
    //             return RET_ERROR;
    //         }
    //         return RET_OK;
    //     }
    //     else
    //     {
    //         // lan error, prepare future connection permission if posible
    //         lan_state_temp = LAN_TRY_PERMISSION;
    //         for (uint32_t i = 0; i < mst->device_number; i++)
    //         {
    //             if (mst->lan_state[i] & LAN_TRY_PERMISSION)
    //                 lan_state_temp = 0;
    //         }
    //         mst->lan_state[mst->device_number] |= lan_state_temp;
    //         mst->state = MST_NEXT_DEV;
    //         return RET_OK;
    //     }
    // }
    // return RET_ERROR;
}

// //-----------------------------------------------------------------------
// // prepare buffer for write to device
// //-----------------------------------------------------------------------
static mst_ret_t mst_create_request(mst_t *mst)
{
    // mst_ret_t ret;
    // if (mst->state == MST_CREATE_REQ)
    // {
    //     if (mst->cb_data.frame_type = MST_TYPE_RTU)
    //     {

    //     }
    //     //address
    //     mst->cb_data.frame_buf[0] = (mst->cb_data.dev_id);
    //     //function id
    //     mst->cb_data.frame_buf[1] = mst->cb_data.func_id;
    // }
    // return RET_ERROR;
}

// buf_ptr[mbb->mb_index++] = (uint8_t)(*(mbb->p_read + RegIndx) >> 8);
// buf_ptr[mbb->mb_index++] = (uint8_t)(*(mbb->p_read + RegIndx) & 0xFF);
