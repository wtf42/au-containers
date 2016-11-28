#include "pid_storage.h"
#include "common.h"

#include <signal.h>
#include <unistd.h>
#include <set>
#include <fstream>

const char CONTAINERS_FILE[] = AUCONT_PREFIX".containers";

std::set<int> read_pids() {
    std::set<int> result;

    std::ifstream in(CONTAINERS_FILE);
    int pid;
    while (in >> pid) {
        result.insert(pid);
    }
    return result;
}

void write_pids(const std::set<int>& pids) {
    std::ofstream out(CONTAINERS_FILE, std::ios::trunc);
    for (int pid : pids) {
        out << pid << std::endl;
    }
}

bool get_containers(std::set<int>& out_pids) {
    out_pids = read_pids();
    return true;
}

bool delete_container(int pid) {
    std::set<int> pids = read_pids();
    auto it = pids.find(pid);
    if (it == pids.end()) {
        return false;
    }
    pids.erase(it);
    write_pids(pids);
    return true;
}

bool add_container(int pid) {
    std::set<int> pids = read_pids();
    if (pids.find(pid) != pids.end()) {
        return false;
    }
    pids.insert(pid);
    write_pids(pids);
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
