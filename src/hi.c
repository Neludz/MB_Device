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
#include "user_lib/mb_dev.h"

mb_config_data_t *config_data;

extern void (*test_pr[NUM_REG_TYPE])(void);
extern void (*test_pr1[NUM_REG_TYPE])(void);

int getBlock(uint8_t *buf, int Timeout, int maxbytes, int id)
{
    int bytes, bytes_count = 0, read_count = 0;
    int time_span = 0;
    tcflush(id, TCIFLUSH);
    tcdrain(id);
    ioctl(id, FIONREAD, &bytes);

    // struct timespec pause;
    // pause.tv_sec = 0;
    // pause.tv_nsec = Toutms * 1000;
    for (;;)
    {
        usleep(5000);
        ioctl(id, FIONREAD, &bytes);
        bytes_count += bytes;
        if (bytes_count >= maxbytes){
            printf(" if (bytes_count >= maxbytes) = %d \n", bytes_count);
            return 0;
        }
            
        if (bytes)
        {
            read_count = read(id, buf, bytes);
            if (read_count != bytes)
            {
            printf("read_count != bytes");
return 0;
            }               
        }
        time_span++;
        if ((((Timeout / 5) < time_span) && (!bytes_count)) ||
            ((bytes_count) && (!bytes)))
            return bytes_count;
    }

    // memset(buf, '\0', sizeof (buf));
    // if (bytes >= maxbytes)bytes=maxbytes;
    // int count = read(id, buf, maxbytes);
    printf("bytes read : %d\n", bytes);
    // buf[count+1] = 0;
    return bytes_count;
}

int main(void)
{

    int sfd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);

    if (sfd == -1)
    {
        printf("Error no is : %d\n", errno);

        return (-1);
    };
    struct termios options;
    tcgetattr(sfd, &options);
    cfsetspeed(&options, B115200);
    cfmakeraw(&options);
    options.c_cflag &= ~CSTOPB;
    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 0;
    tcsetattr(sfd, TCSANOW, &options);

    printf("start_modbus_mb\r\n");
    // test_pr[0]();
    // test_pr1[0]();
    //  config_data = mb_config("config.txt");

    int gg;

    char buf[] = {0x01, 0x03, 0x00, 0x01, 0x00, 0x28, 0x14, 0x14};
    uint8_t buf_read[120];

    for (size_t i = 0; i < 100; i++)
    {
        /* code */
tcflush(sfd, TCIFLUSH);
        write(sfd, buf, 8);

        int count = getBlock(buf_read, 2000, 100, sfd);


    
        printf("data count = %d: ", count);
        for (size_t j = 0; j < 8; j++)
        {
            printf(" %d", buf_read[j]); /* code */
        }
        printf("\r\n");
    }
    close(sfd);

    // while (1)
    // {
    //     /* code */
    // }

    //   if (config_data == NULL)
    //   {
    //     printf("error create config data\n");
    //   }
}

// struct strstr
// {
//     char *str,
//          *one,
//          *two;
// };
// typedef struct strstr *STRSTR;

// // void split(STRSTR str)
// // {
// //     int i;
// //     char *temp = str->str;

// //     //while(isspace(*(str->str)))
// //     //    str->str++;

// //     str->one = strtok(str->str, " \t\n");
// // /*     for(i = 0; i < strlen(str->one); i++)
// //     {
// //         if(!isspace(str->one[i]))
// //             str->str++;
// //     }

// //     str->str++; */

// // /*         while(isspace(*(str->str)))
// //         str->str++;
// //  */

// //    // if(str->str != NULL)
// //    // {
// //      //   puts("In null if");
// //         str->two = strtok(NULL, " \t\n");
// //    //test1  }

// //     str->str = temp;
// // }

/*     STRSTR str = malloc(sizeof(struct strstr));
    str->str = malloc(256);

    fgets(str->str, 256, stdin);




    //split(str);
    printf("%s, %s\n", str->one, str->two);

    free(str->str);
    free(str); */
// config_data_t config_1;

// write(fp,"west",4);

// close(fp);

//   int sfd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
//   if (sfd == -1)
//   {
//     printf("Error no is : %d\n", errno);

//     //return (-1);
//   };
// return (0);
//   char buf[] = "hello---new";
//   char buf2[11];
//   int count = write(sfd, buf, 11);
//   count = read(sfd, buf2, 11);
//   buf2[11] = 0;
//   printf("%s", buf2);
//   close(sfd);
// }
