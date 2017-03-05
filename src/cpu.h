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

#ifndef MSR_REGISTER
#define MSR_REGISTER

#include <string>
#include <stdint.h>
#include <deque>

#include "cpu.h"

struct cpu_fixed_counters {
    uint64_t counter1;
    uint64_t counter2;
};

class Cpu {

    private:
        int _fd;
        uint16_t _cpu_num;
        int16_t _base_operating_ratio;
        float _freq;
        std::deque<struct cpu_fixed_counters> _counters_history;

        /* This objec can't be copied, it has ownership of the fd for /dev/msr */    
        Cpu(Cpu&);

    public:
        Cpu(uint16_t cpu);
        Cpu(Cpu&& rhv);
        ~Cpu();
        
        /* Functions for handling MSR registers */
        int is_open();
        int ReadMsr(uint64_t, std::string range, uint64_t *buff);
        int WriteMsr(uint64_t regno, uint64_t pattern);
        int SetMsrBit(uint64_t regno, uint32_t bitno);
        int ClearMsrBit(uint64_t regno, uint32_t bitno);

        int16_t init_counters();
        void fini_counters();
        uint16_t cpu_num();
       
        void sample_counters();
        float get_freq();
};

#endif

