#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "TCB.hpp"

class Scheduler {
public:
    static void put(TCB* tcb);
    static TCB* get();
private:
    static TCB* head;
    static TCB* tail;
};

#endif