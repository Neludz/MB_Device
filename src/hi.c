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
// #include "mst_disp.h"

#define LOG_ERR(fmt, ...) \
    fprintf(stderr, "[ERROR] <%s:%d> : " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    fprintf(stdout, "[INFO] : " fmt "\n", __VA_ARGS__)

#define CONFIG_READ_FILE "/home/psina/gg/MB_Device/modbus_config.cnf"
#define CONFIG_SAVE_FILE "new_modbus_config.cnf"



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

    while (1)
    {
        pause();
    }
}