#define ASM     1
#include "x86_desc.h"
# /* void clear(void);
#  * Inputs: void
#  * Return Value: none
#  * Function: Clears video memory */
.align 4
jump_table:
        .long 0x0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

# syscall_handler()
# Inputs: edx, ecx, and ebx which store the arguments for system calls
# Return Value: int
# Function: this the assembly handler which calls system calls. The x80 entry in the IDT points to this handler, and eax stores the number which corresponds to which system call is getting called
.global syscall_handler, handler_return_helper
.align 4
syscall_handler:

    pushfl
    pushl %ebp
    pushl %esi
    pushl %edi
    pushl %esp
    pushl %edx
    pushl %ecx
    pushl %ebx

    # push 3 registers which store the arguments
    pushl %edx
    pushl %ecx
    pushl %ebx
    # bound checking eax which is syscall number
    cmpl $0, %eax 
    jle SYSCALL_OUT_OF_BOUNDS
    cmpl $10, %eax
    jg SYSCALL_OUT_OF_BOUNDS
    # call syscall
    call *jump_table(, %eax, 4)
    addl $12, %esp          # move esp because of pushing the three registers above
    jmp DONE_HANDLER
SYSCALL_OUT_OF_BOUNDS:  
    addl $12, %esp
    movl $-1, %eax
DONE_HANDLER:

    popl %ebx
    popl %ecx
    popl %edx
    popl %esp
    popl %edi
    popl %esi
    popl %ebp
    popfl

    iret
       
