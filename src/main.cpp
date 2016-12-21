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

#include <vector>
#include <string>
#include <iomanip>

#include "msr.h"
#include "utils.h"
#include "hardware.h"
#include "debug.h"
#include "cpu.h"

using namespace std; 

bool sample = true;

int 
main() 
{
    
    signal(SIGINT, sigint_callback); 
    MsrRegister cpu0(0);

    struct cpu_fixed_counters prev, curr;
    struct timespec ts = { .tv_sec = 1, .tv_nsec = 0 };
    float prev_freq = 1200, freq = 0; 

    uint64_t base_operating_ratio = init_cpu(cpu0);
    if(base_operating_ratio == 0xFFFFFFFFFFFFFFFF)
    {
        error("Error while initializing CPU");
        return EXIT_FAILURE;
    }

    prev = sample_fixed_counters(cpu0); 
    nanosleep(&ts,NULL);
    while(sample) {
        
        curr = sample_fixed_counters(cpu0); 
        freq = calculate_cpu_freq(&prev, &curr, base_operating_ratio);
        if(freq > FREQ_BOOST_FACTOR_LIMIT * prev_freq)
            freq = prev_freq;
        
        cout << "\rFrequency: ";
        cout << setw(11) << std::fixed << std::setprecision(2) << freq;
        
        prev = curr;
        prev_freq = freq;
 
        fflush(stdout);
        nanosleep(&ts,NULL);
    }
    
    fini_cpu(cpu0); 
    return EXIT_SUCCESS;

}
