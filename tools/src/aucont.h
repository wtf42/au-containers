#pragma once

struct start_options {
    bool daemonize;
    int cpu;
    unsigned net;
    char* image;
    char** cmd;
};
void start_container(const start_options& options);
void kill_container(int pid, int sig);
void run_in_container(int pid, char** cmd);
