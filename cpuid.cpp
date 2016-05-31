#include <iostream>
#include <stdint.h>
#include <string>

using namespace std;

string get_vendor_string() {

    uint64_t cpuid_op = 0x0;
    
    /* One more null uint32 to be used as null ptr for the vendor string */
    uint32_t vendor[4] = {0x0, 0x0, 0x0, 0x0};
    __asm__  volatile("movq %3, %%rax\n\t"
                      "cpuid\n\t"
                      "movl %%ebx, %0\n\t"
                      "movl %%edx, %1\n\t"
                      "movl %%ecx, %2\n\t"
                      : "=m" (vendor[0]), "=m" (vendor[1]), "=m" (vendor[2])
                      : "m" (cpuid_op)
                      : "%ebx", "%edx", "%ecx");

    return string((char*)vendor);
}

