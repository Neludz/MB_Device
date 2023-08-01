#ifndef MST_MODBUS_CONF_H_INCLUDED
#define MST_MODBUS_CONF_H_INCLUDED

#include <stdint.h>
#include "mst_modbus_cb.h"

//-----------------------------------------------------------------------
// Modbus registers X macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// define
//-----------------------------------------------------------------------

//	        Number  Name    *ip_str          port   data    flag      *arg_1  *handler
//
#define MST_BUF_TABLE(a)\
    a##_X_MST_PAR(0, DEV_1  , "192.168.92.1", 5000  , 0     , CYCLIC_D  , 0 , mst_user_cb ) \
    a##_X_MST_PAR(1, DEV_2  , "192.168.92.1", 5001  , 0     , CYCLIC_D  , 0 , mst_user_cb ) \
    a##_X_MST_PAR(2, DEV_3  , "192.168.92.1", 5002  , 0     , CYCLIC_D  , 0 , mst_user_cb) \
    a##_X_MST_PAR(3, DEV_4  , "192.168.92.1", 5003  , 0     , EVENT_D   , 0 , mst_user_cb) \
    a##_X_MST_PAR(4, DEV_5  , "192.168.92.1", 5004  , 0     , EVENT_D   , 0 , mst_user_cb)
#endif /* MST_MODBUS_CONF_H_INCLUDED */