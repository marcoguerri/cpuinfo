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

#include <iostream>
#include <stdint.h>
#include <string>

using namespace std;

string get_vendor_string() {

    uint64_t cpuid_op = 0x0;
    
    /* One more null uint32 to be used as null ptr for the vendor string */
    uint32_t vendor[4] = {0x0, 0x0, 0x0, 0x0};
    __asm__  volatile("movq %3, %%rax\n\t"
                      "cpuid\n\t"
                      "movl %%ebx, %0\n\t"
                      "movl %%edx, %1\n\t"
                      "movl %%ecx, %2\n\t"
                      : "=m" (vendor[0]), "=m" (vendor[1]), "=m" (vendor[2])
                      : "m" (cpuid_op)
                      : "%ebx", "%edx", "%ecx");

    return string((char*)vendor);
}

