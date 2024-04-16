
#include <mst_disp.h>
#include <mst_modbus.h>
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
static mst_ret_t mst_modbus_init(mst_t *mst);
static mst_ret_t mst_disconnect(mst_t *mst);
static mst_ret_t mst_prepare_connect(mst_t *mst);
static mst_ret_t mst_try_to_connect(mst_t *mst);
static mst_ret_t mst_create_request(mst_t *mst);
static mst_ret_t mst_send_request(mst_t *mst);
static mst_ret_t mst_parse_request(mst_t *mst);
static mst_ret_t mst_done(mst_t *mst);
static mst_ret_t mst_error(mst_t *mst);
static mst_ret_t mst_next(mst_t *mst);
static mst_ret_t mst_service(mst_t *mst);
static mst_ret_t mst_wait(mst_t *mst);
// //-----------------------------------------------------------------------
// // State machine processing
// //-----------------------------------------------------------------------
mst_ret_t mst_modbus_iteration(mst_t *mst)
{
    mst_ret_t ret;
    switch (mst->state)
    {
    case MST_INIT:
        ret = mst_modbus_init(mst);
        break;
    case MST_DISCONNECT:
        ret = mst_disconnect(mst);
        break;
    case MST_IDLE:
        mst->state = MST_PREPARE_CONNECT;
        /* fall-through */
    case MST_PREPARE_CONNECT:
        ret = mst_prepare_connect(mst);
        break;
    case MST_CREATE_REQ:
        ret = mst_create_request(mst);
        break;
    case MST_SEND_REQ:
        ret = mst_send_request(mst);
        break;
    case MST_PARSE:
        ret = mst_parse_request(mst);
        break;
    case MST_DONE:
        ret = mst_done(mst);
        break;
    case MST_ERROR:
        ret = mst_error(mst);
        break;
    case MST_NEXT_DEV:
    case MST_NEXT_REQ:
        ret = mst_next(mst);
        break;
    case MST_WAIT:
        ret = mst_wait(mst);
        break;
    case MST_SERVICE:
        ret = mst_service(mst);
        break;
    default:
        ret = RET_ERROR;
        mst->state = MST_SERVICE;
        break;
    }
    return ret;
}

// //-----------------------------------------------------------------------
// // init
// //-----------------------------------------------------------------------
static mst_ret_t mst_modbus_init(mst_t *mst)
{
    if (mst->default_cb == NULL)
    {
        mst->state = MST_SERVICE;
#ifdef USER_DEBUG
        printf("[ERROR] MST default callback not init \n");
#endif
        return RET_SERVICE;
    }
    else
        mst->default_cb(mst);

    mst->state = MST_IDLE;
    return RET_OK;
}

// //-----------------------------------------------------------------------
// // disconnect func for frame like TCP/IP
// //-----------------------------------------------------------------------
static mst_ret_t mst_disconnect(mst_t *mst)
{
    mst_ret_t ret;
    mst_reset_current_event(mst, EVENT_NEXT);
    ret = mst->default_cb(mst);
    if (ret == RET_OK)
        mst->state = MST_NEXT_DEV;
    else if (ret == RET_WAIT)
    {
        mst->state_after_wait = MST_NEXT_DEV;
        mst->state = MST_WAIT;
    }
    else
    {
        mst->state = MST_SERVICE;
        ret = RET_SERVICE;
#ifdef USER_DEBUG
        printf("[ERROR] MST default callback can not disconnect \n");
#endif
    }
    return ret;
}

// //-----------------------------------------------------------------------
// // connect func for frame like TCP/IP
// //-----------------------------------------------------------------------
static mst_ret_t mst_prepare_connect(mst_t *mst)
{
    mst_ret_t ret;
    uint32_t lan_state_temp;
    // check connect permission
    if ((((mst->lan_state[mst->device_number] & LAN_TRY_BITS) != LAN_ERROR_BIT) &&
         ((mst->dev_params[mst->device_number].flag & REQUEST_TYPE_MASK_D) == CYCLIC_D)) ||
        mst_check_current_event(mst, EVENT_REQUEST))
    {
        mst_reset_current_event(mst, EVENT_REQUEST);
        // mst->lan_state[mst->device_number] &= ~LAN_TRY_PERMISSION;
        ret = mst->default_cb(mst);
        if (ret == RET_OK) // connected
            mst->state = MST_CREATE_REQ;
        else if (ret == RET_WAIT)
        {
            // wait connect
            mst->state_after_wait = MST_CREATE_REQ;
            mst->state = MST_WAIT;
        }
        else
        {
            mst->state = MST_ERROR;
#ifdef USER_DEBUG
            printf("[WARNING] MST device [%d] not avalible\n", mst->device_number);
#endif
            return RET_ERROR;
        }
        return ret;
    }
    // create permission for next try
    else if ((mst->dev_params[mst->device_number].flag & REQUEST_TYPE_MASK_D) == CYCLIC_D)
    {
        // lan error, prepare future connection permission if posible
        lan_state_temp = LAN_TRY_PERMISSION;
        for (uint32_t i = 0; i < mst->max_device; i++)
        {
            if (mst->lan_state[i] & LAN_TRY_PERMISSION)
            {
                lan_state_temp = 0;
                break;
            }
        }
        mst->lan_state[mst->device_number] |= lan_state_temp;
    }
    // event-device without event
    mst->state = MST_NEXT_DEV;
    return RET_OK;
}

