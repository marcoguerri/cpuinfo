#include <iostream>
#include <sstream>
#include <memory>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

#define BCLK        100

#define BASE_OPERATING_RATIO "15:8"

/*
 * IA32 registers of interest
 */
#define MSR_PLATFORM_INFO                   0x0CE
/*
 * Control registers
 */
#define MSR_IA32_CORE_PERF_GLOBAL_CTRL      0x38F
#define MSR_IA32_FIXED_CTR_CTRL             0x38D
/*
 * Counter registers
 */
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

#define MASK(hex, mask) (hex|mask)
#define BIT(pos) (1ULL << pos)

using namespace std;

const uint32_t regsize = 64;
const string regmask = "63:0";
bool sample = true;

void sigint_callback(int signum) 
{
    cout << "Got SIGINT, exiting..." << endl;
    sample = false;
    exit(signum);
}


class MSRException: public exception
{
    private:
        string _e;
    public:
        MSRException(string e) {
            this->_e = e;
        }
        ~MSRException() throw () {
        }
        virtual const char* what() const throw(){
            return this->_e.c_str();
        }
};

uint64_t rangeToMask(string range) {
    uint32_t start, end;
    uint8_t separator;

    stringstream ss(range);
    ss >> start >> separator >> end;
    uint64_t mask = 0x0;
    for(int shift = start; shift >= (int)end; shift--) {
        mask |= (1ULL << shift);
    }
    return mask;
}


uint64_t align(uint64_t reg, string range) {

    uint64_t start, end;
    uint8_t separator;

    stringstream ss(range);
    ss >> start >> separator >> end;

    return reg >> end;

}
class MSRRegister {

    private:
        int _fd;

    public:
        MSRRegister(int cpu) {

            stringstream path;
            path << "/dev/cpu/" << cpu << "/msr";

            this->_fd = open(path.str().c_str(), O_RDWR);
            if(this->_fd < 0 ) {
                cout << "Error while opening MSR char device " << endl;
                throw MSRException("Error while opening MSR");
            } 
        }
        
        ~MSRRegister() {
            close(this->_fd);
        }

        /*
         *  Reads 32 bits from the MSR register regno
         */

        void readMSR(uint64_t regno, string  range, uint64_t *buff) {
            uint64_t temp = 0;
            int num_read  = pread(this->_fd, &temp, sizeof(temp), regno);
            if(num_read  == -1) {
                cout << "There was an error while reading MSR register" << endl;
            }
            *buff= align(temp & rangeToMask(range), range);
        }

        /*
         *  Writes 64 bits to the MSR register regno
         */
        void writeMSR(uint64_t regno, uint64_t pattern) {
            int num_written = pwrite(this->_fd, &pattern, sizeof(pattern), regno);
            if(num_written == -1) {
                cout << "Error while writing MSR register" << endl;
            }
        }
        /*
         * Sets one bit on a specific MSR register
         */
        void setMSRBit(uint64_t regno, uint32_t bitno) {
            uint64_t temp;
            this->readMSR(regno, regmask, &temp);
            temp = temp | (1ULL << bitno);
            this->writeMSR(regno, temp);
        }

        /*
         * Resets one bit on a specific MSR register
         */
        void clearMSRBit(uint64_t regno, uint32_t bitno) {
            uint64_t temp;
            this->readMSR(regno, regmask, &temp);
            temp = temp && ~(1ULL << bitno);
            this->writeMSR(regno, temp);
       }
};

int main() {
    MSRRegister cpu0(2);
    
    signal(SIGINT, sigint_callback); 
    
    /* Computing the platform reference frequency */
    uint64_t base_operating_ratio = 0;
    cpu0.readMSR(MSR_PLATFORM_INFO, string("15:8"), &base_operating_ratio);

    /* Enabling fixed counter 1 and 2 via the global performance counter control register
     *  - BIT_FIXED_ARCH_PERF_MONITOR_CTR_1
     *       Counts the number of core cycles while the core is not in halted
     *       sate.
     *  - BIT_FIXED_ARCH_PERF_MONITOR_CTR_2 
     *       Counts the number of base operating frequency cycles while the core
     *       is not in halted state.
     *
     * in MSR_IA32_CORE_PERF_GLOBAL_CTRL
     */

    cpu0.setMSRBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, 
                   BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);

    cpu0.setMSRBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, 
                   BIT_FIXED_ARCH_PERF_MONITOR_CTR_2);
    
    /* Enabling the Fixed Counters in MSR_IA32_FIXED_CTR_CTRL for all ring levels. */

    cpu0.writeMSR(MSR_IA32_FIXED_CTR_CTRL, 1UL << BIT_CONTROL_FIXED_COUNTER_1_LOW  |
                                           1UL << BIT_CONTROL_FIXED_COUNTER_1_HIGH |
                                           1UL << BIT_CONTROL_FIXED_COUNTER_2_LOW  | 
                                           1UL << BIT_CONTROL_FIXED_COUNTER_2_HIGH);

    uint64_t prev_fixed_ctr1 = 0, current_fixed_ctr1;
    uint64_t prev_fixed_ctr2 = 0, current_fixed_ctr2; 
    uint64_t reference_frequency = base_operating_ratio * BCLK;

    struct timespec ts;
    ts.tv_sec   = 2;
    ts.tv_nsec  = 0;

    while(sample)
    {

        cpu0.readMSR(MSR_IA32_FIXED_CTR_1, string("63:0"), &current_fixed_ctr1);
        cpu0.readMSR(MSR_IA32_FIXED_CTR_2, string("63:0"), &current_fixed_ctr2);
        
        uint64_t diff_ctr1 = current_fixed_ctr1 - prev_fixed_ctr1;
        prev_fixed_ctr1 = current_fixed_ctr1;
        
        uint64_t diff_ctr2 = current_fixed_ctr2 - prev_fixed_ctr2;
        prev_fixed_ctr2 = current_fixed_ctr2;

        cout << diff_ctr2 << " " << diff_ctr1 << endl;
        //cout << "Base operating ratio: " << base_operating_ratio << endl;
        //cout << "Reference frequency: " << reference_frequency << endl;
        //cout << "Counters ratio " << (float)diff_ctr2/diff_ctr1  << endl;
        cout << "Frequency: " << dec <<reference_frequency * (float)diff_ctr1/diff_ctr2 << endl;
        nanosleep(&ts,NULL);

    }

    /* Disabling Monitor counter 1 and 2 in the global performance counter control register*/
    cpu0.clearMSRBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, 
                     BIT_FIXED_ARCH_PERF_MONITOR_CTR_1);

    cpu0.clearMSRBit(MSR_IA32_CORE_PERF_GLOBAL_CTRL, 
                     BIT_FIXED_ARCH_PERF_MONITOR_CTR_2);

    /* Disabling the fixed-function performance counter 1 and 2*/

    cpu0.clearMSRBit(MSR_IA32_FIXED_CTR_CTRL, 
                     BIT_CONTROL_FIXED_COUNTER_1_LOW);

    cpu0.clearMSRBit(MSR_IA32_FIXED_CTR_CTRL, 
                     BIT_CONTROL_FIXED_COUNTER_1_HIGH);

    cpu0.clearMSRBit(MSR_IA32_FIXED_CTR_CTRL, 
                     BIT_CONTROL_FIXED_COUNTER_2_LOW);

    cpu0.clearMSRBit(MSR_IA32_FIXED_CTR_CTRL,
                     BIT_CONTROL_FIXED_COUNTER_2_HIGH);
}