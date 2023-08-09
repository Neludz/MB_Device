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
#include "mst_hard.h"
//#include "mst_disp.h"

#define LOG_ERR(fmt, ...) \
    fprintf(stderr, "[ERROR] <%s:%d> : " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    fprintf(stdout, "[INFO] : " fmt "\n", __VA_ARGS__)

#define CONFIG_READ_FILE "modbus_config.cnf"
#define CONFIG_SAVE_FILE "new_modbus_config.cnf"



uint8_t test3[100];
uint8_t test2[sizeof(test3) / sizeof(uint8_t)];
// int get_block(uint8_t *buf, int timeout, int maxbytes, int id)
// {
//     int bytes, bytes_count = 0, read_count = 0;
//     int time_span = 0;
//     tcflush(id, TCIFLUSH);
//     tcdrain(id);
//     for (;;)
//     {
//         usleep(20000);
//         ioctl(id, FIONREAD, &bytes);
//         if ((bytes_count + bytes) >= maxbytes)
//         {
//             printf(" if (bytes_count >= maxbytes) = %d \n", bytes_count);
//             return 0;
//         }
//         if (bytes)
//         {
//             read_count = read(id, buf + bytes_count, bytes);
//             bytes_count += read_count;
//             if ((read_count == -1) || (read_count != bytes))
//             {
//                 printf("read_count != bytes");
//                 return 0;
//             }
//         }
//         time_span++;
//         if ((((timeout / 20) < time_span) && (!bytes_count)) ||
//             ((bytes_count) && (!bytes)))
//             return bytes_count;
//     }
//     printf("bytes read : %d\n", bytes);
//     return bytes_count;
// }

int main(void)
{


    
    Config *cfg = NULL;
    if (ConfigReadFile(CONFIG_READ_FILE, &cfg) != CONFIG_OK)
    {
        LOG_ERR("ConfigOpenFile failed for %s", CONFIG_READ_FILE);
        return 0;
    }
    mh_Slave_Init(cfg);

    ConfigFree(cfg);
    mst_master_init();
    // // ttyS0
    // int sfd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);

    // if (sfd == -1)
    // {
    //     printf("Error no is : %d\n", errno);

    //     return (-1);
    // };
    // // fprintf(stdout, "fprintf\n");

    // struct termios options;
    // tcgetattr(sfd, &options);
    // cfsetspeed(&options, 9600);
    // options.c_cflag &= ~CSTOPB;
    // options.c_cflag |= CLOCAL;
    // options.c_cflag |= CREAD;
    // options.c_cc[VTIME] = 0;
    // options.c_cc[VMIN] = 0;
    // cfmakeraw(&options);
    // tcsetattr(sfd, TCSANOW, &options);

    // char buf[] = {0x01, 0x03, 0x00, 0x01, 0x00, 0x28, 0x14, 0x14};
    // uint8_t buf_read[120];

    // for (size_t i = 0; i < 4; i++)
    // {
    //     int ret = write(sfd, buf, 8);
    //     int count = get_block(buf_read, 1000, 100, sfd);
    //     printf("data count = %d: ", count);
    //     for (size_t j = 0; j < 8; j++)
    //     {
    //         printf(" %d", buf_read[j]); /* code */
    //     }
    //     printf("\r\n");
    // }
    // close(sfd);
    while (1)
    {
        ;
    }
}