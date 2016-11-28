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
#include <arpa/inet.h>

#include <iostream>
#include <fstream>
#include <string>

using std::string;
using std::to_string;

int read_int(int fd);
void write_int(int fd, int value);

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

constexpr int CPUCG_WAIT = 100;
void run_in_container(int pid, char** cmd) {
    if (kill(pid, 0))
        fatal_error("process " + to_string(pid) + " not found");

    int pipefd[2];
    if (pipe2(pipefd, O_CLOEXEC))
        fatal_error("pipe2");

    if (int exec_pid = fork()) {
        close(pipefd[0]);

        if (system((AUCONT_PREFIX"bin/scripts/cpu_cg_cont_setup.sh " +
                    to_string(pid) + " " +
                    to_string(exec_pid) + " " +
                    AUCONT_PREFIX).c_str()))
            fatal_error("cpu_setup: ");

        write_int(pipefd[1], CPUCG_WAIT);

        close(pipefd[1]);
        wait(NULL);
        exit(0);
    }
    close(pipefd[1]);

    if (read_int(pipefd[0]) != CPUCG_WAIT)
        fatal_error("CPUCG_WAIT");

    for (string ns : {"user", "pid", "ipc", "uts", "net", "mnt"}) {
        int fd = open(("/proc/" + to_string(pid) + "/ns/" + ns).c_str(), O_RDONLY);
        if (setns(fd, 0))
            fatal_error("setns " + ns);
        close(fd);
    }

    close(pipefd[0]);

    if (fork()) {
        wait(NULL);
        exit(0);
    }

    execv(cmd[0], cmd);
}


int read_int(int fd) {
    int ret;
    if (read(fd, &ret, sizeof(ret)) != sizeof(ret))
        error("read int");
    return ret;
}

void write_int(int fd, int value) {
    if (write(fd, &value, sizeof(value)) != sizeof(value))
        error("write int");
}

string net2string(unsigned net) {
    char buf[sizeof("000.000.000.000")];
    inet_ntop(AF_INET, &net, buf, sizeof(buf));
    return string(buf);
}

void setup_host_network(int pid) {
    string net_cont = net2string(options.net);
    string net_host = net2string(options.net + (1 << 24));
    if (system((AUCONT_PREFIX"bin/scripts/net_host_setup.sh " +
                to_string(pid) + " " +
                net_cont + " " +
                net_host).c_str()))
        fatal_error("net_host_setup: ");
}

void setup_container_network(int pid) {
    string net_cont = net2string(options.net);
    string net_host = net2string(options.net + (1 << 24));
    if (system((AUCONT_PREFIX"bin/scripts/net_cont_setup.sh " +
                to_string(pid) + " " +
                net_cont + " " +
                net_host).c_str()))
        fatal_error("net_cont_setup: ");
}

void setup_cpu_cg(int pid) {
    if (system((AUCONT_PREFIX"bin/scripts/cpu_cg_setup.sh " +
                to_string(pid) + " " +
                to_string(options.cpu) + " " +
                AUCONT_PREFIX).c_str()))
        fatal_error("cpu_setup: ");
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

int pipe_fd1[2];
int pipe_fd2[2];

constexpr int SETUP_WAIT = 101;
constexpr int SYNC_WAIT = 102;

int setup_inside_container(void*) {
    close(pipe_fd1[0]);
    close(pipe_fd2[1]);
    int in_fd = pipe_fd2[0];
    int out_fd = pipe_fd1[1];

    if (options.daemonize) {
        daemonize();
    }

    if (unshare(CLONE_NEWPID))
        fatal_error("unshare");

    if (int pid = fork()) {
        write_int(out_fd, pid);
        close(in_fd);
        close(out_fd);

        wait(NULL);
        exit(0);
    }

    int cont_pid = read_int(in_fd);

    if (options.net)
        setup_container_network(cont_pid);
    setup_filesystem();
    setup_hostname();

    write_int(out_fd, SETUP_WAIT);

    if (read_int(in_fd) != SYNC_WAIT)
        fatal_error("SYNC_WAIT");

    close(in_fd);
    close(out_fd);

    return execv(options.cmd[0], options.cmd);
}

void start_container() {
    if (system("sudo echo > /dev/null"))
        fatal_error("sudo");
    if (pipe2(pipe_fd1, O_CLOEXEC) || pipe2(pipe_fd2, O_CLOEXEC))
        fatal_error("pipe2");

    int clone_flags = SIGCHLD
        | CLONE_NEWIPC
        | CLONE_NEWNS
        //| CLONE_NEWPID  //
        | CLONE_NEWUTS
        | CLONE_NEWUSER
        | CLONE_NEWNET;
    int cont_pid = clone(setup_inside_container, stack + STACK_SIZE, clone_flags, NULL);

    close(pipe_fd1[1]);
    close(pipe_fd2[0]);
    //0 - read, 1 - write
    int in_fd = pipe_fd1[0];
    int out_fd = pipe_fd2[1];

    cont_pid = read_int(in_fd);

    setup_userns(cont_pid);
    if (options.cpu != 100)
        setup_cpu_cg(cont_pid);
    if (options.net)
        setup_host_network(cont_pid);

    write_int(out_fd, cont_pid);

    if (read_int(in_fd) != SETUP_WAIT)
        fatal_error("SETUP_WAIT");

    setup_devices();

    write_int(out_fd, SYNC_WAIT);

    close(in_fd);
    close(out_fd);

    add_container(cont_pid);
    std::cout << cont_pid << std::endl;

    if (!options.daemonize) {
        wait(NULL);
        delete_container(cont_pid);
    }
}
