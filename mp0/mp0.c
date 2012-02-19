#include "mp0.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>

int debugprintf(const char *format, ...)
{
    int retval = 0;
#ifndef DISABLE_DEBUGPRINTF
    va_list argptr;
    va_start(argptr, format);
    retval = vprintf(format, argptr);
    va_end(argptr);
#endif
    return retval;
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    multicast_init();

    printf("*** Process started, my ID is %d\n", my_id);

    int opt;
    int long_index;
    int crashAfterSecs = -1;

    struct option long_options[] = {
        {"crashAfterSecs", required_argument, 0, 1000},
        {0, 0, 0, 0},
    };
    while ((opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (opt) {
        case 0:
            if (long_options[long_index].flag != 0) {
                break;
            }
            printf("option %s ", long_options[long_index].name);
            if (optarg) {
              printf("with arg %s\n", optarg);
            }
            printf("\n");
            break;

        case 1000:
            crashAfterSecs = strtol(optarg, NULL, 10);
            break;

        default:
            exit(1);
        }
    }

    if (crashAfterSecs > 0) {
        sleep(crashAfterSecs);
        pid_t mypid = getpid();
        kill(mypid, 9);
    }
    else {
        while (1) {
            sleep(60);
        }
    }

    return 0;
}
