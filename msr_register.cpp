#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "hardware.h"
#include "utils.h"
#include "msr_register.h"

using namespace std;

MsrRegister::MsrRegister(int cpu) {
    stringstream path;
    path << "/dev/cpu/" << cpu << "/msr";
    
    this->_fd = open(path.str().c_str(), O_RDWR);
    if(this->_fd < 0 ) 
        cout << "There was an error opening MSR register";
}
        
MsrRegister::~MsrRegister() {
    close(this->_fd);
}
    

bool MsrRegister::is_open() {
    return this->_fd  >= 0;
}

/*
 *  Reads MSR register regno. Supports for now only 64 bits arch.
 */
int MsrRegister::ReadMsr(uint64_t regno, std::string  range, uint64_t *buff) {
    uint64_t temp = 0;
    int num_read  = pread(this->_fd, &temp, sizeof(temp), regno);
    if(num_read  == -1) {
        cout << "Error reading MSR register";
        return num_read;
    }
    *buff= align(temp & range_to_mask(range), range);
    return num_read;
}


/*
 *  Writes pattern to MSR register regno. Supports for now only 64 bits arch,
 */
int MsrRegister::WriteMsr(uint64_t regno, uint64_t pattern) {
    int num_written = pwrite(this->_fd, &pattern, sizeof(pattern), regno);
    if(num_written == -1) {
        cout << "Error writing MSR register";
    }
    return num_written;
}

/*
 * Sets bit bitno on MSR register regno
 */
void MsrRegister::SetMsrBit(uint64_t regno, uint32_t bitno) {
    uint64_t temp;
    this->ReadMsr(regno, regmask, &temp);
    temp = temp | BIT(bitno);
    this->WriteMsr(regno, temp);
}

/*
 * Clears bit bitno on on MSR register regno
 */
void MsrRegister::ClearMsrBit(uint64_t regno, uint32_t bitno) {
    uint64_t temp;
    this->ReadMsr(regno, regmask, &temp);
    temp = temp && ~BIT(bitno);
    this->WriteMsr(regno, temp);
}
