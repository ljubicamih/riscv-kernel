#ifndef TCB_HPP
#define TCB_HPP

#include "../lib/hw.h"
#include "syscall_c.h"

class TCB : public _thread {
public:
    enum State { READY, BLOCKED, FINISHED };

    static TCB* running;
    static uint64 timeSlice;
    static TCB* sleepList;   // ulančana lista uspavanih niti

    static TCB* createThread(void(*body)(void*), void* arg, void* stack);

    bool isFinished() const { return state == FINISHED; }
    void setFinished()      { state = FINISHED; }

    static void addToSleepList(TCB* tcb);
    // poziva se na svaki otkucaj tajmera
    static void tickSleepList();

    struct Context {
        uint64 ra;        // 0
        uint64 sp;        // 8
        uint64 t0, t1, t2, t3, t4, t5, t6;          // 16..64
        uint64 a0, a1, a2, a3, a4, a5, a6, a7;      // 72..128
        uint64 s0,  s1,  s2,  s3,  s4,  s5,
               s6,  s7,  s8,  s9,  s10, s11;        // 136..224
        uint64 sepc;      // 232
        uint64 sstatus;   // 240
    };

    Context context;
    TCB*    next;           // za red schedulera ili sleep listu
    uint64  sleepTime;      // preostali broj otkucaja
    unsigned semWaitCount; // koliko fali da se odblokira

    void*   systemStack;    // sistemski (kernel) stek ove niti
    uint64  systemStackTop; // vrh sistemskog steka

private:
    void(*body)(void*);
    void* arg;
    uint64* stack;
    State state;

    static void threadWrapper();
};

#endif