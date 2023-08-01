
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

#include <mst_hard.h>
#include <mst_disp.h>
#include "mst_modbus_config.h"
#include "mst_modbus_cb.h"

MST_INSTANCE_DEF(mst_1, master_main_cb, MST_BUF_TABLE);

// //-----------------------------------------------------------------------
// // task
// //-----------------------------------------------------------------------

void *mst_tcp_thread(void *ptr)
{
    while (1)
    {
        mst_modbus_iteration((mst_t *)ptr);
        usleep(1000);
    }
}

// //-----------------------------------------------------------------------
// // init
// //-----------------------------------------------------------------------
void mst_master_init()
{
    pthread_t mst_thread_id;
#ifdef USER_DEBUG
    printf("\r\n************ start_modbus_mb ************\r\n\r\n");
    printf("test size is : %d\n", mst_1.max_device);
#endif
    // thread for dispatcher
    if (pthread_create(&mst_thread_id, NULL, mst_tcp_thread, (void *)&mst_1) < 0)
    {
        perror("MBSlave_Thread not creatr");
    }
}
// //-----------------------------------------------------------------------
// // main callback for finite-state machine
// //-----------------------------------------------------------------------
mst_ret_t master_main_cb(mst_t *mst_data)
{
    mst_ret_t ret = RET_OK;
    switch (mst_check_state(mst_data))
    {
    case MST_INIT:
        // init code

        break;
    case MST_PREPARE_CONNECT:
        // connect to device/create socket

        break;
    case MST_DISCONNECT:
        // disconnect/delete socket
        break;
    case MST_SEND_REQ:
        // send request

        break;
    default:
        break;
    }
    return ret;
}