#pragma once

#include <string>

#define AUCONT_PREFIX "/test/aucont/"

void invalid_arg(int idx);

void error(const std::string& message);
void fatal_error(const std::string& message);
