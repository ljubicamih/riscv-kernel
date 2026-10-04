#include "../inc/Scheduler.hpp"

TCB* Scheduler::head = nullptr;
TCB* Scheduler::tail = nullptr;

void Scheduler::put(TCB* tcb) {
    tcb->next = nullptr;
    if (tail) tail->next = tcb;
    else head = tcb;
    tail = tcb;
}

TCB* Scheduler::get() {
    if (!head) return nullptr;
    TCB* tcb = head;
    head = head->next;
    if (!head) tail = nullptr;
    return tcb;
}
