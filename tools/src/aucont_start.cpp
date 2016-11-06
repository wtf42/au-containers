#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "aucont.h"
#include "common.h"

int main(int argc, char** argv) {
    bool daemonize = false;
    int cpu = 100;
    unsigned net = 0;
    char* image;
    char** cmd;

    int idx = 1;
    if (idx < argc && !strcmp(argv[idx], "-d")) {
        daemonize = true;
        ++idx;
    }

    if (idx < argc && !strcmp(argv[idx], "--cpu")) {
        if (idx + 1 >= argc)
            invalid_arg(idx);
        cpu = atoi(argv[idx + 1]);
        if (cpu < 0 || cpu > 100)
            invalid_arg(idx + 1);
        idx += 2;
    }

    if (idx < argc && !strcmp(argv[idx], "--net")) {
        if (idx + 1 >= argc)
            invalid_arg(idx);
        if (inet_pton(AF_INET, argv[idx + 1], &net) != 1)
            invalid_arg(idx + 1);
        idx += 2;
    }

    if (idx >= argc)
        invalid_arg(idx);
    image = argv[idx];
    ++idx;

    if (idx >= argc)
        invalid_arg(idx);
    cmd = argv + idx;

    start_container({daemonize, cpu, net, image, cmd});

    return 0;
}
