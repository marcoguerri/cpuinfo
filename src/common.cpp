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

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include "utils.h"
#include "hardware.h"
#include "debug.h"
#include "cpu.h"

using namespace std;

/**
 * Returns the number of logical CPUs based on /proc/stat
 */
int16_t
get_number_cpus()
{
    uint16_t num_cpus = 0;
    uint16_t cpu_num;

    char c,p,u;

    string line;
    ifstream f("/proc/stat");
    
    if(!f.is_open())
    {
        error("Error while opening /proc/stat: %s", strerror(errno));
        return -1;
    }

    /* Discard first list (cpu ...) */
    getline(f, line);
    
    while(getline(f, line))
    {
        istringstream ss(line);
        if(ss >> c >> p >> u >> cpu_num) 
            num_cpus++;
            continue;   
    }
    return num_cpus;
}

