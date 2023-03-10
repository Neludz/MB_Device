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
#include "user_lib/mb_dev.h"

mb_config_data_t *config_data;

int main(void)
{

  printf("start_modbus_mb\n");

  config_data = mb_config("config.txt");
  if (config_data == NULL)
  {
    printf("error create config data\n");
  }
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
