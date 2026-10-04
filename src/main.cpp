#include "../lib/hw.h"
#include "../inc/MemoryAllocator.h"
#include "../inc/TCB.hpp"
#include "../inc/syscall_c.h"

extern "C" void supervisorTrap();
extern "C" void consoleIOInit();
extern void userMain();

// svaka nit ima svoj sistemski (kernel) stek — alocira se u TCB::createThread.

static sem_t userMainDone;

static void userMainWrapper(void* arg) {
    userMain();
    sem_signal(userMainDone);
}

// idle nit — garantuje da red rasporedjivaca nikad nije prazan
static void idleThreadBody(void* arg) {
    while (true) {
        thread_dispatch();
    }
}

int main() {
    MemoryAllocator::init();

    uint64 stvecVal = (uint64)supervisorTrap;
    __asm__ volatile("csrw stvec, %0" : : "r"(stvecVal));

    // isprazni zaostale karaktere iz bafera
    volatile uint8* status = (volatile uint8*)CONSOLE_STATUS;
    volatile uint8* rxData = (volatile uint8*)CONSOLE_RX_DATA;
    while (*status & CONSOLE_RX_STATUS_BIT) { (void)*rxData; }

    // SSIE (tajmer, bit 1) + SEIE (spoljasnji/UART prekidi, bit 9)
    uint64 sie = (1 << 1) | (1 << 9);
    __asm__ volatile("csrw sie, %0" : : "r"(sie));

    TCB* mainThread = TCB::createThread(nullptr, nullptr, nullptr);
    TCB::running = mainThread;

    // sscratch = systemStackTop tekuce (main) niti
    __asm__ volatile("csrw sscratch, %0" : : "r"(mainThread->systemStackTop));


    consoleIOInit();

    // globalno omogucava prekide
    uint64 sstatus;
    __asm__ volatile("csrr %0, sstatus" : "=r"(sstatus));
    sstatus |= (1 << 1);
    __asm__ volatile("csrw sstatus, %0" : : "r"(sstatus));

    sem_open(&userMainDone, 0);

    thread_t idleHandle;
    thread_create(&idleHandle, idleThreadBody, nullptr);

    thread_t userMainHandle;
    thread_create(&userMainHandle, userMainWrapper, nullptr);

    sem_wait(userMainDone);

    for (volatile int i = 0; i < 100000; i++) { thread_dispatch(); }

    *(volatile uint32*)0x100000 = 0x5555;
    return 0;
}