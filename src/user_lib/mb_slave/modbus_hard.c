#include "modbus_hard.h"
#include "modbus.h"
#include "modbus_reg.h"
// #include "user_lib/configini.h"

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
#include "user_lib/configini.h"
//-----------------------------------------------------------------------
// Variable
//-----------------------------------------------------------------------

pthread_mutex_t mutexMBbuf_main = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexMBSlave_main = PTHREAD_MUTEX_INITIALIZER;
extern pthread_mutex_t mutex_rs_write ;
uint16_t MBbuf_main[MB_NUM_BUF];
//extern const RegParameters_t MBRegParam[];

// //-----------------------------------------------------------------------
// // Task function
// //-----------------------------------------------------------------------

// //-----------------------------------------------------------------------
// // Function
// //-----------------------------------------------------------------------
/*
 * This will handle connection for each client
 * */
void *connection_tcp_thread_handler(void *ptr)
{
    // Get the socket descriptor
    mb_tcp_thread_data_t *data_tcp_inst;
    data_tcp_inst = (mb_tcp_thread_data_t *)ptr;
    int sock = data_tcp_inst->Client_Sock_Inst;
    int read_size;
    char *message, client_message[2000];

    // Receive a message from client
    while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
    {
        pthread_mutex_lock(&mutexMBSlave_main);
        // pthread_mutex_lock(&data_tcp_inst->socket_mutex);
        if ((read_size < MB_FRAME_MAX) && (mb_instance_idle_check(&data_tcp_inst->MB_Data_Inst)))
        {
            memcpy(data_tcp_inst->Buf_Data, client_message, read_size);
            data_tcp_inst->MB_Data_Inst.mb_index = (uint8_t)read_size;
            data_tcp_inst->MB_Data_Inst.user_data = &sock;
            pthread_mutex_lock(&mutexMBbuf_main);
            mb_parsing(&data_tcp_inst->MB_Data_Inst);
            pthread_mutex_unlock(&mutexMBbuf_main);
        }
        pthread_mutex_unlock(&mutexMBSlave_main);
// pthread_mutex_unlock(&data_tcp_inst->socket_mutex);
#ifdef USER_DEBUG
        printf("[MB_SLAVE]: read len: %d\n", read_size);
#endif
    }

    if (read_size == 0)
    {
#ifdef USER_DEBUG
        printf("[MB_SLAVE]: client disconnected\n");
#endif
    }
    else if (read_size == -1)
    {
        perror("[MB_SLAVE_ERROR]: recv failed\n");
    }
    // Client closed socket so clean up
    close(sock);
    return 0;
}

void *mb_tcp_slave_thread(void *ptr)
{
    // int sock = *(int*)socket_desc;
    char buf[20];
    mb_tcp_thread_data_t *data_tcp_inst;
    data_tcp_inst = (mb_tcp_thread_data_t *)ptr;
    int socket_desc, client_sock, cli_len;
    struct sockaddr_in server, client;
    pthread_t thread_id;
    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        perror("[MB_SLAVE_ERROR]: Could not create socket\n");
        return 0;
    }
    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htons(INADDR_ANY);
    server.sin_port = htons(data_tcp_inst->Port);
    // Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        // print the error message
        perror("[MB_SLAVE_ERROR]: bind failed\n");
        return 0;
    }
    // Listen
    int ret = listen(socket_desc, 1);
// Accept and incoming connection
#ifdef USER_DEBUG
    printf("[MB_SLAVE]: Waiting for incoming connections on port %d...\n", data_tcp_inst->Port);
#endif
    cli_len = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&cli_len)))
    {
        inet_ntop(AF_INET, &client.sin_addr, buf, sizeof(buf));
#ifdef USER_DEBUG
        printf("[MB_SLAVE]: Connection accepted, addr: %s\n", buf);
#endif
        data_tcp_inst->Client_Sock_Inst = client_sock;
        if (pthread_create(&thread_id, NULL, connection_tcp_thread_handler, (void *)data_tcp_inst) < 0)
        {
            perror("[MB_SLAVE_ERROR]:could not create thread");
            return 0;
        }
// Now join the thread , so that we dont terminate before the thread
// pthread_join( thread_id , NULL);
#ifdef USER_DEBUG
        printf("[MB_SLAVE]: Handler assigned\n");
#endif
    }
    if (client_sock < 0)
    {
        perror("[MB_SLAVE_ERROR]:accept failed");
        close(client_sock);
        close(socket_desc);
        return 0;
    }
}

