#ifndef MSR_REGISTER
#define MSR_REGISTER

#include <string>
#include <stdint.h>

class MsrRegister {

    private:
        int _fd;
        int _cpu;

    public:
        MsrRegister(int cpu);
        ~MsrRegister();
        
        int is_open();

        int ReadMsr(uint64_t, std::string range, uint64_t *buff);
        int WriteMsr(uint64_t regno, uint64_t pattern);

        int SetMsrBit(uint64_t regno, uint32_t bitno);
        int ClearMsrBit(uint64_t regno, uint32_t bitno);

        int get_cpu();
};

#endif

