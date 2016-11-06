#pragma once

#include <set>

bool add_container(int pid);
bool delete_container(int pid);
bool get_containers(std::set<int>& out_pids);
bool init_containers();
bool cleanup_containers();
