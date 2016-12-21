#include <vector>
#include <string>

#include "msr.h"
#include "utils.h"
#include "hardware.h"
#include "debug.h"
#include "cpu.h"

using namespace std;
/**
 *  Initializes performance counters on the MSR register of a specific CPU
 *  @param msr MsrRegister object of a specific CPU
 *
 *  @retval Base operating ratio of the CPU which is being initialized
 *  @retval -1 An error occurred while initializing CPU
 */
uint64_t
init_cpu(MsrRegister& msr)
{
    uint64_t base_operating_ratio = 0;
    if(msr.ReadMsr(MSR_PLATFORM_INFO, string("15:8"), &base_operating_ratio) != 0) 
    {
        error("Could not read base operating ratio from MSR register");
        return 0xFFFFFFFFFFFFFFFF;
    }
    
    /* Enabling fixed counter 1,2 in the global performance counter control register
     *  BIT_FIXED_ARCH_PERF_MONITOR_CTR_1
     *  Counts the number of core cycles while the core is not in halted state.
     *  BIT_FIXED_ARCH_PERF_MONITOR_CTR_2 
     *  Counts the number of base operating frequency cycles while the core 
     *  is not in halted state.
     */
    msr.SetMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);
    msr.SetMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);

    /* Enabling fixed counters for all rings via the fixed counter control register */
    msr.SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_LOW);
    msr.SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_HIGH);
    msr.SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_LOW);
    msr.SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_HIGH);

    return base_operating_ratio;
}

struct cpu_fixed_counters
sample_fixed_counters(MsrRegister &msr)
{
    struct cpu_fixed_counters fixed_counters;

    msr.ReadMsr(MSR_IA32_FIXED_CTR_1, regmask64, &fixed_counters.counter1);
    msr.ReadMsr(MSR_IA32_FIXED_CTR_2, regmask64, &fixed_counters.counter2);

    return fixed_counters;
}


float
calculate_cpu_freq(cpu_fixed_counters *prev, 
                   cpu_fixed_counters *curr, 
                   uint64_t base_operating_ratio)
{
    uint64_t diff_ctr1 = curr->counter1 - prev->counter1;
    uint64_t diff_ctr2 = curr->counter2 - prev->counter2;

    return base_operating_ratio * BCLK * static_cast<float>(diff_ctr1)/diff_ctr2;
}


void
fini_cpu(MsrRegister& msr)
{

    /* Disabling fixed counter 1,2 in the global performance counter control register */
    msr.ClearMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);
    msr.ClearMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);
    
    /* Disabling all ring levels for fixed-function counter 1 and 2 */
    msr.ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_LOW);
    msr.ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_HIGH);
    msr.ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_LOW);
    msr.ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_HIGH);
}








