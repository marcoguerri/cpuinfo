#ifndef UTILS_H
#define UTILS_H

#include <sstream>
#include <string>
#include <iostream>

#include <stdint.h>
#include <signal.h>
#include <stdlib.h>

#define MASK(hex, mask) (hex|mask)
#define BIT(pos) (1ULL << pos)

void sigint_callback(int signum);
uint64_t range_to_mask(std::string range);
uint64_t align(uint64_t reg, std::string range);

#endif

