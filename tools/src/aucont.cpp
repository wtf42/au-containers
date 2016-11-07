#include "aucont.h"
#include "common.h"
#include "pid_storage.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

//#include <sys/sysmacros.h>  // makedev
//#include <sys/mount.h>  // mount
//#include <sys/syscall.h>
#include <iostream>
#include <fstream>
#include <string>

using std::string;
using std::to_string;

start_options options;
void start_container();
void start_container(const start_options& options) {
    ::options = options;
    start_container();
}

void kill_container(int pid, int sig) {
    if (!delete_container(pid))
        error("failed to delete container record");
    if (kill(pid, sig))
        error("failed to kill " + to_string(pid));
}

void run_in_container(int pid, char** cmd) {
    if (kill(pid, 0))
        fatal_error("process " + to_string(pid) + " not found");

    int pipefd[2];
    if (pipe2(pipefd, O_CLOEXEC))
        error("pipe2");

    if (fork()) {
        close(pipefd[0]);
        // TODO: setup cpu cgroup
        close(pipefd[1]);
        wait(NULL);
        return;
    }

    close(pipefd[1]);
    for (string ns : {"user", "pid", "ipc", "uts", "net", "mnt"}) {
        int fd = open(("/proc/" + to_string(pid) + "/ns/" + ns).c_str(), O_RDONLY);
        if (setns(fd, 0))
            fatal_error("setns " + ns + ":");
        close(fd);
    }
    close(pipefd[0]);

    execv(cmd[0], cmd);
}



constexpr int magic_value = 42;

int read_int(int fd) {
    int magic;
    if (read(fd, &magic, sizeof(magic)) != sizeof(magic) || magic != magic_value)
        error("read magic");
    int ret;
    if (read(fd, &ret, sizeof(ret)) != sizeof(ret))
        error("read int");
    return ret;
}

void write_int(int fd, int value) {
    if (write(fd, &magic_value, sizeof(magic_value)) != sizeof(magic_value))
        error("write magic");
    if (write(fd, &value, sizeof(value)) != sizeof(value))
        error("write int");
}

void setup_devices() {
    if (system((AUCONT_PREFIX"bin/scripts/dev_setup.sh " + string(options.image)).c_str()))
        fatal_error("dev_setup: ");
}

void setup_filesystem() {
    if (system((AUCONT_PREFIX"bin/scripts/fs_setup.sh " + string(options.image)).c_str()))
        fatal_error("fs_setup: ");
    if (chdir("/"))
        fatal_error("Can't change working directory");
}

const char HOSTNAME[] = "container";
void setup_hostname() {
    if (sethostname(HOSTNAME, strlen(HOSTNAME))) {
        fatal_error("setup_hostname");
    }
}

void setup_userns(int cont_pid) {
    string pid = to_string(cont_pid);
    string uid = to_string(geteuid());
    string gid = to_string(getegid());
    { std::ofstream("/proc/" + pid + "/uid_map") << "0 " + uid + " 1"; }
    { std::ofstream("/proc/" + pid + "/setgroups") << "deny"; }
    { std::ofstream("/proc/" + pid + "/gid_map") << "0 " + gid + " 1"; }
}

void daemonize() {
    pid_t sid = setsid();
    if (sid < 0)
        fatal_error("setsid");

    if (chdir("/") < 0)
        fatal_error("chdir");

    int fd = open("/dev/null", O_RDWR, 0);

    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);

        if (fd > 2)
            close(fd);
    }

    umask(027);
}

constexpr int STACK_SIZE = 4 * 1024 * 1024;
char stack[STACK_SIZE];

int in_pipe_fds[2];
int out_pipe_fds[2];

constexpr int USERNS_WAIT = 100;
constexpr int SETUP_WAIT = 101;
constexpr int SYNC_WAIT = 102;

int setup_inside_container(void*) {
    close(in_pipe_fds[0]);
    close(out_pipe_fds[1]);

    if (options.daemonize) {
        daemonize();
    }

    //if (unshare(CLONE_NEWPID))
    //    fatal_error("unshare");

    if (read_int(out_pipe_fds[0]) != USERNS_WAIT)
        fatal_error("USERNS_WAIT");

    setup_filesystem();
    setup_hostname();
    //TODO: setup_network();

    write_int(in_pipe_fds[1], SETUP_WAIT);

    if (read_int(out_pipe_fds[0]) != SYNC_WAIT)
        fatal_error("SYNC_WAIT");

    close(in_pipe_fds[1]);
    close(out_pipe_fds[0]);

    return execv(options.cmd[0], options.cmd);
}

void start_container() {
    if (pipe2(in_pipe_fds, O_CLOEXEC) || pipe2(out_pipe_fds, O_CLOEXEC))
        fatal_error("pipe2: ");

    int clone_flags = SIGCHLD
        | CLONE_NEWIPC
        | CLONE_NEWNS
        | CLONE_NEWPID  //
        | CLONE_NEWUTS
        | CLONE_NEWUSER
        | CLONE_NEWNET;
    int cont_pid = clone(setup_inside_container, stack + STACK_SIZE, clone_flags, NULL);

    close(in_pipe_fds[1]);
    close(out_pipe_fds[0]);
    //0 - read, 1 - write

    //if (options.daemonize)
    //    cont_pid = read_int(in_pipe_fds[0]);

    setup_userns(cont_pid);
    write_int(out_pipe_fds[1], USERNS_WAIT);

    if (read_int(in_pipe_fds[0]) != SETUP_WAIT)
        fatal_error("SETUP_WAIT");
    setup_devices();
    //TODO: setup cgroup
    //TODO: setup networking

    write_int(out_pipe_fds[1], SYNC_WAIT);

    close(in_pipe_fds[0]);
    close(out_pipe_fds[1]);

    init_containers();
    add_container(cont_pid);
    std::cout << cont_pid << std::endl;

    if (!options.daemonize) {
        //if (waitpid(cont_pid, NULL, __WCLONE)) fatal_error("waitpid");
        wait(NULL);
        delete_container(cont_pid);
    }
}
