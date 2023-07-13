#ifndef MASTER_DISP_H_INCLUDED
#define MASTER_DISP_H_INCLUDED
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "mst_modbus.h"
#include "mst_modbus_config.h"

//-----------------------------------------------------------------------
// Debug
//-----------------------------------------------------------------------

#define MST_FRAME_MIN 4          /* Minimal size of a Modbus RTU frame	*/
#define MST_FRAME_MAX 256        /* Maximal size of a Modbus RTU frame	*/
#define MST_ADDRESS_BROADCAST 00 /* MBBuff[0] analysis					*/

#define MST_ANY_ADDRESS 00 /* 0 - any address						*/
#define MST_MAX_REG 120    /*max quantity registers in inquiry. Should be less than MB_FRAME_MAX considering service bytes. Use for 03 function*/

#define MST_TRANSACTION_ID_MASK 0xFF
#define ACYCLIC_FIFO_SIZE 0xFF // power of 2

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

#define F_TYPE_RTU (0x00UL << 0)
#define F_TYPE_TCP (0x01UL << 0)

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

#define mst_event_t uint8_t
#define mst_state_t uint8_t

enum
{
    MST_INIT,
    MST_IDLE,
    MST_DONE,
    MST_WAIT,
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
    MST_TIMEOUT_CONNECT,
    MST_TIMEOUT_REQUEST,
};

#define LAN_TRY_PERMISSION (0x01 << 4) /* 0b00010000 */
#define LAN_ERROR_BIT (0x01 << 5)      /* 0b00100000 */
#define LAN_TRY_BITS (0x03 << 4)       /* 0b00110000 */
#define LAN_COUNTER_MASK (0x0F)        /* 0b00001111 */

typedef struct
{
    uint8_t dev_id;
    uint8_t func_id;
    uint16_t reg_id;
    uint16_t w_len;
    uint32_t req_flags;
    void *user_data;
    uint8_t *w_data_buf;
} cb_req_data_t;

struct mst_s
{
    mst_state_t state;
    uint16_t request_number;
    uint16_t device_number;
    uint16_t max_device;
    uint8_t *lan_state;
    uint8_t *event;`
    mst_event_t msg_event;
    uint8_t trans_id;
    uint16_t len;
    mst_excep_t parse_status;
    cb_req_data_t current_req_inst;
    const mst_dev_param_t *dev_params;
    mst_ret_t (*default_cb)(const mst_t *mst_data);
    uint8_t frame_buf[MST_FRAME_MAX];
};

#define MST_INSTANCE_DEF(instance_name,                                                \
                         default_ev_handler,                                           \
                         device_params)                                                \
    enum                                                                               \
    {                                                                                  \
        device_params(NAME)                                                            \
            device_params##_NUM_STRING                                                 \
    };                                                                                 \
    static const mst_dev_param_t instance_name##_dev_params[] = {device_params(DATA)}; \
    static uint8_t instance_name##_lan_state[device_params##_NUM_STRING];              \
    static uint8_t instance_name##_event[device_params##_NUM_STRING];                  \
    static mst_t(instance_name) = {                                                    \
        .default_cb = default_ev_handler,                                              \
        .dev_params = instance_name##_dev_params,                                      \
        .max_device = device_params##_NUM_STRING,                                      \
        .lan_state = instance_name##_lan_state,                                        \
        .state = MST_INIT,                                                             \
        .event = instance_name##_event,                                                \
    };

mst_ret_t mst_create_req_buf(mst_t *mst_inst,
                             cb_req_data_t *req_data,
                             uint8_t *buf,
                             uint16_t *buf_len);
mst_ret_t mst_parse_data(mst_t *mst_inst,
                         uint8_t *data,
                         uint16_t *data_len,

                         uint8_t *buf,
                         uint16_t *buf_len,
                         mst_excep_t parse_error);
mst_dev_param_t mst_get_dev_param(mst_t *mst_inst);
mst_state_t mst_get_status(mst_t *mst_inst);
uint16_t mst_get_request_number(mst_t *mst_inst);
mst_ret_t mst_set_notified(mst_t *mst_inst, uint32_t notification_value);
mst_ret_t mst_get_current_req(mst_t *mst_inst, cb_req_data_t *req_data);
mst_ret_t mst_set_dev_event(mst_t *mst_inst, uint16_t number_device);
mst_ret_t mst_set_state(mst_t *mst_inst, mst_state_t state);

mst_event_t mst_get_last_event(mst_t *mst_inst);
mst_ret_t mst_send_event(mst_t *mst_inst, mst_event_t event);
mst_ret_t mst_wait_event(mst_t *mst_inst, mst_event_t event);

#endif /* MASTER_DISP_H_INCLUDED */
