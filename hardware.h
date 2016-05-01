#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>
#include <string>

const uint32_t regsize = 64;
const std::string regmask = "63:0";

#define BCLK        100 

#define BASE_OPERATING_RATIO "15:8"g

/*
 * * IA32 registers of interest
 * */
#define MSR_PLATFORM_INFO                   0x0CE
/*
 *  * Control registers
 *   */
#define MSR_IA32_CORE_PERF_GLOBAL_CTRL      0x38F
#define MSR_IA32_FIXED_CTR_CTRL             0x38D
/*
 *  * Counter registers
 *   */
#define MSR_IA32_FIXED_CTR_1                0x30A
#define MSR_IA32_FIXED_CTR_2                0x30B

/*
 *  * BITS definitions
 *   */
#define BIT_FIXED_ARCH_PERF_MONITOR_CTR_1   33 /* CPU_CLK_UNHALTED.CORE */
#define BIT_FIXED_ARCH_PERF_MONITOR_CTR_2   34 /* CPU_CLK_UNHALTED.REF  */

#define BIT_CONTROL_FIXED_COUNTER_1_LOW  4
#define BIT_CONTROL_FIXED_COUNTER_1_HIGH 5

#define BIT_CONTROL_FIXED_COUNTER_2_LOW  8
#define BIT_CONTROL_FIXED_COUNTER_2_HIGH 9


#endif

