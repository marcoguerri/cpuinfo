/************************************************************************
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

/**
 * @brief File defining a class for reading/writing MSR registers via 
 * /dev/cpu/X/msr exported by msr kernel module.
 */
#define __STDC_FORMAT_MACROS

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
#include <assert.h>
#include <string.h>
#include <inttypes.h>

#include "hardware.h"
#include "utils.h"
#include "msr.h"
#include "debug.h"

using namespace std;

/**
 *  Constructor of MsrRegister class.
 *  @param[in]  cpu Number of the CPU that the class will be responsible for.
 */

MsrRegister::MsrRegister(int cpu) 
{
    stringstream path;
    this->_cpu = cpu;
    
    path << "/dev/cpu/" << cpu << "/msr";
    
    this->_fd = open(path.str().c_str(), O_RDWR);
    if(this->_fd < 0 ) { 
        if( errno == ENOENT) {
            error("MSR device file does not exist (msr kernel module loaded?)");
        } else {
            error("Could not open %s: %s", path.str().c_str(), strerror(errno));
        }
    }
}
        
/** 
 * Destructor of MsrRegister class. Closes file descriptors previously opened.
 */
MsrRegister::~MsrRegister() 
{
    close(this->_fd);
}
 

/**
 *  Returns the CPU number which is being handled by this object.
 */
int
MsrRegister::get_cpu() 
{
    return this->_cpu;
}
   
int
MsrRegister::is_open() 
{
    return this->_fd >= 0;
}


/**
 *  Reads MSR register
 *  @param regno  MSR regsiter offset
 *  @param range  Range of bits in the form X:Y to be read from the register
 *  @param buff   Pointer to the buffer which will contain the bit range read from 
 *                the register shifted to the rightmost position
 *
 *  @retval 0   Read operation was successful
 *  @retval -1  Read operation failed
 *
 */
int 
MsrRegister::ReadMsr(uint64_t regno, std::string  range, uint64_t *buff) 
{
    assert(buff != nullptr);
    if(!this->is_open()) {
        error("[MSR: %" PRIu64 ", CPU: %d] Not open for reading.", regno, this->get_cpu());
        return -1;
    }
        
    uint64_t temp = 0;
    int num_read  = pread(this->_fd, &temp, sizeof(temp), regno);
    if(num_read  != sizeof(temp))
        return -1;

    *buff= align(temp & range_to_mask(range), range);
    return 0;
}

/**
 *  Writes MSR register
 *  @param regno    MSR regsiter offset
 *  @param pattern  Pattern to be written to the register
 *
 *  @retval 0   Write operation was successful
 *  @retval -1  Write operation failed
 *
 */

int 
MsrRegister::WriteMsr(uint64_t regno, uint64_t pattern) 
{
    if(!this->is_open()) {
        return -1;
    }
    
    int num_written = pwrite(this->_fd, &pattern, sizeof(pattern), regno);
    if(num_written != sizeof(pattern)) {
        error("[MSR: %" PRIu64 ", CPU: %d] Write error: %s", regno, this->get_cpu(), strerror(errno));
        return -1;
    }
    return 0;
}

/**
 *  Sets a single bit on MSR register leaving everything else unchanged
 *  @param regno    MSR regsiter offset
 *  @param bitno    Bit to be set in the register
 *
 *  @retval 0   Set operation was successful
 *  @retval -1  Set operation failed
 *
 */
int
MsrRegister::SetMsrBit(uint64_t regno, uint32_t bitno) 
{
    assert(bitno < regsize);
    uint64_t temp;
    if(this->ReadMsr(regno, regmask64, &temp) != 0) {
        return -1;
    }
    temp = temp | BIT(bitno);
    if(this->WriteMsr(regno, temp) != 0) {
        return -1;
    }
    return 0;
}


/**
 *  Clears a single bit on MSR register leaving everything else unchanged
 *  @param regno    MSR regsiter offset
 *  @param bitno    Bit to be set in the register
 *
 *  @retval 0   Clear operation was successful
 *  @retval -1  Clear operation failed
 *
 */
int
MsrRegister::ClearMsrBit(uint64_t regno, uint32_t bitno) 
{
    assert(bitno < regsize);

    uint64_t temp;
    if(this->ReadMsr(regno, regmask64, &temp) != 0) {
        error("[MSR: %" PRIu64 ", CPU: %d] Not open for reading.", regno, this->get_cpu());
        return -1;
    }
    temp = temp && ~BIT(bitno);
    if(this->WriteMsr(regno, temp) != 0) {
        error("[MSR: %" PRIu64 ", CPU: %d] Read error: %s", regno, this->get_cpu(), strerror(errno));
        return -1;
    }
    return 0;
}
