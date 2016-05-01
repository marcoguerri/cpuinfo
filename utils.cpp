#include <sstream>
#include <string>
#include <iostream>

#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include "utils.h"

extern bool sample;

void sigint_callback(int signum) 
{
    sample = false;
    exit(signum);
}

uint64_t range_to_mask(std::string range) {
    int32_t start, end;
    uint8_t separator;

    std::stringstream ss(range);
    ss >> start >> separator >> end;
    uint64_t mask = 0x0;
    for(int32_t shift = start; shift >= end; shift--) {
        mask |= BIT(shift);
    }
    return mask;
}


uint64_t align(uint64_t reg, std::string range) {

    uint64_t start, end;
    uint8_t separator;

    std::stringstream ss(range);
    ss >> start >> separator >> end;
    return reg >> end;
}
