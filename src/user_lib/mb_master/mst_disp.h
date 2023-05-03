#ifndef MASTER_DISP_H_INCLUDED
#define MASTER_DISP_H_INCLUDED

#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "mst_modbus_config.h"

//-----------------------------------------------------------------------
// Debug
//-----------------------------------------------------------------------

#define MST_FRAME_MIN 4          /* Minimal size of a Modbus RTU frame	*/
#define MST_FRAME_MAX 256        /* Maximal size of a Modbus RTU frame	*/
#define MST_ADDRESS_BROADCAST 00 /* MBBuff[0] analysis					*/

#define MST_ANY_ADDRESS 00 /* 0 - any address						*/
#define MST_MAX_REG 120    /*max quantity registers in inquiry. Should be less than MB_FRAME_MAX considering service bytes. Use for 03 function*/

#define MST_FUNC_NONE 00
#define MST_FUNC_READ_COILS 01
#define MST_FUNC_READ_DISCRETE_INPUTS 02
#define MST_FUNC_WRITE_SINGLE_COIL 05
#define MST_FUNC_WRITE_MULTIPLE_COILS 15
#define MST_FUNC_READ_HOLDING_REGISTER 03
#define MST_FUNC_READ_INPUT_REGISTER 04
#define MST_FUNC_WRITE_REGISTER 06
#define MST_FUNC_WRITE_MULTIPLE_REGISTERS 16
#define MST_FUNC_READWRITE_MULTIPLE_REGISTERS 23
#define MST_FUNC_ERROR 0x80

#define NOTIFIED_OK 01
#define NOTIFIED_ERROR 02
#define NOTIFIED_READ 03
#define NOTIFIED_WRITE 04
#define NOTIFIED_CONNECT 05
#define NOTIFIED_PARSE 06

typedef enum
{
    MST_NONE = 0x00,
    MST_ILLEGAL_FUNCTION = 0x01,
    MST_ILLEGAL_DATA_ADDRESS = 0x02,
    MST_ILLEGAL_DATA_VALUE = 0x03
} mst_excep_t;

// typedef enum
// {
//     MST_STATE_IDLE,			//
//     MST_STATE_RCVE,			//
//     MST_STATE_WAIT,	        //
//     MST_STATE_PARS,			//
//     MST_STATE_SEND,			//
//     MST_STATE_SENT,			//
//     MST_STATE_CONNECT,		//
//     MST_STATE_CONNECTED,	//
// } MSTDataSt_t;

typedef enum
{
    RET_OK = 0,
    RET_ERROR,
    RET_WAIT,
    RET_WAIT_MESSAGE,
    RET_NEXT_DEVICE,
    RET_NEXT_REQUEST,
    RET_REPEAT_REQUEST,
} mst_ret_t;

typedef enum
{
    MST_INIT,
    MST_IDLE,
    MST_DONE,
    MST_DISCONNECT,
    MST_PREPARE_CONNECT,
    MST_CONNECT,
    MST_CREATE_REQ,
    MST_WRITE_REQ,
    MST_READ,
    MST_ERROR,
    MST_NEXT_DEV,
    MST_NEXT_REQ,
    MST_SERVICE,
    MST_ACYCLIC_CONNECT,
} mst_state_t;

typedef struct
{
    char id;
    void *data;
} mst_msg_t;

typedef enum
{
    MST_TYPE_RTU,
    MST_TYPE_TCP
} mst_type_t;

#define LAN_TRY_PERMISSION (0x01 << 4) /* 0b00010000 */
#define LAN_ERROR_BIT (0x01 << 5)      /* 0b00100000 */
#define LAN_TRY_BITS (0x03 << 4)       /* 0b00110000 */
#define LAN_COUNTER_MASK (0x0F)        /* 0b00001111 */

typedef struct
{
    uint8_t *buf;
    uint16_t len;
    uint16_t status;
    const mst_dev_param_t *dev_param_inst;
    void *socket;
    void *user_data;
} cb_out_t;

typedef struct
{
    mst_type_t frame_type;
    uint8_t *w_data_buf;
    uint8_t dev_id;
    uint8_t func_id;
    uint16_t reg_id; 
    uint16_t w_len;
    void *socket;
    void *user_data;
    mst_ret_t (*acyclic_cb)(cb_out_t *data, mst_state_t event);
} cb_req_data_t;


typedef struct
{
    mst_state_t state;
    uint16_t request_number;
    uint16_t device_number;
    uint16_t max_device;
    uint8_t *lan_state;
    mst_msg_t msg;
    uint32_t notified_value;
    void *socket;
    void *user_data;
    const mst_dev_param_t *dev_param;
    mst_ret_t (*sm_user_cb)(cb_data_t *data, cb_out_t out, mst_state_t event);
    uint8_t frame_buf[MST_FRAME_MAX];
} mst_t;

#define MST_INSTANCE_DEF(instance_name,                                            \
                         user_ev_handler,                                          \
                         dev_params)                                               \
    static const mst_dev_param_t instance_name##_dev_param[] = {dev_params};       \
    static uint8_t instance_name##_lan_state[sizeof(instance_name##_dev_param) /   \
                                             sizeof(mst_dev_param_t)];             \
    static mst_t(instance_name) = {                                                \
        .sm_user_cb = user_ev_handler,                                             \
        .dev_param = instance_name##_dev_param,                                    \
        .max_device = sizeof(instance_name##_dev_param) / sizeof(mst_dev_param_t), \
        .state = MST_INIT,                                                         \
        .lan_state = instance_name##_lan_state,                                    \
    };
    

#endif /* MASTER_DISP_H_INCLUDED */
