#ifndef _KERNEL_CONSOLE_HPP
#define _KERNEL_CONSOLE_HPP

#include "syscall_c.h"

// singleton
// putc preko niti
// getc preko prekida

class KernelConsole {
public:
    static void init();

    static void handleRxInterrupt();

    static void putcKernel(char c);   // upise znak u izlazni bafer
    static char getcKernel();         // procita znak iz ulaznog bafera

    static sem_t outItemAvailable, outSpaceAvailable, outMutexHead, outMutexTail;
    static sem_t inItemAvailable, inMutexHead;

private:

    static void outputThreadBody(void* arg);

    // izlazni bafer
    static const int OUT_CAP = 64;
    static const int OUT_ARR = OUT_CAP + 1;
    static char  outBuf[OUT_ARR];
    static int   outHead, outTail;

    // ulazni bafer
    static const int IN_CAP = 64;
    static const int IN_ARR = IN_CAP + 1;
    static char  inBuf[IN_ARR];
    static int   inHead, inTail;
};

#endif