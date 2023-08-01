#ifndef MASTER_DISP_H_INCLUDED
#define MASTER_DISP_H_INCLUDED
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

//-----------------------------------------------------------------------
// Debug
//-----------------------------------------------------------------------

#define MST_FRAME_MIN 4          /* Minimal size of a Modbus RTU frame	*/
#define MST_FRAME_MAX 256        /* Maximal size of a Modbus RTU frame	*/
#define MST_ADDRESS_BROADCAST 00 /* MBBuff[0] analysis					*/

#define MST_ANY_ADDRESS 00 /* 0 - any address						*/
#define MST_MAX_REG 120    /*max quantity registers in inquiry. Should be less than MB_FRAME_MAX considering service bytes. Use for 03 function*/

#define MST_TRANSACTION_ID_MASK 0xFF

#define MST_MAX_LAN_ERROR_COUNT 0x03

// request flags
#define F_TYPE_RTU              (0x00UL << 0)
#define F_TYPE_TCP              (0x01UL << 0)
#define F_TYPE_USER_FILL_BUF    (0x01UL << 1)

typedef struct mst_s mst_t;

typedef enum
{
    RET_OK = 0,
    RET_ERROR,
    RET_SERVICE,
    RET_WAIT,
    RET_WAIT_MESSAGE,
    RET_NEXT_DEVICE,
    RET_NEXT_REQUEST,
    RET_REPEAT_REQUEST,
    RET_PARSE_ERROR,
    RET_PARSE_OK,
    RET_FREE,
    RET_CONNECT_TIMEOUT,
} mst_ret_t;

typedef enum
{
    MST_NONE = 0x00,
    MST_ILLEGAL_FUNCTION = 0x01,
    MST_ILLEGAL_DATA_ADDRESS = 0x02,
    MST_ILLEGAL_DATA_VALUE = 0x03
} mst_excep_t;

typedef enum
{
    MST_STATUS_FREE = 0x00,
    MST_STATUS_WAIT = 0x01,
} mst_wait_status_t;

typedef enum
{
    MST_EV_CONTINUE = 0x01,
    MST_EV_NEXT = 0x01,
} mst_event_t;

typedef enum
{
    MST_NOEVENT,
    MST_HAPPEND,
} mst_frame_bad_events_t;

typedef enum
{
    MST_INIT,
    MST_IDLE,
    MST_DONE,
    MST_WAIT,
    MST_DISCONNECT,
    MST_PREPARE_CONNECT,
    MST_TRY_TO_CONNECT,
    MST_CREATE_REQ,
    MST_SEND_REQ,
    MST_PARSE,
    MST_WRITE_REQ,
    MST_READ,
    MST_ERROR,
    MST_NEXT_DEV,
    MST_NEXT_REQ,
    MST_REPEAT_REQ,
    MST_SERVICE,
    MST_ACYCLIC_CONNECT,
    MST_TIMEOUT_CONNECT,
    MST_TIMEOUT_REQUEST,
} mst_state_t;
// lan state bits
#define LAN_TRY_PERMISSION  (0x01 << 1)     /* 0b00000000*/
#define LAN_ERROR_BIT       (0x01 << 0)     /* 0b00000000*/
#define LAN_TRY_BITS        (0x03 << 0)     /* 0b00000000 */
// event and notification bits
#define EVENT_NEXT          (0x01 << 0)
#define EVENT_REQUEST       (0x01 << 1)
#define EVENT_START_PARSE   (0x01 << 2)

#define DATA_X_MST_PAR(a, b, c, d, e, f, g, h) {c, d, e, f, (void*)g, h},
#define NAME_X_MST_PAR(a, b, c, d, e, f, g, h) b,

typedef struct
{
    const char *str_ip;
    const uint16_t port;
    const uint16_t data;
    const uint16_t flag;
    const void *arg_1;
    mst_ret_t (*const device_cb)(mst_t *mst_data);
} mst_dev_param_t;

#define EVENT_D (0x00)
#define CYCLIC_D (1 << 0x00)    // 0 bit  
#define REQUEST_TYPE_MASK_D (0x01)    // 0 bit  

typedef struct
{
    uint8_t dev_id;
    uint8_t func_id; 
    uint16_t reg_id;
    uint16_t len;
    uint16_t data_16;
    uint32_t req_flags;
    void *user_data;
    uint8_t *w_data_buf;
    uint8_t *r_data_buf;
} cb_req_data_t;

struct mst_s
{
    mst_state_t state;
    uint16_t request_number;
    uint16_t device_number;
    uint16_t max_device;
    mst_state_t  state_after_wait;
    mst_wait_status_t wait_status;
    uint8_t *lan_state;
    uint8_t *lan_err_count;
    uint32_t *event;
    mst_event_t msg_event;
    uint8_t trans_id;
    uint16_t len;
    mst_frame_bad_events_t er_frame_bad;
    mst_excep_t parse_status;
    cb_req_data_t current_req_inst;
    const mst_dev_param_t *dev_params;
    mst_ret_t (*default_cb)(mst_t *mst_data);
    uint8_t *tx_buf;
    uint8_t *rx_buf;
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
    static uint8_t instance_name##_lan_state[device_params##_NUM_STRING] = {0};        \
    static uint8_t instance_name##_lan_err_count[device_params##_NUM_STRING] = {0};    \
    static uint32_t instance_name##_event[device_params##_NUM_STRING] = {0};           \
    static mst_t(instance_name) = {                                                    \
        .default_cb = default_ev_handler,                                              \
        .dev_params = instance_name##_dev_params,                                      \
        .max_device = device_params##_NUM_STRING,                                      \
        .lan_state = instance_name##_lan_state,                                        \
        .lan_err_count = instance_name##_lan_err_count,                                \
        .state = MST_INIT,                                                             \
        .event = instance_name##_event,                                                \
        .request_number = 0,                                                           \
        .device_number = 0,                                                            \
        .wait_status = MST_STATUS_FREE,                                                \
    };

mst_ret_t mst_modbus_iteration(mst_t *mst);
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

mst_event_t mst_get_current_event(mst_t *mst);
mst_ret_t mst_reset_current_event(mst_t *mst, mst_event_t reset_event);
mst_event_t mst_check_current_event(mst_t *mst, mst_event_t check_event);
mst_ret_t mst_check_wait(mst_t *mst);

mst_ret_t mst_change_state(mst_t *mst, mst_state_t new_state);
mst_state_t mst_check_state(mst_t *mst);
#endif /* MASTER_DISP_H_INCLUDED */
