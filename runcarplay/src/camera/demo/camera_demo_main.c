#include <stdio.h>
#include <string.h>

#include "camera_demo.h"

/*
 * 最小可执行入口示例:
 *   ./camera_demo single
 *   ./camera_demo dual
 */
int demo_main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: %s single|dual\n", argv[0]);
        return -1;
    }

    if (strcmp(argv[1], "single") == 0) {
        return camera_demo_run_single();
    }
    if (strcmp(argv[1], "dual") == 0) {
        return camera_demo_run_dual();
    }

    printf("unknown mode: %s\n", argv[1]);
    printf("usage: %s single|dual\n", argv[0]);
    return -1;
}