int32_t mh_Slave_Init(Config *cfg)
{
    uint32_t i, len, j, tcp_port;
    in_addr_t addr;
    char strFullSection[64];
    pthread_t MBslave_thread_id;
    mb_tcp_thread_data_t *MB_tcp_thread_data;

    // TCP check
    for (i = 1; i <= CONFIG_MAX_MBSLAVE_NUM; i++)
    {
        len = strlen(CONFIG_MB_TCP_SLAVE);
        memcpy(strFullSection, CONFIG_MB_TCP_SLAVE, len);
        snprintf(strFullSection + len, sizeof(strFullSection) - len, "%d", i);
        if (ConfigHasSection(cfg, strFullSection))
        {
#ifdef USER_DEBUG
            printf("[MB_SLAVE_CONFIG]:%s find section %s\n", __FUNCTION__, strFullSection);
#endif
            MB_tcp_thread_data = calloc(1, sizeof(mb_tcp_thread_data_t));
            if (MB_tcp_thread_data == NULL)
            {
#ifdef USER_DEBUG
                printf("[MB_SLAVE_CONFIG]:Memory error in func: %s\n", __FUNCTION__);
#endif
                return 0;
            }
            MB_tcp_thread_data->MB_Data_Inst.p_read = MBbuf_main;
            MB_tcp_thread_data->MB_Data_Inst.p_write = MBbuf_main;
            MB_tcp_thread_data->MB_Data_Inst.reg_read_last = MB_NUM_BUF - 1;
            MB_tcp_thread_data->MB_Data_Inst.reg_write_last = MB_NUM_BUF - 1;
            MB_tcp_thread_data->MB_Data_Inst.cb_state = MB_CB_FREE;
            MB_tcp_thread_data->MB_Data_Inst.er_frame_bad = EV_NOEVENT;
            MB_tcp_thread_data->MB_Data_Inst.mb_state = MB_STATE_IDLE;
            MB_tcp_thread_data->MB_Data_Inst.p_mb_buff = MB_tcp_thread_data->Buf_Data;
            ConfigReadInt(cfg, strFullSection, CONFIG_ADDR_ID, &j, DEFAULT_MB_ADDR);
            MB_tcp_thread_data->MB_Data_Inst.slave_address = (uint8_t)j;
            ConfigReadInt(cfg, strFullSection, CONFIG_TCP_FRAME_TYPE, &j, 0);
            if (j)
                MB_tcp_thread_data->MB_Data_Inst.mb_frame_type = MB_TYPE_TCP;
            else
                MB_tcp_thread_data->MB_Data_Inst.mb_frame_type = MB_TYPE_RTU;
            ConfigReadInt(cfg, strFullSection, CONFIG_TCP_PORT, &tcp_port, DEFAULT_PORT);
            MB_tcp_thread_data->Port = tcp_port;
            MB_tcp_thread_data->MB_Data_Inst.wr_callback = mh_Callback_TCP;
            MB_tcp_thread_data->MB_Data_Inst.f_start_trans = mh_TCP_Transmit_Start;
            MB_tcp_thread_data->MB_Data_Inst.f_start_receive = NULL;

            // pthread_mutex_init(&MB_tcp_thread_data->socket_mutex, NULL);

            if (pthread_create(&MBslave_thread_id, NULL, mb_tcp_slave_thread, (void *)MB_tcp_thread_data) < 0)
            {
                perror("[MB_SLAVE_ERROR]:MBSlave_Thread not create\n");
                return 0;
            }
        }
    }
}

void mh_TCP_Transmit_Start(void *mbb)
{
    ssize_t ret;
    MBStruct_t *st_mb;
    st_mb = (void *)mbb;
    ret = write(*(int *)st_mb->user_data, st_mb->p_mb_buff, st_mb->response_size);
    st_mb->mb_state = MB_STATE_IDLE;
}

void mh_Callback_TCP(void *mbb)
{
    MBStruct_t *st_mb;
    st_mb = (void *)mbb;
#ifdef USER_DEBUG
    printf("[MB_SLAVE_USER_CB]: Callback: %s \n", __FUNCTION__);
#endif
    void (*ccb)(uint16_t reg_id, uint16_t data);
    for (int32_t i = 0; i < (st_mb->cb_index); i++)
    {
        if ((mb_reg_option_check(i + (st_mb->cb_reg_start), USER_FUNC) == MB_OK))
        {

            ccb = mb_getRegUserArg1(i + (st_mb->cb_reg_start));  
            ccb(i + (st_mb->cb_reg_start), st_mb->p_write[i + (st_mb->cb_reg_start)]);


#ifdef USER_DEBUG
            printf("[write_rs]: Callback: %s ________________________________\n", __FUNCTION__);
#endif
        }
    }
}

uint16_t mh_get_reset_data_mb_buf(uint16_t number)
{
    uint16_t data;
    pthread_mutex_lock(&mutexMBbuf_main);
    data = MBbuf_main[number];
    MBbuf_main[number] = 0;
    pthread_mutex_unlock(&mutexMBbuf_main);
    return data;
}

uint16_t mh_get_data_mb_buf(uint16_t number)
{
    uint16_t data;
    pthread_mutex_lock(&mutexMBbuf_main);
    data = MBbuf_main[number];
    pthread_mutex_unlock(&mutexMBbuf_main);
    return data;
}
