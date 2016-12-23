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
#include <iomanip>
#include <memory>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>

#include "msr.h"
#include "utils.h"
#include "hardware.h"
#include "debug.h"
#include "cpu.h"

using namespace std; 

bool sample = true;

int 
main(int argc, char **argv) 
{
    signal(SIGINT, sigint_callback); 
    float freq = 0;   
    int c = 0;
    uint8_t interval = 1;
    char cpu_list_cli[256], sep;

    /* Parsing command line options */
    while ((c = getopt (argc, argv, "i:c:")) != -1)
    switch (c)
    {
        case 'i':
            interval = atoi(optarg);
            break;
        case 'c':
            strncpy(cpu_list_cli, optarg, 256);
            break;
        default:
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            return EXIT_FAILURE;
    }
    struct timespec ts = { .tv_sec = interval, .tv_nsec = 0 };
    
    /* Extracting the list of CPUs to be monitored */
    uint16_t cpu;
    vector<uint16_t> cpu_list;
    stringstream ss(cpu_list_cli);
    
     while(ss >> cpu)
    {
        cpu_list.push_back(cpu);
        ss >> sep;
    }
    sort(cpu_list.begin(), cpu_list.end());

    /* Initializing CPU objects */ 
    vector<MsrRegister> cpu_vector;
    for(uint16_t n : cpu_list)
    {
        if(n >= get_number_cpus())
        {
            error("CPU number higher then number of CPUs");
            return EXIT_FAILURE;
        }
        cpu_vector.emplace_back(MsrRegister(n));
    }
     
    /* If cpu_vector is empty, then no CPU was specified. Monitor them all */
    if(cpu_vector.size() == 0)
        for(uint16_t j = 0; j< get_number_cpus(); ++j)
            cpu_vector.emplace_back(MsrRegister(j));
    
    
    vector<MsrRegister>::iterator cpu_iterator;
    FOR_ALL_CPUS(cpu_iterator, cpu_vector, init_counters);
    FOR_ALL_CPUS(cpu_iterator, cpu_vector, sample_counters);

    while(sample) {
        FOR_ALL_CPUS(cpu_iterator, cpu_vector, sample_counters);
        stringstream ss;
        for(uint16_t j = 0; j < cpu_vector.size(); ++j)
        {
            freq = cpu_vector[j].get_freq();
            ss << "\rFrequency CPU" << setw(2) << cpu_vector[j].cpu_num() << ": ";
            ss << setw(8) << std::fixed << std::setprecision(2) << freq << "\n";
        }
        
        cout << ss.str();
        fflush(stdout);
        nanosleep(&ts,NULL);
        cout << "\033[2J";
        cout << "\033[H";
    }
   
    FOR_ALL_CPUS(cpu_iterator, cpu_vector, fini_counters);
    
    return EXIT_SUCCESS;

}
