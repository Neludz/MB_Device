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
    a##_X_MST_PAR(0, DEV_1  , "192.168.0.6", 5000  , 1     , CYCLIC_D  , 0 , mst_user_cb ) \
    a##_X_MST_PAR(1, DEV_2  , "192.168.0.6", 5000  , 1     , CYCLIC_D  , 0 , mst_user_cb ) \
    a##_X_MST_PAR(2, DEV_3  , "192.168.0.6", 5000  , 1     , CYCLIC_D  , 0 , mst_user_cb) \
    a##_X_MST_PAR(3, DEV_4  , "192.168.0.6", 5000  , 1     , CYCLIC_D   , 0 , mst_user_cb) \
    a##_X_MST_PAR(4, DEV_5  , "192.168.0.6", 5000  , 1     , CYCLIC_D   , 0 , mst_user_cb)


//	        Number  Name    *ip_str          port   data    flag      *arg_1  *handler
//
#define MST_RS_485_1_BUF_TABLE(a)\
    a##_X_MST_PAR(0, RS_1  , NULL, 5000  , 1     , CYCLIC_D  , 0 , mst_rs_user_cb) \
    a##_X_MST_PAR(1, RS_2  , NULL, 5000  , 2     , CYCLIC_D  , 0 , mst_rs_user_cb) \
    a##_X_MST_PAR(2, RS_3  , NULL, 5001  , 3     , CYCLIC_D  , 0 , mst_rs_user_cb) \
    a##_X_MST_PAR(3, RS_4  , NULL, 5001  , 4     , CYCLIC_D  , 0 , mst_rs_user_cb) \
    a##_X_MST_PAR(4, RS_5  , NULL, 5001  , 1     , EVENT_D  , 0 ,  mst_rs_user_write_cb)

//	        Number  Name    *ip_str          port   data    flag      *arg_1  *handler
//
#define MST_RS_485_2_BUF_TABLE(a)\
    a##_X_MST_PAR(0, RS_2_1  , NULL, 5000  , 5     , CYCLIC_D  , 0 , mst_rs_user_cb) \
    a##_X_MST_PAR(1, RS_2_2  , NULL, 5000  , 5     , CYCLIC_D  , 0 , mst_rs_user_cb) \
    a##_X_MST_PAR(2, RS_2_3  , NULL, 5001  , 5     , CYCLIC_D  , 0 , mst_rs_user_cb) \
    a##_X_MST_PAR(3, RS_2_4  , NULL, 5001  , 5     , CYCLIC_D  , 0 , mst_rs_user_cb) \
    a##_X_MST_PAR(4, RS_2_5  , NULL, 5001  , 5     , EVENT_D  , 0 ,  mst_rs_user_write_cb)


#endif /* MST_MODBUS_CONF_H_INCLUDED */