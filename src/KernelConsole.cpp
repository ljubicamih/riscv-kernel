#include "../inc/KernelConsole.hpp"
#include "../inc/KernelSemaphore.hpp"

// izlazni bafer
char  KernelConsole::outBuf[KernelConsole::OUT_ARR];
int   KernelConsole::outHead = 0;
int   KernelConsole::outTail = 0;
sem_t KernelConsole::outItemAvailable;
sem_t KernelConsole::outSpaceAvailable;
sem_t KernelConsole::outMutexHead;
sem_t KernelConsole::outMutexTail;

// ulazni bafer
char  KernelConsole::inBuf[KernelConsole::IN_ARR];
int   KernelConsole::inHead = 0;
int   KernelConsole::inTail = 0;
sem_t KernelConsole::inItemAvailable;
sem_t KernelConsole::inMutexHead;


static void uartPutc(char c) {
    volatile uint8* status = (volatile uint8*)CONSOLE_STATUS;
    volatile uint8* txData = (volatile uint8*)CONSOLE_TX_DATA;
    while (!(*status & CONSOLE_TX_STATUS_BIT));   // cekaj dok TX nije spreman (busy-wait)
    *txData = (uint8)c;
}

// interna nit jezgra — uzima znak po znak iz izlaznog bafera
void KernelConsole::outputThreadBody(void* arg) {
    while (true) {
        sem_wait(outItemAvailable);
        sem_wait(outMutexHead);
        char c = outBuf[outHead];
        outHead = (outHead + 1) % OUT_ARR;
        sem_signal(outMutexHead);
        sem_signal(outSpaceAvailable);
        uartPutc(c);
    }
}

void KernelConsole::init() {

    sem_open(&outItemAvailable,  0);
    sem_open(&outSpaceAvailable, OUT_CAP);
    sem_open(&outMutexHead, 1);
    sem_open(&outMutexTail, 1);
    sem_open(&inItemAvailable, 0);
    sem_open(&inMutexHead, 1);

    thread_t outThread;
    thread_create(&outThread, outputThreadBody, nullptr);
}


void KernelConsole::putcKernel(char c) {
    outBuf[outTail] = c;
    outTail = (outTail + 1) % OUT_ARR;
}

char KernelConsole::getcKernel() {
    char c = inBuf[inHead];
    inHead = (inHead + 1) % IN_ARR;
    return c;
}


// posto smo u prekidnoj rutini (ne u korisnickoj niti), signal se radi DIREKTNO
// na semaforu, a ne preko ecall-a
void KernelConsole::handleRxInterrupt() {
    volatile uint8* status = (volatile uint8*)CONSOLE_STATUS;
    volatile uint8* rxData = (volatile uint8*)CONSOLE_RX_DATA;

    while (*status & CONSOLE_RX_STATUS_BIT) {
        char c = (char)*rxData;
        int next = (inTail + 1) % IN_ARR;
        if (next != inHead) {            // ima mesta u ulaznom baferu
            inBuf[inTail] = c;
            inTail = next;
            ((KernelSemaphore*)inItemAvailable)->signal();
        }
        // ako je bafer pun, znak se odbacuje (proizvodjac je hardver, ne moze da blokira)
    }
}