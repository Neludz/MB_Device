/* mb_dev.c */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "mb_dev.h"


void parse_config(char *buf)
{
  printf("%s", buf);
}


int mb_config( char *st_config)
{
  int line = 0;
  FILE *file;
  char buf[256];
  
  if ((file = fopen(st_config, "r")) == NULL)
  {
    printf("can't open config file");
    return 1;
  }

  while (fgets(buf, sizeof(buf), file) != NULL)
  {
    parse_config(buf);
    line++;
  }

  fclose(file);
  return (0);
}