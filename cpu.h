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
