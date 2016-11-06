#include "pid_storage.h"
#include "common.h"

#include <stdio.h>
#include <sys/file.h>
#include <signal.h>
#include <unistd.h>
#include <set>
#include <iostream>

const char CONTAINERS_FILE[] = AUCONT_PREFIX".containers";

std::set<int> read_pids(int fd) {
    std::set<int> result;
    FILE* file = fdopen(dup(fd), "r");

    int pid;
    while (fscanf(file, "%d", &pid) == 1) {
        if (!kill(pid, 0)) {
            result.insert(pid);
        }
    }

    fclose(file);
    return result;
}

void write_pids(int fd, const std::set<int>& pids) {
    if (ftruncate(fd, 0)) {};
    FILE* file = fdopen(dup(fd), "w");
    for (int pid : pids) {
        fprintf(file, "%d\n", pid);
    }
    fclose(file);
}

bool get_containers(std::set<int>& out_pids) {
    int fd = open(CONTAINERS_FILE, O_RDONLY);
    if (flock(fd, LOCK_EX)) {
        close(fd);
        return false;
    }

    out_pids = read_pids(fd);

    close(fd);
    return true;
}

bool delete_container(int pid) {
    int fd = open(CONTAINERS_FILE, O_RDWR);
    if (flock(fd, LOCK_EX)) {
        close(fd);
        return false;
    }

    std::set<int> pids = read_pids(fd);
    auto it = pids.find(pid);
    if (it == pids.end()) {
        close(fd);
        return false;
    }
    pids.erase(it);
    write_pids(fd, pids);

    close(fd);
    return true;
}

bool add_container(int pid) {
    int fd = open(CONTAINERS_FILE, O_RDWR);
    if (flock(fd, LOCK_EX)) {
        close(fd);
        return false;
    }

    std::set<int> pids = read_pids(fd);
    if (pids.find(pid) != pids.end()) {
        close(fd);
        return false;
    }
    pids.insert(pid);
    write_pids(fd, pids);

    close(fd);
    return true;
}

bool init_containers() {
    int fd = open(CONTAINERS_FILE, O_CREAT, 0700);
    if (fd < 0) {
        return false;
    }
    close(fd);
    return true;
}

bool cleanup_containers() {
    std::set<int> pids;
    bool ok = true;
    ok &= get_containers(pids);
    for (int pid : pids) {
        ok &= kill(pid, SIGKILL) != -1;
    }
    ok &= unlink(CONTAINERS_FILE) != -1;
    return ok;
}
