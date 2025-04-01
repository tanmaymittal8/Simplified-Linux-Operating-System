#ifndef _EXCEPTION_HANDLERS
#define _EXCEPTION_HANDLERS

//definition of the c functions to be set into the idt for exceptions and system calls. We print the exception/vector and then squash the program
void systemCall_handler();
void interrupt_0_handler();
void interrupt_1_handler();
void interrupt_2_handler();
void interrupt_3_handler();
void interrupt_4_handler();
void interrupt_5_handler();
void interrupt_6_handler();
void interrupt_7_handler();
void interrupt_8_handler();
void interrupt_9_handler();
void interrupt_10_handler();
void interrupt_11_handler();
void interrupt_12_handler();
void interrupt_13_handler();
void interrupt_14_handler();
void interrupt_16_handler();
void interrupt_17_handler();
void interrupt_18_handler();
void interrupt_19_handler();

//function to be called in kernel to enable the idt
void initialize_idt();


#endif /* _EXCEPTION_HANDLERS */
