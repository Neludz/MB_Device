/* hi.c */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "mb_dev.h"




int main(void)
{

  printf("hi_15\n");

  // config_data_t config_1;

  // write(fp,"west",4);

  // close(fp);


  
  mb_config("config.txt");
  while (1)
  {
     ; 
  }
}
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
