#include <stdlib.h>
#include <signal.h>

#include "aucont.h"
#include "common.h"

int main(int argc, char** argv) {
    int pid;
    int sig = SIGTERM;  // 15

    if (argc < 2)
        invalid_arg(1);
    pid = atoi(argv[1]);

    if (argc >= 3)
        sig = atoi(argv[2]);

    kill_container(pid, sig);

    return 0;
}