// //-----------------------------------------------------------------------
// // prepare buffer for write to device
// //-----------------------------------------------------------------------
static mst_ret_t mst_create_request(mst_t *mst)
{
    mst_ret_t ret;

    if (mst->dev_params[mst->device_number].device_cb)
        ret = mst->dev_params[mst->device_number].device_cb(mst);
    else
        ret = mst->default_cb(mst);

    if (ret == RET_OK)
    {
        ret = mst_fill_buff(mst); // fill request buffer
        if (ret != RET_OK)
        {
            mst->state = MST_SERVICE;
#ifdef USER_DEBUG
            printf("[ERROR] MST device [%d] can't create request\n", mst->device_number);
#endif
            ret = RET_SERVICE;
        }
        else
        {
            mst->state = MST_SEND_REQ;
        }
    }
    else if (ret == RET_NEXT_REQUEST)
    {
        mst->state = MST_NEXT_REQ;
    }
    else if (ret == RET_NEXT_DEVICE)
    {
        mst->state = MST_NEXT_DEV;
    }
    return ret;
}
// //-----------------------------------------------------------------------
// // send packet
// //-----------------------------------------------------------------------
static mst_ret_t mst_send_request(mst_t *mst)
{
    mst_ret_t ret;
    ret = mst->default_cb(mst);
    if (ret == RET_WAIT)
    {
        // wait connect
        mst->state_after_wait = MST_PARSE;
        mst->state = MST_WAIT;
    }
    else if (ret == RET_OK)
    {
        mst->state = MST_PARSE;
    }
    else
    {
        mst->state = MST_ERROR;
#ifdef USER_DEBUG
       printf("[ERROR] MST device [%d] can't send request\n", mst->device_number);
#endif
        ret = RET_ERROR;
    }
    return ret;
}
// //-----------------------------------------------------------------------
// // parse packet
// //-----------------------------------------------------------------------
static mst_ret_t mst_parse_request(mst_t *mst)
{
    if (mst->er_frame_bad == MST_NOEVENT)
    {
        // parse request buffer
        if (mst_parse_buff(mst) == RET_OK)
        {
            mst->state = MST_DONE;
            return RET_OK;
        }
    }
#ifdef USER_DEBUG
   // printf("[ERROR] MST device [%d] parsing error\n", mst->device_number);
#endif
    mst->state = MST_ERROR;
    return RET_ERROR;
}
// //-----------------------------------------------------------------------
// // done -> OK
// //-----------------------------------------------------------------------
static mst_ret_t mst_done(mst_t *mst)
{
    mst_ret_t ret;
    mst->trans_id++;
    mst->lan_err_count[mst->device_number] = 0;
    mst->lan_state[mst->device_number] &= (~LAN_TRY_BITS);

    if (mst->dev_params[mst->device_number].device_cb)
        ret = mst->dev_params[mst->device_number].device_cb(mst);
    else
        ret = mst->default_cb(mst);

    switch (ret)
    {
    case RET_OK:
    // fall through
    case RET_NEXT_DEVICE:
        mst->state = MST_DISCONNECT;
        break;
    case RET_NEXT_REQUEST:
        mst->state = MST_NEXT_REQ;
        break;
    case RET_REPEAT_REQUEST:
        mst->state = MST_CREATE_REQ;
        break;
    default:
        mst->state = MST_SERVICE;
        ret = RET_SERVICE;
#ifdef USER_DEBUG
        printf("[ERROR] MST device [%d] user parsing while DONE error\n", mst->device_number);
#endif
        break;
    }
    return ret;
}
// //-----------------------------------------------------------------------
// // error -> from any state (except service error)
// //-----------------------------------------------------------------------
static mst_ret_t mst_error(mst_t *mst)
{
    mst_ret_t ret;
    mst->trans_id++;
    mst->lan_err_count[mst->device_number]++;
    mst->lan_state[mst->device_number] &= ~LAN_TRY_PERMISSION;
    if (mst->lan_err_count[mst->device_number] >= MST_MAX_LAN_ERROR_COUNT)
    {
        mst->lan_err_count[mst->device_number] = MST_MAX_LAN_ERROR_COUNT;
        mst->lan_state[mst->device_number] |= LAN_ERROR_BIT;
    }

    if (mst->dev_params[mst->device_number].device_cb)
        ret = mst->dev_params[mst->device_number].device_cb(mst);
    else
        ret = mst->default_cb(mst);

    switch (ret)
    {
    case RET_OK:
    // fall through
    case RET_NEXT_DEVICE:
        mst->state = MST_DISCONNECT;
        break;
    default:
        mst->state = MST_SERVICE;
        ret = RET_SERVICE;
#ifdef USER_DEBUG
        printf("[ERROR] MST device [%d] user parsing while ERROR error\n", mst->device_number);
#endif
        break;
    }
    return ret;
}

