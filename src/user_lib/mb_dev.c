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
#include "mb_dev_config.h"
//-------------- from X macros --------------
const char *str_with_type[NUM_REG_TYPE] =
{
#define X_IO(a, b, c, d) b,
        IO_TABLE
#undef X_IO
};

const char**str_with_type1[NUM_REG_TYPE1] =
{
#define X_IO(a, b, c, d) b,
        IO_TABLE_1
#undef X_IO
};

const uint32_t test_val[NUM_REG_TYPE] =
{
#define X_IO(a, b, c, d) d,
        IO_TABLE
#undef X_IO
};


const uint32_t test_val_1[NUM_REG_TYPE1] =
{
#define X_IO(a, b, c, d) d,
        IO_TABLE_1
#undef X_IO
};



void test_print()
{

    printf("first1_1: %d, second: %d \n", test_val[0], test_val[1]);
    printf("test1111 uhuhhu %s\n", *str_with_type1[0]);
}

void test_print1()
{

    printf("first: %d, second: %d \n", test_val_1[0], test_val_1[1]);
    printf("testttt uhuhhu \n");
}

//-------------------------------------------

mb_config_data_t *parse_config(FILE *file)
{
    char buf[256];
    char *token, *p_val;
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
                if (strcmp(token, "max_space_span") == 0)
                {
                    config_ptr->max_space_span = (atoi(p_val));
                    printf("uart max_space_span detected =  %d\n", config_ptr->max_space_span);
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

static void mh_SwapReg(mb_reg_t *xp, mb_reg_t *yp)
{
    mb_reg_t temp = *xp;
    *xp = *yp;
    *yp = temp;
}

static void mh_SpanSortData(mb_config_data_t *data)
{
    int i, j, min_idx;

    // One by one move boundary of unsorted subarray
    for (i = 0; i < (data->reg_count - 1); i++)
    {
        // Find the minimum element in unsorted array
        min_idx = i;
        for (j = i + 1; j < data->reg_count; j++)
            if ((data->p_reg[j].addr < data->p_reg[min_idx].addr) || // condition 1
                ((data->p_reg[j].func < data->p_reg[min_idx].func) && // condition 2
                (data->p_reg[j].addr == data->p_reg[min_idx].addr)) ||
                ((data->p_reg[j].reg < data->p_reg[min_idx].reg) && // condition 3
                 (data->p_reg[j].addr == data->p_reg[min_idx].addr)&&
                 (data->p_reg[j].func == data->p_reg[min_idx].func)))
                min_idx = j;
        // Swap the found minimum element
        // with the first element
        mh_SwapReg(&(data->p_reg[min_idx]), &(data->p_reg[i]));
    }
}

static uint32_t mh_CheckRegCount(uint8_t reg_type)
{
    if (reg_type >= REG_INT32_ABCD)
    {
        return 2; // add 2 reg
    }
    return 1; // add 1 reg
}

static void mh_CreateReadRequest(mb_config_data_t *data)
{
    uint32_t span_count, i, j, delta_item, delta_max, current_reg;
    mb_request_span_t *span_req = NULL;
    mb_request_span_t span_item;
    //two cycles:first for num span, then fill data span
    for (j = 0; j < 2; j++)
    {
        span_item.addr = data->p_reg[0].addr;
        span_item.start_reg = data->p_reg[0].reg;
        span_item.func = data->p_reg[0].func;
        span_item.err=0;
        current_reg = data->p_reg[0].reg;
        span_count=0;
        for (i = 0; i < (data->reg_count); i++)
        {
            delta_item = data->p_reg[i].reg - current_reg;
            delta_max = data->p_reg[i].reg - span_item.start_reg;
            if (delta_max > data->max_request_span ||
                delta_item > data->max_space_span ||
                span_item.addr != data->p_reg[i].addr ||
                span_item.func != data->p_reg[i].func)
            {
                if (span_req != NULL)
                {
                    memcpy(&data->p_span[span_count], &span_item, sizeof(span_item));
                }
                span_count++;
                span_item.start_reg = data->p_reg[i].reg;
                span_item.func = data->p_reg[i].func;
                span_item.count_reg = mh_CheckRegCount(data->p_reg[i].type);
                span_item.err=0;
                current_reg = span_item.start_reg;
                span_item.addr = data->p_reg[i].addr;
            }
            else
            {
                current_reg = data->p_reg[i].reg;
                span_item.count_reg = delta_max + mh_CheckRegCount(data->p_reg[i].type);
            }
        }
        
        if (span_req != NULL)
        {
            memcpy(&data->p_span[span_count], &span_item, sizeof(span_item));          
        }
        else
        {
           span_count++;
            span_req = malloc(sizeof(mb_request_span_t) * span_count);
            if (span_req == NULL)
            {
                printf("malloc_error \n");
                data->p_span = NULL;
                return;
            }
            else
            {
                 data->request_count = span_count;
                data->p_span = span_req;
            }
        }
    }

    // data->p_span = span_req;
    // memcpy(&data->p_span[0], &span_item, sizeof(span_item));
    // span_req->err = span_count;
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
    mh_SpanSortData(data);
    mh_CreateReadRequest(data);
    fclose(file);
    return data;
}