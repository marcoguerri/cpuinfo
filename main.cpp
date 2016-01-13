#include <iostream>
#include <sstream>
#include <memory>


#include <stdio.h>
#include <unistd.h>
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
#define MSR_IA32_CORE_PERF_GLOBAL_CTRL      0x38F
#define MSR_IA32_PERF_FIXED_CTR_CTRL        0x38D

#define BIT_FIXED_ARCH_PERF_MONITOR_CTR_1   33 /* CPU_CLK_UNHALTED.CORE */
#define BIT_FIXED_ARCH_PERF_MONITOR_CTR_2   34 /* CPU_CLK_UNHALTED.REF  */4

#define MASK(hex, mask) (hex|mask)
#define BIT(pos) (1ULL << pos)

typedef unsigned char byte;
typedef unsigned int word;
typedef unsigned long int dword;


const dword regsize = 32;
const string regmask = "31:0";

using namespace std;

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

dword rangeToMask(string range) {
    word start, end;
    byte separator;

    stringstream ss(range);
    ss >> start >> separator >> end;

    dword mask = 0x0;
    for(int shift = start; shift >= end; shift--) 
        mask |= (1ULL << shift);
    return mask;
}


dword align(dword reg, string range) {

    word start, end;
    byte separator;

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
                throw MSRException("Error while opening MSR");
            } 
        }
        
        ~MSRRegister() {
            close(this->_fd);
        }

        /*
         *  Reads sizeof(dword), normally 32 bits, from the MSR register regno
         */

        void readMSR(dword regno, string  range, dword *buff) {
            dword temp;

            /* Error handling here please */
            lseek(this->_fd, (off_t) regno, SEEK_SET);
            read(this->_fd, &temp, sizeof(temp));
            *buff= align(temp & rangeToMask(range), range);
        }

        /*
         *  Write sizeof(dword), normally 32 bits, from the MSR register regno
         */
        void writeMSR(dword regno, dword pattern) {
            dword temp;
            lseek(this->_fd, (off_t) regno, SEEK_SET);
            write(this->_fd, (void*)&pattern, sizeof(pattern));
        }


        void setMSRBit(dword regno, dword bitno) {
            dword temp;
            this->readMSR(regno, regmask, &temp);
            temp = temp | (1ULL << bitno);
        }

};

int main() {
    dword temp = 0;
    MSRRegister cpu0(0);
 
    cpu0.readMSR(MSR_PLATFORM_INFO, string("15:8"), &temp);

    cout << "MSR_PLATFORM_INFO(15:8) "; 
    cout << "0x" << hex << temp << endl;

    /* Enabling 
       - BIT_FIXED_ARCH_PERF_MONITOR_CTR_1
       - BIT_FIXED_ARCH_PERF_MONITOR_CTR_2 

      in MSR_IA32_PERF_FIXED_CTR_CTRL

    */
    cpu0.setMSRBit(BIT_FIXED_ARCH_PERF_MONITOR_CTR_1, MSR_IA32_PERF_FIXED_CTR_CTRL)

}
