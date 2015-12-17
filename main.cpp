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

#define MSR_PLATFORM_INFO            0x0CE
#define MSR_CORE_PERF_GLOBAL_CTRL    0x38F
#define MSR_CORE_PERF_FIXED_CTR_CTRL 0x38D

#define MASK(hex, mask) (hex|mask)
#define BIT(pos) (1ULL << pos)

typedef unsigned char byte;
typedef unsigned int word;
typedef unsigned long int dword;

using namespace std;

class MSRException: public exception
{
    private:
        string _e;
    public:
        MSRException(string e) {
            this->_e = e;
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

        void readMSR(dword msr, dword mask, dword *buff) {
            dword temp;

            /* Error handling here please */
            lseek(this->_fd, (off_t) msr, SEEK_SET);
            read(this->_fd, &temp, sizeof(temp));
            *buff= temp & mask;
        }
};

int main() {
    dword temp = 0;
    
    MSRRegister cpu0(0);
    cpu0.readMSR(MSR_PLATFORM_INFO,rangeToMask("15:8"), &temp);

    cout << "0x" << hex << temp << endl;

}
