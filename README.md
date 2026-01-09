# Simplified Linux Operating System

## Overview
This project involves the design and implementation of a feature-complete, Unix-like operating system kernel built from scratch for the 32-bit x86 architecture. 
The kernel boots from a floppy image, enters protected mode, manages hardware interrupts, and supports preemptive multitasking for user-level programs.


## Key Features

### Core Kernel Architecture
* **Hybrid Codebase:** Logic implemented in **C**, with **x86 Assembly** used for low-level hardware interactions (context switching, interrupt service routines, bootloader).
* **Protection Rings:** Enforces privilege separation between **Ring 0 (Kernel Mode)** and **Ring 3 (User Mode)** to prevent user programs from accessing restricted instructions or crashing the system.
* **Memory Management:**
    * Implemented **Virtual Memory** using 4KB paging mechanisms.
    * Maps kernel memory (0-4MB) and dynamically allocates user-space program memory (starting at 128MB).

### Process Management & Multitasking
* **Preemptive Multitasking:** Supports running up to 6 concurrent processes.
* **Scheduler:** Implemented a Round-Robin scheduler driven by the Programmable Interval Timer (PIT).
* **Context Switching:** Transparently saves and restores process state (PCB, register sets, stack pointers) during interrupts.
* **System Calls:** Implemented a POSIX-style interface including:
    * `execute`, `halt` (Process control)
    * `read`, `write`, `open`, `close` (File I/O)
    * `getargs`, `vidmap` (User interaction)

### File System & Drivers
* **Read-Only Filesystem:** Custom driver to parse inodes, data blocks, and boot blocks to load executables and text files.
* **Device Drivers:**
    * **Keyboard:** Interrupt-driven driver supporting shift states and circular buffer handling.
    * **RTC (Real-Time Clock):** Virtualized interrupt frequency control.
    * **Terminal:** Specialized driver handling standard input/output (stdin/stdout) and video memory mapping.

### Advanced Features
* **Virtual Consoles:** Supports multiple concurrent terminal sessions (Alt+F1, Alt+F2, Alt+F3) with independent video memory buffers.
* **Synchronization:** Critical sections guarded using **CLI/STI** and **spinlocks** to prevent race conditions in shared kernel data structures.

## Technical Architecture

| Component | Implementation Details |
| :--- | :--- |
| **Bootloader** | Loads kernel image into memory and initializes Protected Mode (GDT/IDT). |
| **Interrupts** | IDT initialized for hardware interrupts (IRQ 0-15) and software exceptions (0-31). |
| **Paging** | Page Directory and Page Tables set up to map virtual addresses to physical frames. |
| **Video** | VGA text mode driver with direct memory access for screen rendering. |

## Challenges & Solutions

* **Triple Faults & Debugging:** Overcame early boot crashes by mastering **GDB** with QEMU to trace stack corruption and IDT misconfigurations.
* **Race Conditions:** Solved keyboard buffer corruption during high-frequency interrupts by implementing rigorous critical sections and spinlocks.
* **User/Kernel Separation:** Successfully managed the transition between Ring 0 and Ring 3 using `IRET` and careful TSS (Task State Segment) configuration.

## Development Environment
* **OS:** Linux (CentOS/Ubuntu)
* **Compiler:** GCC
* **Debugger:** GDB
* **Emulation:** QEMU (x86 system emulation)

