
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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>
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

// tcp example
MST_INSTANCE_DEF(mst_1, master_main_cb, MST_BUF_TABLE);
int socket_desc;

// rs485 example
int sfd;
MST_INSTANCE_DEF(mst_2, master_rs485_cb, MST_RS_485_1_BUF_TABLE);

// queue msg
// msg_mst_t msg_queue;
int msg_queue_ID;
struct msqid_ds queue_spec;
// //-----------------------------------------------------------------------
// // task
// //-----------------------------------------------------------------------

void *mst_tcp_thread(void *ptr)
{
    mst_t *p_mst = (void *)ptr;
    msg_mst_t message;
    while (1)
    {
        mst_modbus_iteration((mst_t *)ptr);
        usleep(1);
        if (msgrcv(msg_queue_ID, &message, sizeof(message.data),
                   (long)p_mst->user_data, IPC_NOWAIT) > 0)
        {
            mst_set_event(p_mst, message.data, EVENT_REQUEST);
            printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
        }
    }
}

// //-----------------------------------------------------------------------
// // init
// //-----------------------------------------------------------------------
void mst_master_init()
{
    pthread_t mst_thread_id;
    struct mq_attr attr;
#ifdef USER_DEBUG
    printf("\r\n************ start_modbus_master ************\r\n\r\n");
    printf("mst_1 size is : %d\n", mst_1.max_device);
#endif
    // create ethernet socket
    mst_1.socket_data = (void *)&socket_desc;
    mst_1.user_data = &mst_1;
    mst_2.user_data = &mst_2;
    // create queue for recieve event
    if ((msg_queue_ID = msgget(IPC_PRIVATE, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))) < 0)
    {
        perror("CLIENT: msgget");
    }
    if (msgctl(msg_queue_ID, IPC_STAT, &queue_spec) < 0) //(msg_queue_ID = msgget( IPC_PRIVATE  , (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))) < 0)
    {
        perror("CLIENT: msgget");
    }
    queue_spec.msg_qbytes = sizeof(msg_mst_t) * 2;
    if (msgctl(msg_queue_ID, IPC_SET, &queue_spec) < 0) // mq_getattr(msg_queue_ID, &attr)<0)
    {
        printf("\r\n************ EEERRRRRRROOORRR************\r\n\r\n");
    }
    // thread for dispatcher
    if (pthread_create(&mst_thread_id, NULL, mst_tcp_thread, (void *)&mst_1) < 0)
    {
        perror("MB_MASTER_Thread not create\n");
    }
    if (pthread_create(&mst_thread_id, NULL, mst_tcp_thread, (void *)&mst_2) < 0)
    {
        perror("MB_MASTER_Thread not create\n");
    }
}
// //-----------------------------------------------------------------------
// // main callback for finite-state machine
// //-----------------------------------------------------------------------
mst_ret_t master_main_cb(mst_t *mst_data)
{
    mst_ret_t ret = RET_OK;
    struct sockaddr_in server; /* server address */
    struct timeval tv;
    int read_size;
    switch (mst_check_state(mst_data))
    {
        //*********************************************************
    case MST_INIT:
        // init code
        break;
        //*********************************************************
    case MST_PREPARE_CONNECT:
        // connect to device/create socket
        // create socket
        // usleep(50000);
        *(int *)mst_data->socket_data = socket(AF_INET, SOCK_STREAM, 0);
        if (*(int *)mst_data->socket_data < 0)
        {
            perror("[MB_MASTER_ERROR]: Socket()");
            return RET_ERROR;
        }
        else
        {
        }
        // fill server data
        server.sin_family = AF_INET;
        server.sin_port = htons(mst_data->dev_params[mst_data->device_number].port);
        server.sin_addr.s_addr = inet_addr(mst_data->dev_params[mst_data->device_number].str_ip);
        tv.tv_sec = 0;
        tv.tv_usec = 250000;
        setsockopt(*(int *)mst_data->socket_data, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
        tv.tv_sec = 2;
        tv.tv_usec = 500000;
        setsockopt(*(int *)mst_data->socket_data, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));

        // connect to server
        while ((connect(*(int *)mst_data->socket_data, (struct sockaddr *)&server, sizeof(server)) < 0))
        {
            perror("[MB_MASTER_ERROR]: Connect()");
            return RET_ERROR;
        }

        break;
        //*********************************************************
    case MST_DISCONNECT:
        // disconnect/delete socket

        close(*(int *)mst_data->socket_data);
        break;
        //*********************************************************
    case MST_SEND_REQ:
        // send request

        if (send(*(int *)mst_data->socket_data,
                 mst_data->rx_buf,
                 mst_data->len,
                 0) < 0)
        {
            perror("[MB_MASTER_ERROR]: Send()");
            return RET_ERROR;
        }
        else
        {
            read_size = recv(*(int *)mst_data->socket_data,
                             mst_data->tx_buf,
                             MST_FRAME_MAX, 0);
            if (read_size > 0)
            {
                mst_data->len = read_size;
                return RET_OK;
            }
            else
            {
                return RET_ERROR;
            }
        }
        break;
        //*********************************************************
    default:
        break;
    }
    return ret;
}

