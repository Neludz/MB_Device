
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
static mst_ret_t mst_wait(mst_t *mst);
// //-----------------------------------------------------------------------
// // State machine processing
// //-----------------------------------------------------------------------
mst_ret_t mst_modbus_iteration(mst_t *mst, mst_event_t event)
{
    mst_ret_t ret;
    switch (mst->state)
    {
    case MST_INIT:
        ret = mst_modbus_init(mst);
        break;
    case MST_IDLE:
        mst->state = MST_DISCONNECT;
        /* fall-through */
    case MST_DISCONNECT:
        ret = mst_disconnect(mst);
        break;
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
    case MST_WAIT:
        ret = mst_wait(mst);
        break;
    default:
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
        return RET_ERROR;
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
        mst->state = MST_PREPARE_CONNECT;
    else if (ret == RET_WAIT)
    {
        mst->state_after_wait = MST_PREPARE_CONNECT;
        mst->state = MST_WAIT;
    }
    else
    {
        mst->state = MST_ERROR;
        ret = RET_ERROR;
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
         ((mst->dev_params->flag & REQUEST_TYPE_MASK_D) == CYCLIC_D)) ||
        mst_check_current_event(mst, EVENT_REQUEST))
    {
        mst_reset_current_event(mst, EVENT_REQUEST);
        mst->lan_state[mst->device_number] &= ~LAN_TRY_PERMISSION;
        ret = mst->default_cb(mst);
        if (ret == RET_OK) // connected
            mst->state = MST_CREATE_REQ;
        else if (ret == RET_CONNECT_TIMEOUT)
        {
            mst->state = MST_ERROR;
        }
        else if (ret == RET_WAIT)
        {
            // wait connect
            mst->state_after_wait = MST_CREATE_REQ;
            mst->state = MST_WAIT;
        }
        else
        {
            mst->state = MST_ERROR;
            return RET_ERROR;
        }
        return ret;
    }
    // reate permission for next try
    else if ((mst->dev_params->flag & REQUEST_TYPE_MASK_D) == CYCLIC_D)
    {
        // lan error, prepare future connection permission if posible
        lan_state_temp = LAN_TRY_PERMISSION;
        for (uint32_t i = 0; i < mst->max_device; i++)
        {
            if (mst->lan_state[i] & LAN_TRY_PERMISSION)
                lan_state_temp = 0;
            break;
        }
        mst->lan_state[mst->device_number] |= lan_state_temp;
    }
    // event-device without event
    mst->state = MST_NEXT_DEV;
    return RET_NEXT_DEVICE;
}

// //-----------------------------------------------------------------------
// // prepare buffer for write to device
// //-----------------------------------------------------------------------
static mst_ret_t mst_create_request(mst_t *mst)
{
    mst_ret_t ret;

    if (mst->dev_params->device_cb)
        ret = mst->dev_params->device_cb(mst);
    else
        ret = mst->default_cb(mst);

    if (ret == RET_OK)
        ret = mst_fill_buff(mst); // fill request buffer

    if (ret != RET_OK)
    {
        mst->state = MST_ERROR;
        ret = RET_ERROR;
    }
    else
    {
        mst->state = MST_SEND_REQ;
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
    if (ret == RET_OK || ret == RET_WAIT)
    {
        // wait connect
        mst->state_after_wait = MST_PARSE;
        mst->state = MST_WAIT;
    }
    else
    {
        mst->state = MST_ERROR;
        return RET_ERROR;
    }
    if (ret != RET_OK)
    {
        mst->state = MST_ERROR;
        ret = RET_ERROR;
    }
    else
    {
        mst->state = MST_SEND_REQ;
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
    mst->state = MST_ERROR;
    return MST_ERROR;
}
// //-----------------------------------------------------------------------
// // done -> OK
// //-----------------------------------------------------------------------
static mst_ret_t mst_done(mst_t *mst)
{
    mst_ret_t ret;

    if (mst->dev_params->device_cb)
        ret = mst->dev_params->device_cb(mst);
    else
        ret = mst->default_cb(mst);

    switch (ret)
    {
    case RET_OK:
    case RET_NEXT_DEVICE:
        mst->state = MST_NEXT_DEV;
        break;
   case RET_NEXT_REQUEST:
        mst->state = MST_NEXT_REQ;
        break;
    case 
    default:
        break;
    }

    if (ret == RET_OK || ret == MST_NEXT_DEV)
         mst->state = MST_NEXT_DEV;
    else if (ret == MST_NEXT_REQ)
        mst->state = MST_NEXT_REQ;

    {
        mst->state = MST_ERROR;
        ret = MST_ERROR;
    }
    else
    {
        mst->state = MST_SEND_REQ;
    }
    return ret;


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