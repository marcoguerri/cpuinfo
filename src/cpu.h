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

#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>
#include "msr.h"

struct cpu_fixed_counters {
    uint64_t counter1;
    uint64_t counter2;
};

uint64_t init_cpu(MsrRegister& msr);
void fini_cpu(MsrRegister& msr);

float calculate_cpu_freq(cpu_fixed_counters *prev, 
                         cpu_fixed_counters *curr, 
                         uint64_t base_operating_ratio);

struct cpu_fixed_counters sample_fixed_counters(MsrRegister &msr);
#endif