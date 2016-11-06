#include <stdlib.h>
#include <string>
#include <iostream>

void invalid_arg(int idx) {
    std::cerr << "invalid argument " << idx << std::endl;
    exit(1);
}

void error(const std::string& message) {
    perror(message.c_str());
    fflush(stderr);
}

void fatal_error(const std::string& message) {
    error(message);
    exit(1);
}
