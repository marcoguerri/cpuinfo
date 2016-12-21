/*
 * cpuinfo - Tool which reports CPU statistics using MSR registers
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

class MsrRegister {

    private:
        int _fd;
        int _cpu;

    public:
        MsrRegister(int cpu);
        ~MsrRegister();
        
        int is_open();

        int ReadMsr(uint64_t, std::string range, uint64_t *buff);
        int WriteMsr(uint64_t regno, uint64_t pattern);

        int SetMsrBit(uint64_t regno, uint32_t bitno);
        int ClearMsrBit(uint64_t regno, uint32_t bitno);

        int get_cpu();
};

#endif

