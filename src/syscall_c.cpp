#include "../inc/syscall_c.h"
#include "../inc/MemoryAllocator.h"
#include "../inc/KernelSemaphore.hpp"
#include "../lib/hw.h"
#include "../inc/TCB.hpp"
#include "../inc/Scheduler.hpp"
#include "../inc/KernelConsole.hpp"


//   BNT (bit najvece tezine, bit 63) = 1 -> spoljasnji/softverski prekid
//   BNT = 0 -> izuzetak (ecall, ilegalna instrukcija)
static const uint64 SCAUSE_INTERRUPT_BIT = (uint64)1 << 63;   // BNT = 1
static const uint64 SCAUSE_SOFTWARE_IRQ  = SCAUSE_INTERRUPT_BIT | 1; // BNT=1, vr=1 (tajmer)
static const uint64 SCAUSE_EXTERNAL_IRQ  = SCAUSE_INTERRUPT_BIT | 9; // BNT=1, vr=9 (spoljasnji HW)
static const uint64 SCAUSE_ECALL_USER    = 8;   // BNT=0, vr=8 (ecall iz korisnickog rezima)
static const uint64 SCAUSE_ECALL_SUPER   = 9;   // BNT=0, vr=9 (ecall iz sistemskog rezima)
static const uint64 SCAUSE_ILLEGAL_INSTR = 2;   // BNT=0, vr=2 (ilegalna instrukcija)

extern "C" void consoleIOInit() {
    KernelConsole::init();
}


