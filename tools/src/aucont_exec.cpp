#include <stdlib.h>

#include "aucont.h"
#include "common.h"

int main(int argc, char** argv) {
    int pid;
    char** cmd;

    if (argc < 3)
        invalid_arg(2);
    pid = atoi(argv[1]);
    cmd = argv + 2;

    run_in_container(pid, cmd);

    return 0;
}
