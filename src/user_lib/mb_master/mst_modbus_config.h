#ifndef MST_MODBUS_CONF_H_INCLUDED
#define MST_MODBUS_CONF_H_INCLUDED

#include <stdint.h>

//-----------------------------------------------------------------------
// Modbus registers X macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// define
//-----------------------------------------------------------------------

#define MST_ASYNC_QUEUE_SIZE	10

#define MB_LIMIT_REG	        1 // check limit
#define MB_CALLBACK_REG	        1 // use write callback
//#define MB_USER_ARG1_REG	    1 // use user argument (for example: run user callback after write function)
//#define MB_USER_ARG2_REG	    1 // not implement now
#define MB_TCP_PERMISSION       1 // TCP frame avalible
//#define MB_REG_END_TO_END     1 // If not define "MB_REG_END_TO_END" then number register is determined in "a" field from X-macros


#define REG_END_REGISTER                Reg_End

typedef struct
{
    void *arg_1;
    void *arg_2;
    void *arg_3;
    void (*mst_dev_cb)(void *data);
} mst_dev_param_t;


#define X_MST_PAR(a, b, c, d, e, f) {c, d, e, f},

//  MAIN_BUF_Start_Table_Mask
#define USER_FUNC		(0x20)
#define USER_ARG		(0x10)
#define READ_R		    (0)
#define WRITE_R		    (0x01)	// 0 bit                        <--|
#define CB_WR		    (0x02)	// 1 bit                        <--|
#define LIM_SIGN		(0x04)	// 2 bit for limit              <--|
#define LIM_UNSIGN	    (0x08)  // 3 bit for limit	            <--|
#define LIM_MASK	    (0x0C)	// 2 and 3 bit for limit        <--|____________

#define LIM_BIT_MASK    LIM_MASK	
//	 Number		Name        Arg_1       Arg_2       Arg_3       Dev_CB_handler
//                          Address     Dev_ID      User
#define MST_BUF_TABLE\
	X_MST_PAR(0,	Dev_1,		0,          0,		    0,          0)\
	X_MST_PAR(1,	Dev_2,	    0,          0,		    0, 		    0)\
	X_MST_PAR(2,	Dev_3,	    0,	    	0,		    0, 		    0)\
	X_MST_PAR(3,	Dev_4,	    0,		    0,		    0, 		    0)\
	X_MST_PAR(4,	Dev_5,		0,	        0,          0,          0)\


#endif /* MST_MODBUS_CONF_H_INCLUDED */
