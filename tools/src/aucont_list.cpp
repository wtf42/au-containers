#include <iostream>
#include <set>

#include "pid_storage.h"


int main() {
    std::set<int> pids;

    if (!get_containers(pids)) {
        std::cerr << "failed to read containers list" << std::endl;
        return 1;
    }

    for (int pid : pids) {
        std::cout << pid << std::endl;
    }

    return 0;
}
