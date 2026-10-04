# RISC-V Kernel

A compact multithreaded OS kernel, built for the RISC-V (RV64IMA) architecture.

## Summary

Source code for a preemptive, multithreaded kernel written in C++ and RISC-V assembly, running directly on bare-metal hardware. Covers the essential OS mechanisms: memory management, context switching, thread scheduling, synchronization, and console I/O. The kernel and user application are statically linked into a single executable sharing one address space. The kernel executes in RISC-V supervisor mode, while user code stays in user mode — the only path into supervisor mode is through ecall.

## Implemented features

- **Memory management** — first-fit allocation, exposed through `mem_alloc`/`mem_free`
- **Threads** — TCB-based thread model supporting both synchronous and asynchronous context switches
-  **Scheduling** — round-robin FIFO scheduler
- **Time sharing** — threads are preempted on timer interrupts, each with a configurable time slice
- **Synchronization** — counting semaphores with blocking/unblocking, batched wait/signal operations through `sem_wait_n`/`sem_signal_n`, plus timed sleeping through `time_sleep`
- **Console I/O** — buffered `getc`/`putc`, with a dedicated kernel thread handling output
- **C API** — procedural syscall interface (`mem_alloc`, `thread_create`, `sem_wait`, `getc`)
- **C++ API** — object-oriented wrappers (`Thread`, `Semaphore`, `Console`, `PeriodicThread`)

## Structure

- `inc/` — header files for each kernel module
- `src/` — kernel module implementations, plus the trap handler written in assembly (`trap.S`)
