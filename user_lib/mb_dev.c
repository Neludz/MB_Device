/* mb_dev.c */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "mb_dev.h"

void rtrim(char *src)
{
    size_t i, len;
    volatile int isblank = 1;

    if (src == NULL)
        return;

    len = strlen(src);
    if (len == 0)
        return;
    for (i = len - 1; i > 0; i--)
    {
        isblank = isspace(src[i]);
        if (isblank)
            src[i] = 0;
        else
            break;
    }
    if (isspace(src[i]))
        src[i] = 0;
}

void ltrim(char *src)
{

    while (isspace(src))
        src++;

    /*     size_t i, len;

        if (src == NULL)
            return;

        i = 0;
        len = strlen(src);
        if (len == 0)
            return;
        while (src[i] && isspace(src[i]))
            i++;

        memmove(src, src + i, len - i + 1);
        return; */
}

void trim(char *src)
{
    rtrim(src);
    ltrim(src);
}

mb_config_data_t *parse_config(FILE *file)
{
    char buf[256];
    char *p;
    char varname[100];
    char value[100];
    uint32_t start_reg = 0;
    uint32_t reg_count = 0;
    const char *sep = "\n"; // get also rid of newlines
    char *token;

    // check register count
    while (fgets(buf, sizeof(buf), file) != NULL)
    {
        p=buf;
        while (isspace(p))
            p++;
        if (*p == '#' || *p == NULL)
            continue;

        if (start_reg == 1)
        {
            reg_count++;
            continue;
        }
        if (strcmp(p, "modbus_register:") == 0)
        {
            start_reg = 1;
        }
    }
    printf("count reg: %d \n", reg_count);

    if (reg_count > MAX_REG)
    {
        printf("so many registers \n");
        return NULL;
    }

    // create register & config data
    mb_config_data_t *config_ptr = malloc(sizeof(mb_config_data_t));
    mb_reg_t *register_ptr = malloc(reg_count * sizeof(mb_config_data_t));

    if (config_ptr == NULL || register_ptr == NULL)
    {
        printf("malloc_error \n");
        return NULL;
    }
    config_ptr->p_reg = register_ptr;

    // parse config
    while (fgets(buf, sizeof(buf), file) != NULL)
    {
         p=buf;
         

    }
}

int mb_config(char *st_config)
{
    int line = 0;
    FILE *file;
    char buf[256];
    int i = 0;
    printf(" strart open config file: %s\n", st_config);
    file = fopen(st_config, "r");
    if (file == NULL)
    {
        printf("can't open config file\n");
        return 1;
    }
    parse_config(file);
    // while (fgets(buf, sizeof(buf), file) != NULL)
    // {
    //   ifparse_config(buf);
    //   line++;
    // }

    fclose(file);
    //  return (0);
}