/*
 * cpuinfo - Tool which reports CPU clock frequency using MSR registers
 *
 * Copyright (C) 2016 Marco Guerri <marco.guerri.dev@fastmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

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
MsrRegister::MsrRegister(uint16_t cpu_num) : _base_operating_ratio(-1), _freq(1200)
{
    stringstream path;
    this->_cpu_num = cpu_num;
    
    path << "/dev/cpu/" << this->_cpu_num << "/msr";
    
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
 *  Move constructor. Removes ownership of the file descriptor from the object it
 *  is moving from. 
 */
MsrRegister::MsrRegister(MsrRegister&& rhv)
{
    this->_fd = rhv._fd;
    this->_cpu_num = rhv._cpu_num;
    this->_base_operating_ratio = rhv._base_operating_ratio;
    rhv._fd = -1;
}

/**
 *  Initializes performance counters
 *  @retval Base operating ratio of the CPU
 */
int16_t
MsrRegister::init_counters()
{
    uint64_t buff = 0;
    
    if(this->ReadMsr(MSR_PLATFORM_INFO, string("15:8"), &buff) != 0) 
    {
        error("Could not read base operating ratio from MSR register, CPU %d", 
              this->cpu_num());
        return -1;
    }

    /* Move low 16 bits of 64 bits unsigned to signed 16 */
    this->_base_operating_ratio = buff;
    
    /*  Enabling fixed counter 1 and 2 in the global performance counter control reg.
     *  BIT_FIXED_ARCH_PERF_MONITOR_CTR_1
     *      Counts the number of core cycles while the core is not in halted state.
     *  BIT_FIXED_ARCH_PERF_MONITOR_CTR_2 
     *      Counts the number of base operating frequency cycles while the core is 
     *      not in halted state.
     */
    this->SetMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);
    this->SetMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_2);

    /* Enabling fixed counters for all rings via the fixed counter control register */
    this->SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_LOW);
    this->SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_HIGH);
    this->SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_LOW);
    this->SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_HIGH);

    return this->_base_operating_ratio;
}


/**
 * Disables performance counters
 */
void 
MsrRegister::fini_counters()
{
    /* Disabling fixed counter 1,2 in the global performance counter control register */
    this->ClearMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);
    this->ClearMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_2);
    
    /* Disabling all ring levels for fixed-function counter 1 and 2 */
    this->ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_LOW);
    this->ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_HIGH);
    this->ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_LOW);
    this->ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_HIGH);
}
        
/** 
 * Destructor of MsrRegister class. Closes file descriptors previously opened.
 */
MsrRegister::~MsrRegister() 
{
    if(this->is_open())
        this->fini_counters();
    close(this->_fd);
}
 

/**
 *  Returns the CPU number which is being handled by this object.
 */
uint16_t
MsrRegister::cpu_num() 
{
    return this->_cpu_num;
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
        error("[MSR: %" PRIu64 ", CPU: %d] Not open for reading.", regno, this->cpu_num());
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
        error("[MSR: %" PRIu64 ", CPU: %d] Write error: %s", regno, this->cpu_num(), strerror(errno));
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
        error("[MSR: %" PRIu64 ", CPU: %d] Not open for reading.", regno, this->cpu_num());
        return -1;
    }
    temp = temp && ~BIT(bitno);
    if(this->WriteMsr(regno, temp) != 0) {
        error("[MSR: %" PRIu64 ", CPU: %d] Read error: %s", regno, this->cpu_num(), strerror(errno));
        return -1;
    }
    return 0;
}


void
MsrRegister::sample_counters()
{
    struct cpu_fixed_counters fixed_counters;

    this->ReadMsr(MSR_IA32_FIXED_CTR_1, regmask64, &fixed_counters.counter1);
    this->ReadMsr(MSR_IA32_FIXED_CTR_2, regmask64, &fixed_counters.counter2);

    this->_counters_history.push_back(fixed_counters);

    while(this->_counters_history.size() > 2)
        this->_counters_history.pop_front();
}


float
MsrRegister::get_freq()
{
    if(this->_base_operating_ratio == -1)
        return -1;

    if(this->_counters_history.size() < 2)
    {
        /* Sample as many times as it's necessary to have a prev and curr sample */ 
        while(this->_counters_history.size() != 2)
            this->sample_counters();
    }

    uint64_t ctr_prev, ctr_curr;
    uint64_t diff_ctr1, diff_ctr2;

    ctr_curr = this->_counters_history.back().counter1;
    ctr_prev = this->_counters_history.front().counter1;

    if(ctr_curr > ctr_prev)
        diff_ctr1 = ctr_curr - ctr_prev;
    else
        return this->_freq;

    ctr_curr = this->_counters_history.back().counter2;
    ctr_prev = this->_counters_history.front().counter2;
    

    if(ctr_curr > ctr_prev)
        diff_ctr2 = ctr_curr - ctr_prev;
    else
        return this->_freq;

    /* base_operating_ratio * (Bus CLK) is the base operating frequency */
    float new_freq = this->_base_operating_ratio * BCLK * 
                     static_cast<float>(diff_ctr1)/diff_ctr2;

    this->_freq = new_freq;
    return new_freq;
}


