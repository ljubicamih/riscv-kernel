#ifndef KERNELSEMAPHORE_HPP
#define KERNELSEMAPHORE_HPP

#include "syscall_c.h"
#include "TCB.hpp"

class KernelSemaphore : public _sem {
public:
    static KernelSemaphore* createSemaphore(unsigned init);
    static void deleteSemaphore(KernelSemaphore* sem);

    int wait();
    int signal();
    int closeAll();
    int waitN(unsigned n);
    int signalN(unsigned n);

private:
    int  value; //broj niti
    bool closed;

    TCB* blockedHead; // red blokiranih niti
    TCB* blockedTail;

    void blockedPut(TCB* tcb);
    TCB*  blockedGet();
    void wakeBlocked();
};

#endif