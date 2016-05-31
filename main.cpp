#include <vector>
#include <string>

#include "msr.h"
#include "utils.h"
#include "hardware.h"
#include "debug.h"

using namespace std; 

bool sample = true;

int 
main() 
{
    
    signal(SIGINT, sigint_callback); 
    
    MsrRegister cpu0(0);

    /* Computing the platform reference frequency */
    uint64_t base_operating_ratio = 0;
    if(cpu0.ReadMsr(MSR_PLATFORM_INFO, string("15:8"), &base_operating_ratio) != 0) 
    {
        error("Could not read base operating ratio from MSR register");
        return EXIT_FAILURE;
    }
    
    /* Enabling fixed counter 1 and 2 in the global performance counter control register
     *    
     *    BIT_FIXED_ARCH_PERF_MONITOR_CTR_1
     *    Counts the number of core cycles while the core is not in halted state.
     *    
     *    BIT_FIXED_ARCH_PERF_MONITOR_CTR_2 
     *    Counts the number of base operating frequency cycles while the core 
     *    is not in halted state.
     */
    cpu0.SetMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);
    cpu0.SetMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);

    /* Enabling the fixed counters for all rings via the fixed counter control register*/
    cpu0.SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_LOW);
    cpu0.SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_HIGH);
    cpu0.SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_LOW);
    cpu0.SetMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_HIGH);

    uint64_t prev_fixed_ctr1 = 0, current_fixed_ctr1;
    uint64_t prev_fixed_ctr2 = 0, current_fixed_ctr2; 
    
    uint64_t reference_frequency = base_operating_ratio * BCLK;

    struct timespec ts;
    ts.tv_sec = 1;
    ts.tv_nsec = 0;

    cpu0.ReadMsr(MSR_IA32_FIXED_CTR_1, regmask, &prev_fixed_ctr1);
    cpu0.ReadMsr(MSR_IA32_FIXED_CTR_2, regmask, &prev_fixed_ctr2);
    
    nanosleep(&ts,NULL);
    while(sample) {

        cpu0.ReadMsr(MSR_IA32_FIXED_CTR_1, regmask, &current_fixed_ctr1);
        cpu0.ReadMsr(MSR_IA32_FIXED_CTR_2, regmask, &current_fixed_ctr2);
        
        uint64_t diff_ctr1 = current_fixed_ctr1 - prev_fixed_ctr1;
        uint64_t diff_ctr2 = current_fixed_ctr2 - prev_fixed_ctr2;
        
        prev_fixed_ctr1 = current_fixed_ctr1;
        prev_fixed_ctr2 = current_fixed_ctr2;

        //cout << "Base operating ratio: " << base_operating_ratio << endl;
        //cout << "Reference frequency: " << reference_frequency << endl;
        //cout << "Counters ratio " << (float)diff_ctr2/diff_ctr1  << endl;
        
        cout << "\rFrequency: " \
             << dec \
             << reference_frequency * static_cast<float>(diff_ctr1)/diff_ctr2;

        fflush(stdout);
        nanosleep(&ts,NULL);
    }
    
    /* Disabling fixed counter 1 and 2 in the global performance counter 
     * control register */
    cpu0.ClearMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);
    cpu0.ClearMsrBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);
    
    /* Disabling all ring levels for fixed-function counter 1 and 2 */
    cpu0.ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_LOW);
    cpu0.ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_1_HIGH);
    cpu0.ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_LOW);
    cpu0.ClearMsrBit(MSR_IA32_FIXED_CTR_CTRL, BIT_CONTROL_FIXED_COUNTER_2_HIGH);
    
    return EXIT_SUCCESS;

}
