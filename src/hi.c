/* hi.c */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include "user_lib/mb_dev.h"
#include "user_lib/configini.h"
#include "modbus_hard.h"

#define LOG_ERR(fmt, ...)	\
	fprintf(stderr, "[ERROR] <%s:%d> : " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#define LOG_INFO(fmt, ...)	\
	fprintf(stdout, "[INFO] : " fmt "\n", __VA_ARGS__)


#define CONFIG_READ_FILE		"modbus_config.cnf"
#define CONFIG_SAVE_FILE		"new_modbus_config.cnf"



#define MB_PORT 5000

mb_config_data_t *config_data;

extern void (*test_pr[NUM_REG_TYPE])(void);
extern void (*test_pr1[NUM_REG_TYPE])(void);

int get_block(uint8_t *buf, int timeout, int maxbytes, int id)
{
    int bytes, bytes_count = 0, read_count = 0;
    int time_span = 0;
    tcflush(id, TCIFLUSH);
    tcdrain(id);
   //ioctl(id, FIONREAD, &bytes);
    //LOG_INFO("%s","test!!!!!!!!!!!");
    for (;;)
    {
        usleep(20000);
        ioctl(id, FIONREAD, &bytes);
       // bytes_count += bytes;
        if ((bytes_count + bytes) >= maxbytes)
        {
            printf(" if (bytes_count >= maxbytes) = %d \n", bytes_count);
            return 0;
        }

        if (bytes)
        {
            read_count = read(id, buf+bytes_count, bytes);
            bytes_count += read_count;
            //printf("read_count %d \n", read_count);
            //printf("buf[0]: %d\n", buf[0]);
            if ((read_count == -1) || (read_count != bytes))
            {
                printf("read_count != bytes");
                return 0;
            }
        }
        time_span++;
         // printf("time_span %d \n", time_span);
        if ((((timeout / 20) < time_span) && (!bytes_count)) ||
            ((bytes_count) && (!bytes)))
            return bytes_count;
    }
    printf("bytes read : %d\n", bytes);

    return bytes_count;
}

/*
 * This will handle connection for each client
 * */
void *connection_thread_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
     
    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));
     
    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //end of string marker
		client_message[read_size] = '\0';
		
		//Send the message back to client
        write(sock , client_message , strlen(client_message));
        printf("read len: %d\n", read_size);
		printf("read data: %s\n", client_message);
		//clear the message buffer
		memset(client_message, 0, 2000);
    }
     
    if(read_size == 0)
    {
        printf("Client disconnected\n");
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
    // Client closed socket so clean up
    close(sock);
    return 0;
} 

void *mbslave_thread(void *ptr)
{
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
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(MB_PORT);
    // Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        // print the error message
        perror("bind failed. Error");
        return 0;
    }
    // Listen
    listen(socket_desc, 3);
    // Accept and incoming connection
    printf("Waiting for incoming connections...\n");
    cli_len = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&cli_len)))
    {
        printf("Connection accepted\n");

        if (pthread_create(&thread_id, NULL, connection_thread_handler, (void *)&client_sock) < 0)
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

int main(void)
{
    printf("\r\n************ start_modbus_mb ************\r\n\r\n");

Config *cfg = NULL;
if (ConfigReadFile(CONFIG_READ_FILE, &cfg) != CONFIG_OK) {
 		LOG_ERR("ConfigOpenFile failed for %s", CONFIG_READ_FILE);
		return 0;
	}
mh_Slave_Init(cfg);
ConfigFree(cfg);

    // pthread_t mbslave_thread_id;

    // if (pthread_create(&mbslave_thread_id, NULL, mbslave_thread, NULL) < 0)
    // {
    //     perror("MBSlave_Thread not creatr");
    // }
//ttyS0
    int sfd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);

    if (sfd == -1)
    {
        printf("Error no is : %d\n", errno);

        return (-1);
    };
   // fprintf(stdout, "fprintf\n");

    struct termios options;
    tcgetattr(sfd, &options);
    cfsetspeed(&options, 9600);
    //cfmakeraw(&options);
    options.c_cflag &= ~CSTOPB;
    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 0;
    cfmakeraw(&options);
    tcsetattr(sfd, TCSANOW, &options);

    // test_pr[0]();
    // test_pr1[0]();
    // config_data = mb_config("config.txt");

    char buf[] = {0x01, 0x03, 0x00, 0x01, 0x00, 0x28, 0x14, 0x14};
    uint8_t buf_read[120];

    for (size_t i = 0; i < 4; i++)
    {
       //tcflush(sfd, TCIOFLUSH );
       // usleep(50000);
        /* code */
      //  tcflush(sfd, TCIOFLUSH);
      //  tcflush(sfd, TCIFLUSH);
        write(sfd, buf, 8);

        int count = get_block(buf_read, 1000, 100, sfd);

        printf("data count = %d: ", count);
        for (size_t j = 0; j < 8; j++)
        {
            printf(" %d", buf_read[j]); /* code */
        }
        printf("\r\n");
    }
    close(sfd);
    while (1)
    {
      ;
    }
}