// //-----------------------------------------------------------------------
// // get data from uart
// //-----------------------------------------------------------------------
int get_block(uint8_t *buf, int timeout, int maxbytes, int id)
{
    int bytes, bytes_count = 0, read_count = 0;
    int time_span = 0;
    tcflush(id, TCIFLUSH);
    tcdrain(id);
    for (;;)
    {
        usleep(20000);
        ioctl(id, FIONREAD, &bytes);
        if ((bytes_count + bytes) >= maxbytes)
        {
#ifdef USER_DEBUG
            printf("[MB_MASTER_UART_ERROR]: (bytes_count >= maxbytes) = %d \n", bytes_count);
#endif
            return 0;
        }
        if (bytes)
        {
            read_count = read(id, buf + bytes_count, bytes);
            bytes_count += read_count;
            if ((read_count == -1) || (read_count != bytes))
            {
#ifdef USER_DEBUG
                printf("[MB_MASTER_UART_ERROR]: read_count != bytes");
#endif
                return 0;
            }
        }
        time_span++;
        if ((((timeout / 20) < time_span) && (!bytes_count)) ||
            ((bytes_count) && (!bytes)))
            return bytes_count;
    }
#ifdef USER_DEBUG
    printf("[MB_MASTER_UART]:bytes read : %d\n", bytes);
#endif
    return bytes_count;
}
// //-----------------------------------------------------------------------
// // main callback for finite-state machine
// //-----------------------------------------------------------------------
mst_ret_t master_rs485_cb(mst_t *mst_data)
{
    mst_ret_t ret = RET_OK;
    int read_size, count;
    switch (mst_check_state(mst_data))
    {
        //*********************************************************
    case MST_INIT:
// init code
#ifdef USER_DEBUG
        printf("[MB_MASTER_UART]: MST_INIT\n");
#endif
        // // ttyS0
        sfd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);

        if (sfd == -1)
        {
#ifdef USER_DEBUG
            printf("[MB_MASTER_UART_ERROR]: Error no is : %d\n", errno);
#endif
            return (-1);
        };

        struct termios options;
        tcgetattr(sfd, &options);
        cfsetspeed(&options, 19200);
        options.c_cflag &= ~CSTOPB;
        options.c_cflag |= CLOCAL;
        options.c_cflag |= CREAD;
        options.c_cc[VTIME] = 0;
        options.c_cc[VMIN] = 0;
        cfmakeraw(&options);
        tcsetattr(sfd, TCSANOW, &options);

        break;
        //*********************************************************
    case MST_PREPARE_CONNECT:

        // connect to device/create socket
        break;
        //*********************************************************
    case MST_DISCONNECT:
        // disconnect/delete socket
        break;
        //*********************************************************
    case MST_SEND_REQ:
        // send request
        usleep(50000);
        write(sfd, mst_data->tx_buf, mst_data->len);
        mst_data->len = get_block(mst_data->rx_buf, 250, MST_FRAME_MAX, sfd);
#ifdef USER_DEBUG
        printf("[MB_MASTER_UART]: MST_RX/TX\n");
#endif
        break;
        //*********************************************************
    default:
        break;
    }
    return ret;
}

// //-----------------------------------------------------------------------
// // write function
// //----------------------------------------------------------------------
void write_rs(uint16_t reg_id, uint16_t data)
{
    msg_mst_t message;
    message.mtype = (long)&mst_2;
    message.data = RS_5;
    if (msgsnd(msg_queue_ID, &message,
               sizeof(message.data), IPC_NOWAIT) < 0) // IPC_NOWAIT
    {
        printf("=============================================================================\n\n\n");
    }
    // mst_set_event(&mst_2, RS_5, EVENT_REQUEST);
}