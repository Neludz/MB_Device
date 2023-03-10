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

//-------------- from X macros --------------
const char *str_with_type[NUM_REG_TYPE] =
    {
#define X_IO(a, b) b,
        IO_TABLE
#undef X_IO
};
//-------------------------------------------

mb_config_data_t *parse_config(FILE *file)
{
    char buf[256];
    char *token, *p_val;
    char varname[100];
    char value[100];
    uint32_t start_reg = 0;
    uint32_t reg_count = 0;
    uint32_t i, j;
    // check register count
    while (fgets(buf, sizeof(buf), file) != NULL)
    {
        token = strtok(buf, " \t\n");

        if ((token == NULL))
        {
            continue;
        }
        if (*token == '#')
        {
            continue;
        }
        if (start_reg == 1)
        {
            reg_count++;
            continue;
        }
        if (strcmp(token, "modbus_register:") == 0)
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
    config_ptr->reg_count = reg_count;
    config_ptr->uart_port[0] = 0;
    // parse config
    start_reg = 0;
    fseek(file, 0, SEEK_SET);
    while (fgets(buf, sizeof(buf), file) != NULL)
    {

        if ((token = strtok(buf, " \t\n")))
        {
            if (*token == '#')
                continue;

            if (strcmp(token, "modbus_register:") == 0)
            {
                start_reg = 1;
                reg_count = 0;
                continue;
            }

            if (!start_reg)
            {
                p_val = strtok(NULL, " \t\n");

                if (strcmp(token, "uart") == 0)
                {
                    if (strlen(p_val) < sizeof(config_ptr->uart_port))
                        strcpy(config_ptr->uart_port, p_val);
                    printf("uart port detected =  %s\n", config_ptr->uart_port);
                    continue;
                }
                if (strcmp(token, "baudrate") == 0)
                {
                    config_ptr->baudrate = (atoi(p_val));
                    printf("uart baudrate detected =  %d\n", config_ptr->baudrate);
                    continue;
                }
                if (strcmp(token, "parity") == 0)
                {
                    if ((strcmp(p_val, "odd") == 0))
                        config_ptr->parity = MB_ODD;
                    else if ((strcmp(p_val, "even") == 0))
                        config_ptr->parity = MB_EVEN;
                    else
                        config_ptr->parity = MB_NONE;
                    printf("uart parity detected =  %d\n", config_ptr->parity);
                    continue;
                }
                if (strcmp(token, "vtime") == 0)
                {
                    config_ptr->vtime = (atoi(p_val));
                    printf("uart vtime detected =  %d\n", config_ptr->vtime);
                    continue;
                }
                if (strcmp(token, "vmin") == 0)
                {
                    config_ptr->vmin = (atoi(p_val));
                    printf("uart vmin detected =  %d\n", config_ptr->vmin);
                    continue;
                }
                if (strcmp(token, "max_request_span") == 0)
                {
                    config_ptr->max_request_span = (atoi(p_val));
                    printf("uart max_request_span detected =  %d\n", config_ptr->max_request_span);
                    continue;
                }
                continue;
            }
            else
            {
                for (i = 0; i < LAST_REG_DATA_IN_STR; i++)
                {
                    if (reg_count < config_ptr->reg_count)
                    {
                        switch (i)
                        {
                        case ADDRESS_IN_STR:
                            config_ptr->p_reg[reg_count].addr = (atoi(token));
                            break;
                        case REG_IN_STR:
                            config_ptr->p_reg[reg_count].reg = (atoi(token));
                            break;
                        case TYPE_IN_STR:
                            for (j = 0; j < NUM_REG_TYPE; j++)
                            {
                                if (strcmp(token, str_with_type[j]) == 0)
                                {
                                    config_ptr->p_reg[reg_count].type = j;
                                    break;
                                }
                            }
                            break;
                        case FUNC_IN_STR:
                            config_ptr->p_reg[reg_count].func = (atoi(token));
                            break;
                        case OPTION_IN_STR:
                            config_ptr->p_reg[reg_count].user_option = (atoi(token));
                            break;
                        default:
                            break;
                        }
                        token = strtok(NULL, " \t\n");
                    }
                    else
                        break;
                }
                reg_count++;
                continue;
            }
        }
    }
    return config_ptr;
}

static void create_span(mb_config_data_t data)
{
    
}

mb_config_data_t *mb_config(char *st_config)
{
    mb_config_data_t *data;
    int line = 0;
    FILE *file;

    printf("start open config file: %s\n", st_config);
    file = fopen(st_config, "r");
    if (file == NULL)
    {
        printf("can't open config file\n");
        return NULL;
    }
    data = parse_config(file);
    fclose(file);
    return data;
}