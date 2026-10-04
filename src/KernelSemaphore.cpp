#include "../inc/KernelSemaphore.hpp"
#include "../inc/Scheduler.hpp"
#include "../inc/MemoryAllocator.h"


KernelSemaphore* KernelSemaphore::createSemaphore(unsigned init) {
    KernelSemaphore* sem = (KernelSemaphore*)MemoryAllocator::mem_alloc(sizeof(KernelSemaphore));
    if (!sem) return nullptr;

    sem->value       = (int)init;
    sem->closed      = false;
    sem->blockedHead = nullptr;
    sem->blockedTail = nullptr;

    return sem;
}

void KernelSemaphore::deleteSemaphore(KernelSemaphore* sem) {
    MemoryAllocator::mem_free(sem);
}

void KernelSemaphore::blockedPut(TCB* tcb) {
    tcb->next = nullptr;
    if (blockedTail) blockedTail->next = tcb;
    else             blockedHead = tcb;
    blockedTail = tcb;
}

TCB* KernelSemaphore::blockedGet() {
    if (!blockedHead) return nullptr;
    TCB* tcb = blockedHead;
    blockedHead = blockedHead->next;
    if (!blockedHead) blockedTail = nullptr;
    return tcb;
}


int KernelSemaphore::wait() {
    return waitN(1);
}

// wait(n): trazi n zetona
// ako nema dovoljno, uzme koliko ima (v) i blokira
// cekajuci preostalih (n - v) zetona
int KernelSemaphore::waitN(unsigned n) {
    if (closed) return -1;
    if (n == 0) return 0;

    value -= (int)n;

    if (value < 0) {
        // nije bilo dovoljno: nit je "uzela" v zetona, ceka jos (-value)
        TCB* oldRunning = TCB::running;
        oldRunning->semWaitCount = (unsigned)(-value);
        value = 0;
        this->blockedPut(oldRunning);

        TCB* newRunning = Scheduler::get();
        if (newRunning == nullptr) return 0;

        TCB::running = newRunning;
    }

    return 0;
}


int KernelSemaphore::signal() {
    if (closed) return -1;

    TCB* thr = blockedHead;   // first(): celo reda bez vadjenja
    if (thr) {
        // ima blokiranih: daj zeton celu reda
        thr->semWaitCount--;
        if (thr->semWaitCount == 0) {
            // nit je dobila sve trazene zetone — odblokiraj je
            this->blockedGet();          // izvadi je iz reda
            Scheduler::put(thr);
        }
    } else {
        // nema blokiranih: samo povecaj broj zetona
        value++;
    }

    return 0;
}


int KernelSemaphore::signalN(unsigned n) {
    if (closed) return -1;
    for (unsigned i = 0; i < n; i++) {
        signal();
    }
    return 0;
}

int KernelSemaphore::closeAll() {
    closed = true;

    TCB* tcb;
    while ((tcb = this->blockedGet()) != nullptr) {
        // upisuje -1 u sacuvani a0 niti
        // da njen wait() vrati gresku kad se probudi zbog zatvaranja semafora.
        uint64* frameA0 = (uint64*)(tcb->context.sp - 256 + 128);
        *frameA0 = (uint64)(-1);
        Scheduler::put(tcb);
    }

    return 0;
}