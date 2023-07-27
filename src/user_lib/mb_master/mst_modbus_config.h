#ifndef MST_MODBUS_CONF_H_INCLUDED
#define MST_MODBUS_CONF_H_INCLUDED

#include <stdint.h>

//-----------------------------------------------------------------------
// Modbus registers X macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// define
//-----------------------------------------------------------------------

//	 Number		Name    *ip_str  flag    *data  *arg_1  *handler
//
#define MST_BUF_TABLE(a)\
    a##_X_MST_PAR(0, DEV_1, "testttt0", 0, 0, 0, 0, 0 ) \
    a##_X_MST_PAR(1, DEV_2, "testttt1", 0, 0, 0, 0, 0 ) \
    a##_X_MST_PAR(2, DEV_3, "testtttDEV_3", 0, 0, CYCLIC_D,  33, 0) \
    a##_X_MST_PAR(3, DEV_4, "testtttDEV_4", 0, 0, EVENT_D,  44, 0) \
    a##_X_MST_PAR(4, DEV_5, "testttt4", 0, 0, 0, 0, 0)
#endif /* MST_MODBUS_CONF_H_INCLUDED */