void* mem_alloc(size_t size) {
    if (size == 0) return nullptr;
    uint64 numBlocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;

    void* ret;
    __asm__ volatile("mv a1,%0"::"r"(numBlocks):"a1");
    __asm__ volatile("li a0,0x01":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

int mem_free(void* ptr) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(ptr):"a1");
    __asm__ volatile("li a0,0x02":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

int thread_create(thread_t* handle, void(*start_routine)(void*), void* arg) {
    void* stack = mem_alloc(DEFAULT_STACK_SIZE);
    if (!stack) return -1;

    void* stack_space = (void*)((char*)stack + DEFAULT_STACK_SIZE);

    int ret;
    __asm__ volatile("mv a1,%0"::"r"(handle):"a1");
    __asm__ volatile("mv a2,%0"::"r"(start_routine):"a2");
    __asm__ volatile("mv a3,%0"::"r"(arg):"a3");
    __asm__ volatile("mv a4,%0"::"r"(stack_space):"a4");
    __asm__ volatile("li a0,0x11":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

int thread_exit() {
    int ret;
    __asm__ volatile("li a0,0x12":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

void thread_dispatch() {
    __asm__ volatile("li a0,0x13":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
}

int sem_open(sem_t* handle, unsigned init) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(handle):"a1");
    __asm__ volatile("mv a2,%0"::"r"((uint64)init));
    __asm__ volatile("li a0,0x21":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

int sem_close(sem_t handle) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(handle):"a1");
    __asm__ volatile("li a0,0x22":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

int sem_wait(sem_t id) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(id):"a1");
    __asm__ volatile("li a0,0x23":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

int sem_signal(sem_t id) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(id):"a1");
    __asm__ volatile("li a0,0x24":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

int sem_wait_n(sem_t id, unsigned n) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(id):"a1");
    __asm__ volatile("mv a2,%0"::"r"((uint64)n));
    __asm__ volatile("li a0,0x25":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

int sem_signal_n(sem_t id, unsigned n) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(id):"a1");
    __asm__ volatile("mv a2,%0"::"r"((uint64)n));
    __asm__ volatile("li a0,0x26":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

int time_sleep(time_t t) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(t):"a1");
    __asm__ volatile("li a0,0x31":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

char getc() {
    sem_wait(KernelConsole::inItemAvailable);

    int ret;
    __asm__ volatile("li a0,0x41":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return (char)ret;
}

void putc(char c) {
    sem_wait(KernelConsole::outSpaceAvailable);
    sem_wait(KernelConsole::outMutexTail);

    __asm__ volatile("mv a1,%0"::"r"((uint64)c):"a1");
    __asm__ volatile("li a0,0x42":::"a0");
    __asm__ volatile("ecall":::"a0","a1","a2","a3","a4","memory");

    sem_signal(KernelConsole::outMutexTail);
    sem_signal(KernelConsole::outItemAvailable);
}



extern "C" uint64 handleTrap() {
    // trap.S je sacuvao ceo kontekst tekuce niti u
    // TCB::running->context (ra, sp, t*, a*, s*, sepc, sstatus) na ulasku
    // handleTrap samo obradjuje uzrok i po potrebi menja TCB::running.
    // pri povratku, trap.S vraca ceo kontekst iz TCB::running->context


    uint64 scause;
    __asm__ volatile("csrr %0, scause" : "=r"(scause));

    // argumenti sistemskog poziva su sacuvani u kontekstu tekuce niti
    TCB* cur = TCB::running;
    uint64 code = cur->context.a0;
    uint64 arg1 = cur->context.a1;
    uint64 arg2 = cur->context.a2;
    uint64 arg3 = cur->context.a3;
    uint64 arg4 = cur->context.a4;

    // tajmerski softverski prekid
    if (scause == SCAUSE_SOFTWARE_IRQ) {
        uint64 sip;
        __asm__ volatile("csrr %0, sip" : "=r"(sip));
        sip &= ~2ULL;
        __asm__ volatile("csrw sip, %0" : : "r"(sip));

        TCB::tickSleepList();

        if (TCB::timeSlice > 0) TCB::timeSlice--;

        if (TCB::timeSlice == 0) {
            TCB* old  = TCB::running;
            TCB* next = Scheduler::get();
            if (next != nullptr) {
                // kontekst stare niti je u old->context (trap.S ga sacuvao)
                Scheduler::put(old);
                TCB::running = next;
            }
            TCB::timeSlice = DEFAULT_TIME_SLICE;
        }
        return 0;
    }

    // spoljasnji hardverski prekid — tastatura
    if (scause == SCAUSE_EXTERNAL_IRQ) {
        int irq = plic_claim();
        if (irq == (int)CONSOLE_IRQ) {
            KernelConsole::handleRxInterrupt();
        }
        if (irq != 0) plic_complete(irq);
        return 0;
    }

    // ── ecall ──
    if (scause == SCAUSE_ECALL_USER || scause == SCAUSE_ECALL_SUPER) {
        // sepc je sacuvan u kontekstu; pomeri ga za 4 da ne ponovi ecall.
        cur->context.sepc += 4;

        if (code == 0x01) {
            cur->context.a0 = (uint64)MemoryAllocator::mem_alloc(arg1);
            return 0;

        } else if (code == 0x02) {
            cur->context.a0 = (uint64)MemoryAllocator::mem_free((void*)arg1);
            return 0;

        } else if (code == 0x11) {
            thread_t* handle   = (thread_t*)arg1;
            void(*body)(void*) = (void(*)(void*))arg2;
            void* arg          = (void*)arg3;
            void* stack        = (void*)arg4;

            TCB* tcb = TCB::createThread(body, arg, stack);
            if (!tcb) { cur->context.a0 = (uint64)(-1); return 0; }

            *handle = tcb;
            Scheduler::put(tcb);
            cur->context.a0 = 0;
            return 0;

        } else if (code == 0x12) {
            TCB::running->setFinished();
            TCB* next = Scheduler::get();
            if (next == nullptr) {
                *(volatile uint32*)0x100000 = 0x5555;
                return 0;
            }
            TCB::running   = next;
            TCB::timeSlice = DEFAULT_TIME_SLICE;
            return 0;

        } else if (code == 0x13) {
            TCB* old  = TCB::running;
            TCB* next = Scheduler::get();
            if (next == nullptr) return 0;
            Scheduler::put(old);
            TCB::running   = next;
            TCB::timeSlice = DEFAULT_TIME_SLICE;
            return 0;

        } else if (code == 0x21) {
            sem_t*   handle = (sem_t*)arg1;
            unsigned init   = (unsigned)arg2;
            KernelSemaphore* sem = KernelSemaphore::createSemaphore(init);
            if (!sem) { cur->context.a0 = (uint64)(-1); return 0; }
            *handle = sem;
            cur->context.a0 = 0;
            return 0;

        } else if (code == 0x22) {
            KernelSemaphore* sem = (KernelSemaphore*)arg1;
            sem->closeAll();
            KernelSemaphore::deleteSemaphore(sem);
            cur->context.a0 = 0;
            return 0;

        } else if (code == 0x23) {
            KernelSemaphore* sem = (KernelSemaphore*)arg1;
            if (sem == nullptr) { cur->context.a0 = (uint64)(-1); return 0; }
            // podrazumevana povratna vrednost ako nit blokira je 0
            // (closeAll je menja u -1 ako semafor bude zatvoren)
            cur->context.a0 = 0;
            cur->context.a0 = sem->wait();
            return 0;

        } else if (code == 0x24) {
            KernelSemaphore* sem = (KernelSemaphore*)arg1;
            if (sem == nullptr) { cur->context.a0 = (uint64)(-1); return 0; }
            cur->context.a0 = sem->signal();
            return 0;

        } else if (code == 0x25) {
            KernelSemaphore* sem = (KernelSemaphore*)arg1;
            unsigned n = (unsigned)arg2;
            if (sem == nullptr) { cur->context.a0 = (uint64)(-1); return 0; }
            cur->context.a0 = 0;
            cur->context.a0 = sem->waitN(n);
            return 0;

        } else if (code == 0x26) {
            KernelSemaphore* sem = (KernelSemaphore*)arg1;
            unsigned n = (unsigned)arg2;
            if (sem == nullptr) { cur->context.a0 = (uint64)(-1); return 0; }
            cur->context.a0 = sem->signalN(n);
            return 0;

        } else if (code == 0x31) {
            TCB* curr = TCB::running;
            if (arg1 == 0) { curr->context.a0 = 0; return 0; }
            curr->sleepTime = arg1;
            TCB::addToSleepList(curr);
            TCB* next = Scheduler::get();
            if (next == nullptr) return 0;
            TCB::running   = next;
            TCB::timeSlice = DEFAULT_TIME_SLICE;
            return 0;

        } else if (code == 0x41) {
            cur->context.a0 = (uint64)KernelConsole::getcKernel();
            return 0;

        } else if (code == 0x42) {
            KernelConsole::putcKernel((char)arg1);
            cur->context.a0 = 0;
            return 0;
        }
    }

    // ilegalna instrukcija (korisnicka nit pokusala privilegovano)
    if (scause == SCAUSE_ILLEGAL_INSTR) {
        TCB::running->setFinished();
        TCB* next = Scheduler::get();
        if (next == nullptr) {
            *(volatile uint32*)0x100000 = 0x5555;
            return 0;
        }
        TCB::running   = next;
        TCB::timeSlice = DEFAULT_TIME_SLICE;
        return 0;
    }

    return 0;
}
