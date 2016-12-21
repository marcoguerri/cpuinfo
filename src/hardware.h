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

#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>
#include <string>
#include <inttypes.h>

const uint32_t regsize = 64;
const std::string regmask64 = "63:0";

#define FREQ_BOOST_FACTOR_LIMIT 5
#define BCLK        100 

#define BASE_OPERATING_RATIO "15:8"

/* IA32 registers of interest */
#define MSR_PLATFORM_INFO                   0x0CE

/* Control registers */
#define MSR_IA32_CORE_PERF_GLOBAL_CTRL      0x38F
#define MSR_IA32_FIXED_CTR_CTRL             0x38D

/* Counter registers */
#define MSR_IA32_FIXED_CTR_1                0x30A
#define MSR_IA32_FIXED_CTR_2                0x30B

/*
 * BITS definitions
 */
#define BIT_FIXED_ARCH_PERF_MONITOR_CTR_1   33 /* CPU_CLK_UNHALTED.CORE */
#define BIT_FIXED_ARCH_PERF_MONITOR_CTR_2   34 /* CPU_CLK_UNHALTED.REF  */

#define BIT_CONTROL_FIXED_COUNTER_1_LOW  4
#define BIT_CONTROL_FIXED_COUNTER_1_HIGH 5

#define BIT_CONTROL_FIXED_COUNTER_2_LOW  8
#define BIT_CONTROL_FIXED_COUNTER_2_HIGH 9


#endif

