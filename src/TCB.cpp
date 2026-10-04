#include "../inc/TCB.hpp"
#include "../inc/Scheduler.hpp"
#include "../inc/MemoryAllocator.h"
#include "../inc/syscall_c.h"

TCB*   TCB::running   = nullptr;
uint64 TCB::timeSlice = DEFAULT_TIME_SLICE;
TCB*   TCB::sleepList = nullptr;

// SPP  = bit 8  (0 -> sret se vraca u USER rezim, 1 -> supervisor)
// SPIE = bit 5  (vrednost SIE posle sret-a; 1 -> prekidi omoguceni u niti)
static const uint64 SSTATUS_SPP  = (1UL << 8);
static const uint64 SSTATUS_SPIE = (1UL << 5);

TCB* TCB::createThread(void(*body)(void*), void* arg, void* stack) {
    TCB* tcb = (TCB*)MemoryAllocator::mem_alloc(sizeof(TCB));
    if (!tcb) return nullptr;

    tcb->body      = body;
    tcb->arg       = arg;
    tcb->stack     = (uint64*)stack;
    tcb->state     = READY;
    tcb->next      = nullptr;
    tcb->sleepTime = 0;
    tcb->semWaitCount = 0;

    // svaka nit dobija svoj sistemski (kernel) stek na kom se izvrsava kod jezgra
    const uint64 SYS_STACK_SIZE = 4096;
    tcb->systemStack = MemoryAllocator::mem_alloc(SYS_STACK_SIZE);
    if (!tcb->systemStack) return nullptr;
    tcb->systemStackTop = ((uint64)tcb->systemStack + SYS_STACK_SIZE) & ~((uint64)15);

    if (body != nullptr) {
        // skok u nit se radi preko sepc (sret)
        tcb->context.sepc = (uint64)threadWrapper;  // gde nit pocinje
        tcb->context.ra   = (uint64)threadWrapper;
        tcb->context.sp   = (uint64)stack;          // vrh korisnickog steka

        // korisnicka nit krece u korisnickom rezimu (SPP=0) sa omogucenim prekidima (SPIE=1).
        uint64 st;
        asm volatile("csrr %0, sstatus" : "=r"(st));
        st &= ~SSTATUS_SPP;     // SPP = 0  -> user rezim
        st |=  SSTATUS_SPIE;    // SPIE = 1 -> prekidi omoguceni kad nit krene
        tcb->context.sstatus = st;
    } else {
        // main nit vec radi (u supervisor rezimu).
        tcb->context.sepc = 0;
        tcb->context.ra   = 0;
        tcb->context.sp   = 0;
        uint64 st;
        asm volatile("csrr %0, sstatus" : "=r"(st));
        st |= SSTATUS_SPP;      // SPP = 1 -> supervisor
        st |= SSTATUS_SPIE;
        tcb->context.sstatus = st;
    }

    // nulluje sve ostale registre u contextu
    tcb->context.t0 = tcb->context.t1 = tcb->context.t2 = tcb->context.t3 = 0;
    tcb->context.t4 = tcb->context.t5 = tcb->context.t6 = 0;
    tcb->context.a0 = tcb->context.a1 = tcb->context.a2 = tcb->context.a3 = 0;
    tcb->context.a4 = tcb->context.a5 = tcb->context.a6 = tcb->context.a7 = 0;
    tcb->context.s0  = tcb->context.s1  = tcb->context.s2  = 0;
    tcb->context.s3  = tcb->context.s4  = tcb->context.s5  = 0;
    tcb->context.s6  = tcb->context.s7  = tcb->context.s8  = 0;
    tcb->context.s9  = tcb->context.s10 = tcb->context.s11 = 0;

    return tcb;
}

void TCB::threadWrapper() {
    TCB::running->body(TCB::running->arg);
    thread_exit();
}

void TCB::addToSleepList(TCB* tcb) {
    TCB** cur = &sleepList;
    while (*cur != nullptr && (*cur)->sleepTime <= tcb->sleepTime) {
        tcb->sleepTime -= (*cur)->sleepTime;
        cur = &(*cur)->next;
    }
    if (*cur != nullptr) {
        (*cur)->sleepTime -= tcb->sleepTime;
    }
    tcb->next = *cur;
    *cur = tcb;
}

void TCB::tickSleepList() {
    if (sleepList == nullptr) return;
    if (sleepList->sleepTime > 0) sleepList->sleepTime--;
    while (sleepList != nullptr && sleepList->sleepTime == 0) {
        TCB* toWake = sleepList;
        sleepList = sleepList->next;
        toWake->next = nullptr;
        Scheduler::put(toWake);
    }
}