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

