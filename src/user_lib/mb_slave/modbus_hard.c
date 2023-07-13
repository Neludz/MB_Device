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
uint16_t MBbuf_main[MB_NUM_BUF];
extern const RegParameters_t MBRegParam[];

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
        pthread_mutex_lock(&mutexMBbuf_main);
        // pthread_mutex_lock(&data_tcp_inst->socket_mutex);
        if ((read_size < MB_FRAME_MAX) && (mb_instance_idle_check(&data_tcp_inst->MB_Data_Inst)))
        {
            memcpy(data_tcp_inst->Buf_Data, client_message, read_size);
            data_tcp_inst->MB_Data_Inst.mb_index = (uint8_t)read_size;
            data_tcp_inst->MB_Data_Inst.user_data = &sock;
            mb_parsing(&data_tcp_inst->MB_Data_Inst);
        }
        pthread_mutex_unlock(&mutexMBbuf_main);
        // pthread_mutex_unlock(&data_tcp_inst->socket_mutex);
        printf("read len: %d\n", read_size);
    }

    if (read_size == 0)
    {
        printf("Client disconnected\n");
    }
    else if (read_size == -1)
    {
        perror("recv failed");
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
        perror("Could not create socket");
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
        perror("bind failed. Error");
        return 0;
    }
    // Listen
    int ret = listen(socket_desc, 1);
    // Accept and incoming connection
    printf("Waiting for incoming connections on port %d...\n", data_tcp_inst->Port);
    cli_len = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&cli_len)))
    {
        inet_ntop(AF_INET, &client.sin_addr, buf, sizeof(buf));
        printf("Connection accepted, addr: %s\n", buf);
        data_tcp_inst->Client_Sock_Inst = client_sock;
        if (pthread_create(&thread_id, NULL, connection_tcp_thread_handler, (void *)data_tcp_inst) < 0)
        {
            perror("could not create thread");
            return 0;
        }

        // Now join the thread , so that we dont terminate before the thread
        // pthread_join( thread_id , NULL);
        printf("Handler assigned\n");
    }
    if (client_sock < 0)
    {
        perror("accept failed");
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
            printf("%s find section %s\n", __FUNCTION__, strFullSection);
            MB_tcp_thread_data = calloc(1, sizeof(mb_tcp_thread_data_t));
            if (MB_tcp_thread_data == NULL)
            {
                printf("Memory error in func: %s \n", __FUNCTION__);
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
                perror("MBSlave_Thread not creatr");
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
    printf("Callback: %s \n", __FUNCTION__);
    ret = write(*(int *)st_mb->user_data, st_mb->p_mb_buff, st_mb->response_size);
    st_mb->mb_state = MB_STATE_IDLE;
}

void mh_Callback_TCP(void *mbb)
{

    printf("Callback: %s \n", __FUNCTION__);
}

// // Callback for usb com
// void mh_Recieve(uint8_t *USB_buf, uint16_t len)	//interrupt	function
// {
//     if (mb_instance_idle_check((MBStruct_t*)&MB_USB)==MB_OK)
//     {
//         if(len>MB_FRAME_MAX)
//         {
//             len=MB_FRAME_MAX;
//         }
//         MB_USB.mb_state=MB_STATE_PARS;
//         MB_USB.mb_index=(len);
//         memcpy (MB_USB.p_mb_buff,USB_buf,len);
//         MBStruct_t *st_mb=(MBStruct_t*)&MB_USB;
//         xQueueSend(xModbusQueue, &st_mb, 0);
//     }
// }

// // Init modbus
// void mh_Modbus_Init(void)
// {
//     //create queue
//     xModbusQueue=xQueueCreate(3,sizeof(MBStruct_t *));

//     //create modbus task
//     if(pdTRUE != xTaskCreate(mh_task_Modbus, "RS485", MODBUS_TASK_STACK_SIZE, NULL, MODBUS_TASK_PRIORITY, &m_modbus_task_handle)) ERROR_ACTION(TASK_NOT_CREATE, 0);

//     mh_USB_Init();
//     mh_RS485_Init();
// }

// void mh_USB_Init(void)
// {
//     MB_USB.p_write = MBbuf_main;
//     MB_USB.p_read = MBbuf_main;
//     MB_USB.reg_read_last=MB_NUM_BUF-1;
//     MB_USB.reg_write_last=MB_NUM_BUF-1;
//     MB_USB.cb_state=MB_CB_FREE;
//     MB_USB.er_frame_bad=EV_NOEVENT;
//     MB_USB.slave_address=MB_ANY_ADDRESS;	//0==any address
//     MB_USB.mb_state=MB_STATE_IDLE;
//     MB_USB.p_mb_buff=&USB_MB_Buf[0];
//     MB_USB.wr_callback = mh_Write_Eeprom;
//     MB_USB.f_start_trans = mh_USB_Transmit_Start;
//     MB_USB.f_start_receive = NULL;
// }

// void mh_RS485_Init(void)
// {
//     uint32_t Rs485_Time_ms;

//     MB_RS485.p_write = MBbuf_main;
//     MB_RS485.p_read = MBbuf_main;
//     MB_RS485.reg_read_last=MB_NUM_BUF-1;
//     MB_RS485.reg_write_last=MB_NUM_BUF-1;
//     MB_RS485.cb_state=MB_CB_FREE;
//     MB_RS485.er_frame_bad=EV_NOEVENT;
//     MB_RS485.slave_address=MBbuf_main[Reg_RS485_Modbus_Addr];
//     MB_RS485.mb_state=MB_STATE_IDLE;
//     MB_RS485.p_mb_buff=&RS485_MB_Buf[0];
//     MB_RS485.wr_callback = mh_Write_Eeprom;
//     MB_RS485.f_start_trans=mh_Rs485_Transmit_Start;
//     MB_RS485.f_start_receive = NULL;

//     Rs485_Time_ms = (MBbuf_main[Reg_RS485_Ans_Delay]);
//     rs485_timer_handle = xTimerCreate( "T_RS485", Rs485_Time_ms/portTICK_RATE_MS, pdFALSE, NULL, rs485_timer_callback);
//     IO_Uart1_Init();
// }

// void rs485_timer_callback (xTimerHandle xTimer)
// {
//     if( MB_STATE_RCVE == MB_RS485.mb_state)
//     {
//         // If we are receiving, it's the end event: t3.5
//         MB_RS485.mb_state=MB_STATE_PARS;					// Begin parsing of a frame.
//         MBStruct_t *st_mb=(MBStruct_t*)&MB_RS485;
//         xQueueSend(xModbusQueue, &st_mb, 0);
//     }
// }

// void IO_Uart1_Init(void)
// {
//     RCC->APB2ENR	|= RCC_APB2ENR_USART1EN;						//USART1 Clock ON
//     USART1->BRR = Baud_rate[MBbuf_main[Reg_RS485_Baud_Rate]&0x3];	// Bodrate
//     USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_IDLEIE | USART_CR1_TCIE | USART_CR1_RXNEIE;

//     switch (MBbuf_main[Reg_Parity_Stop_Bits])
//     {
//     case NO_PARITY_1_STOP:
//         break; //default setting

//     case NO_PARITY_2_STOP:
//         USART1->CR2  |=  USART_CR2_STOP_1;
//         break;

//     case EVEN_PARITY_1_STOP:
//         USART1->CR1  |= USART_CR1_PCE | USART_CR1_M;
//         break;

//     case ODD_PARITY_1_STOP:
//         USART1->CR1  |= USART_CR1_PCE | USART_CR1_M | USART_CR1_PS;
//         break;

//     default:
//         break;
//     }

//     USART1->CR1 |= USART_CR1_UE;

//     NVIC_SetPriority(USART1_IRQn,14);
//     NVIC_EnableIRQ (USART1_IRQn);
// }

// void mh_Write_Eeprom (void *mbb)
// {
//     MBStruct_t *st_mb;
//     st_mb = (void*) mbb;
//     uint16_t len =  sizeof(MBRegParam[0].Default_Value);

//     for (int32_t i = 0; i < (st_mb->cb_index); i++)
//     {
//         if((mb_reg_option_check(i+(st_mb->cb_reg_start), CB_WR) == MB_OK))
//         {
//             AT25_mutex_update_byte( ((st_mb->cb_reg_start)+i)*len, (uint8_t*) &(st_mb->p_write[i+(st_mb->cb_reg_start)]), len);
//         }
//     }
// }

// void mh_USB_Transmit_Start (void *mbb)
// {
//     MBStruct_t *st_mb;
//     st_mb = (void*) mbb;
//     CDC_Transmit_FS (st_mb->p_mb_buff, st_mb->response_size);
//     MB_USB.mb_state=MB_STATE_IDLE;
// }

// void mh_Rs485_Transmit_Start (void *mbb)
// {
//     mh_EnableTransmission(true);
//     USART1->CR1 |= USART_CR1_TXEIE;
// }

// void mh_Factory (void)
// {
//     taskENTER_CRITICAL();
//     uint16_t len =  sizeof(MBRegParam[0].Default_Value);
//     for (int32_t i=0; i< MB_NUM_BUF; i++)
//     {
//         if (mb_reg_option_check(i, CB_WR)==MB_OK)
//         {
//             MBbuf_main[i] = MBRegParam[i].Default_Value;
//             AT25_update_byte((i*len), (uint8_t *) &MBbuf_main[i],  len);
//         }
//     }
//     taskEXIT_CRITICAL();
//     MBbuf_main[Reg_Set_Default_Reset]=0;
// }

// void mh_Buf_Init (void)
// {
//     taskENTER_CRITICAL();
//     AT25_Init();
//     uint16_t len =  sizeof(MBRegParam[0].Default_Value);
//     for (int32_t i=0; i< MB_NUM_BUF; i++)
//     {
//         if(mb_reg_option_check(i, CB_WR)==MB_OK)
//         {
//             AT25_read_byte((i*len), (uint8_t *) &MBbuf_main[i],  len);
//             if(mb_reg_limit_check(i, MBbuf_main[i])==MB_ERROR)
//             {
//                 MBbuf_main[i]=MBRegParam[i].Default_Value;
//                 AT25_update_byte((i)*len, (uint8_t *) &MBbuf_main[i],  len);
//             }
//         }
//     }
//     MBbuf_main[Reg_Set_Default_Reset]=0;
//     taskEXIT_CRITICAL();
// }
