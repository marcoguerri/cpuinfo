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

#define MSR_SIZE    2048
#define BCLK        133.33

#define BASE_OPERATING_RATIO "15:8"

/*
 * IA32 registers of interest
 */

#define MSR_PLATFORM_INFO                   0x0CE

/*
 * Control registers
 */
#define MSR_IA32_CORE_PERF_GLOBAL_CTRL      0x38F
#define MSR_IA32_PERF_FIXED_CTR_CTRL        0x38D


/*
 * Counter registers
 */
#define MSR_IA32_FIXED_CTR1                 0x30A
#define MSR_IA32_FIXED_CTR2                 0x30B


/*
 * BITS definitions
 */

#define BIT_FIXED_ARCH_PERF_MONITOR_CTR_1   33 /* CPU_CLK_UNHALTED.CORE */
#define BIT_FIXED_ARCH_PERF_MONITOR_CTR_2   34 /* CPU_CLK_UNHALTED.REF  */4

#define BIT_CTL_FC0 0
#define BIT_CTL_FC1 1

#define MASK(hex, mask) (hex|mask)
#define BIT(pos) (1ULL << pos)

using namespace std;

const uint32_t regsize = 32;
const string regmask = "31:0";


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
    for(int shift = start; shift >= end; shift--) 
        mask |= (1ULL << shift);
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

            this->_fd = open(path.str().c_str(), O_RDONLY);
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

            cout << "Reading MSR register 0x" << hex << regno << endl;
            /* Error handling here please */
            lseek(this->_fd, (off_t) regno, SEEK_SET);
            int num_read  = read(this->_fd, &temp, sizeof(temp));

            if(num_read != sizeof(temp)) {
                cout << "There was an error while reading MSR register" << endl;
            }

            *buff= align(temp & rangeToMask(range), range);
        }

        /*
         *  Writes 64 bits to the MSR register regno
         */
        void writeMSR(uint64_t regno, uint64_t pattern) {
            lseek(this->_fd, (off_t) regno, SEEK_SET);
            int num_written = write(this->_fd, &pattern, sizeof(pattern));
            if(num_written != sizeof(pattern)) {
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

};

int main() {
    uint64_t temp = 0;
    MSRRegister cpu0(0);
     
    cout << "Reading MSR_PLATFORM_INFO(15:8) " << endl; 
    cpu0.readMSR(MSR_PLATFORM_INFO, string("15:8"), &temp);

    cout << "MSR_PLATFORM_INFO(15:8) "; 
    cout << "0x" << hex << temp << endl;

    /* Enabling 
     *  - BIT_FIXED_ARCH_PERF_MONITOR_CTR_1
     *       Counts the number of core cycles while the core is not in halted
     *       sate.
     *  - BIT_FIXED_ARCH_PERF_MONITOR_CTR_2 
     *       Counts the number of base operating frequency cycles while the core
     *       is not in halted state.
     *
     * in MSR_IA32_PERF_FIXED_CTR_CTRL
     */

    //cpu0.setMSRBit(BIT_FIXED_ARCH_PERF_MONITOR_CTR_1, MSR_IA32_PERF_FIXED_CTR_CTRL);
    
    /* Enabling the Fixed-Function Performance Counter Control, aka MSR_IA32_PERF_FIXED_CTR_CTRL. 
     *
     * Settings bits:
     *  - BIT_CTL_FC0
     *  - BIT_CTL_FC1
     */
    //cpu0.setMSRBit(BIT_CTL_FC0, MSR_IA32_PERF_FIXED_CTR_CTRL)
    //cpu0.setMSRBit(BIT_CTL_FC1, MSR_IA32_PERF_FIXED_CTR_CTRL)