// //-----------------------------------------------------------------------
// // next
// //-----------------------------------------------------------------------
static mst_ret_t mst_next(mst_t *mst)
{
    if (mst->state == MST_NEXT_DEV)
    {
        mst->device_number++;
        if (mst->device_number >= mst->max_device)
            mst->device_number = 0;

        mst->request_number = 0;
        mst->state = MST_IDLE;
    }
    else if (mst->state == MST_NEXT_REQ)
    {
        mst->request_number++;
        mst->state = MST_CREATE_REQ;
    }
    return RET_OK;
}

// //-----------------------------------------------------------------------
// // service
// //-----------------------------------------------------------------------
static mst_ret_t mst_service(mst_t *mst)
{
    mst->state = MST_DISCONNECT;
#ifdef USER_DEBUG
    printf("[ERROR] MST device [%d] SERVICE\n", mst->device_number);
#endif

    return RET_OK;
}
// //-----------------------------------------------------------------------
// // wait
// //-----------------------------------------------------------------------
static mst_ret_t mst_wait(mst_t *mst)
{
    mst_ret_t ret;
    mst->wait_status = MST_STATUS_WAIT;
    if (mst_check_current_event(mst, EVENT_NEXT))
    {
        mst_reset_current_event(mst, EVENT_NEXT);
        mst->state = mst->state_after_wait;
        mst->wait_status = MST_STATUS_FREE;
        return RET_OK;
    }
    else
        return RET_WAIT;
}

// //-----------------------------------------------------------------------
// // check ivent
// //-----------------------------------------------------------------------
mst_event_t mst_get_current_event(mst_t *mst)
{
    return mst->event[mst->device_number];
}

mst_ret_t mst_set_current_event(mst_t *mst, mst_event_t event)
{
    mst->event[mst->device_number] |= event;
    return RET_OK;
}

mst_ret_t mst_reset_current_event(mst_t *mst, mst_event_t reset_event)
{
    mst->event[mst->device_number] &= ~reset_event;
    return RET_OK;
}

mst_event_t mst_check_current_event(mst_t *mst, mst_event_t check_event)
{
    return mst->event[mst->device_number] & check_event;
}

mst_event_t mst_get_event(mst_t *mst, uint32_t id, mst_event_t event)
{
    if (id < mst->max_device)
        return mst->event[id];
    else
        return 0;
}

mst_ret_t mst_set_event(mst_t *mst, uint32_t id, mst_event_t event)
{
    if (id < mst->max_device)
    {
        mst->event[id] |= event;
        return RET_OK;
    }
    else
        return RET_ERROR;
}

mst_ret_t mst_reset_event(mst_t *mst, uint32_t id, mst_event_t reset_event)
{
    if (id < mst->max_device)
        return mst->event[id] &= ~reset_event;
    else
        return RET_ERROR;
}

mst_event_t mst_check_event(mst_t *mst, uint32_t id, mst_event_t check_event)
{
    if (id < mst->max_device)
        return mst->event[id] & check_event;
    else
        return 0;
}
// //-----------------------------------------------------------------------
// // check wait status
// //-----------------------------------------------------------------------
mst_ret_t mst_check_wait(mst_t *mst)
{
    if (mst->state == MST_WAIT)
        return RET_WAIT;
    else
        return RET_FREE;
}

// //-----------------------------------------------------------------------
// // change main dispatcher state
// //-----------------------------------------------------------------------
mst_ret_t mst_change_state(mst_t *mst, mst_state_t new_state)
{
    mst->state = new_state;
    return RET_OK;
}

// //-----------------------------------------------------------------------
// // check dispatcher state
// //-----------------------------------------------------------------------
mst_state_t mst_check_state(mst_t *mst)
{
    return mst->state;
}