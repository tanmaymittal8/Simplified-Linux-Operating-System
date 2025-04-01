#include "exception_handlers.h"
#include "x86_desc.h"
#include "lib.h"
#include "assembly_handler.h"
#include "syscall_helper.h"

#define HALTCODE    255

/* void interrupt_0_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: divide by 0 error handler, print and halt program
 */
void interrupt_0_handler() {      
    printf("Divide Error Exception");  
    halt(HALTCODE);
}

/* void interrupt_1_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: debug exception handler, print and halt program
 */
void interrupt_1_handler() {      
    printf("Debug Exception");         
    halt(HALTCODE);
}

/* void interrupt_2_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: NMI non maskable interrupt handler, print and halt program
 */
void interrupt_2_handler() {      
    printf("NMI Interrupt");           
    halt(HALTCODE);
}

/* void interrupt_3_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: brakboint exception handler, print and halt program
 */
void interrupt_3_handler() {      
    printf("Breakpoint Exception");    
    halt(HALTCODE);
}

/* void interrupt_4_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: overflow exception handler, print and halt program
 */
void interrupt_4_handler() {      
    printf("Overflow Exception");      
    halt(HALTCODE);
}

/* void interrupt_5_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: BOUND Range Exceeded Exception handler, print and halt program
 */
void interrupt_5_handler() {      
    printf("BOUND Range Exceeded Exception");
    halt(HALTCODE);
}

/* void interrupt_6_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: Invalid Opcode Exception handler, print and halt program
 */
void interrupt_6_handler() {      
    printf("Invalid Opcode Exception");
    halt(HALTCODE);
}

/* void interrupt_7_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: Device Not Available Exception handler, print and halt program
 */
void interrupt_7_handler() {      
    printf("Device Not Available Exception");
    halt(HALTCODE);
}

/* void interrupt_8_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: Double Fault Exception handler, print and halt program
 */
void interrupt_8_handler() {      
    printf("Double Fault Exception");  
    halt(HALTCODE);
}

/* void interrupt_9_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: Coprocessor Segment Overrun handler, print and halt program
 */
void interrupt_9_handler() {      
    printf("Coprocessor Segment Overrun");
    halt(HALTCODE);
}

/* void interrupt_10_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: Invalid TSS Exception handler, print and halt program
 */
void interrupt_10_handler() {     
    printf("Invalid TSS Exception");   
    halt(HALTCODE);
}

/* void interrupt_11_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: Segment Not Present handler, print and halt program
 */
void interrupt_11_handler() {     
    printf("Segment Not Present");     
    halt(HALTCODE);
}

/* void interrupt_12_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: Stack Fault Exception handler, print and halt program
 */
void interrupt_12_handler() {     
    printf("Stack Fault Exception");   
    halt(HALTCODE);
}

/* void interrupt_13_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: General Protection Exception handler, print and halt program
 */
void interrupt_13_handler() {     
    printf("General Protection Exception");
    halt(HALTCODE);
}

/* void interrupt_14_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: Page-Fault Exception handler, print and halt program
 */
void interrupt_14_handler() {     
    printf("Page-Fault Exception\n");    
    halt(HALTCODE);
}

// Interupt 15 reserved by Intel

/* void interrupt_16_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: x87 FPU Floating-Point Error handler, print and halt program
 */
void interrupt_16_handler() {     
    printf("x87 FPU Floating-Point Error");
    halt(HALTCODE);
}

/* void interrupt_17_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: Alignment Check Exception handler, print and halt program
 */
void interrupt_17_handler() {     
    printf("Alignment Check Exception");
    halt(HALTCODE);
}

/* void interrupt_18_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: Machine Check handler, print and halt program
 */
void interrupt_18_handler() {     
    printf("Machine Check");           
    halt(HALTCODE);
}

/* void interrupt_19_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: SIMD Floating-Point Exception handler, print and halt program
 */
void interrupt_19_handler() {     
    printf("SIMD Floating-Point Exception");
    halt(HALTCODE);
}


/* void initialize_idt();
 * Inputs: nothing
 * Return Value: none
 * Function: initializes the idt
 */
void initialize_idt()
{
    //use the SET_IDT_ENTRY function
    SET_IDT_ENTRY(idt[0], &interrupt_0_handler); // set the divide 0 exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[1], &interrupt_1_handler); // set the debug exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[2], &interrupt_2_handler); // set the NMI exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[3], &interrupt_3_handler); // set the breakpoint exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[4], &interrupt_4_handler); // set the overflow exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[5], &interrupt_5_handler); // set the bound range exceeded exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[6], &interrupt_6_handler); // set the invalid opcode exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[7], &interrupt_7_handler); // set the device not availaible exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[8], &interrupt_8_handler); // set the double fault exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[9], &interrupt_9_handler); // set the Coprocessor Segment Overrun exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[10], &interrupt_10_handler); // set the Invalid TSS exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[11], &interrupt_11_handler); // set the Segment Not Present exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[12], &interrupt_12_handler); // set the Stack Fault Exception exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[13], &interrupt_13_handler); // set the General Protection Exception exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[14], &interrupt_14_handler); // set the Page-Fault Exception exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[16], &interrupt_16_handler); // set the x87 FPU Floating-Point Error exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[17], &interrupt_17_handler); // set the Alignment Check exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[18], &interrupt_18_handler); // set the Machine Check exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[19], &interrupt_19_handler); // set the SIMD Floating-Point exception vector number to its corresponding handler
    SET_IDT_ENTRY(idt[32], &pit_translated); // idt value for pit is 0x20(since its connected to irq0)
    SET_IDT_ENTRY(idt[33], &keyboard_translated); // idt value for keyboard is 0x21 irq 1 ie 33 in decimal
    SET_IDT_ENTRY(idt[128], &syscall_handler);   //system call is 0x80 in hex for vector number

    //rtc_translated
    SET_IDT_ENTRY(idt[40], &rtc_translated); // idt value for rtc is 0x28 irq 8 so decimal value 40
    //setup looping variable
    int i;
    for(i = 0;i<20;i++){    // setting values for exceptions which use trap gates which are 0-19
        if(i != 15){        //skip 15 since it says so in ISA
            idt[i].seg_selector = KERNEL_CS;
            idt[i].reserved4 = 0; // trap gate setup decides these reserved values
            idt[i].reserved3 = 1;
            idt[i].reserved2 = 1;
            idt[i].reserved1 = 1;
            idt[i].reserved0 = 0;
            //set size to 1 for 32 bits
            idt[i].size = 1;
            //high privelege
            idt[i].dpl = 0;
            //present to 1 since we are setting these
            idt[i].present = 1;
        }
    }
    for(i = 32;i<NUM_VEC;i++){  // setting values for interrupts that use use interrupt gates which are those 32 and after
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0; // interrupt gates setup decides these reserved values
        idt[i].reserved3 = 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved0 = 0;
        //set size to 1 for 32 bits
        idt[i].size = 1;
        //high privelege
        idt[i].dpl = 0;
        //present to 0 since we are not setting most of these
        idt[i].present = 0;
        if(i == 128){           // idt specific initalizations for the system call handler which is vextor 0x80=128
            idt[i].dpl = 3;     // setup specific for the system call privelege
            idt[i].present = 1;
        }
        if(i == 32){
            idt[i].dpl = 0;     //idt specific initalizations for the pit which is irq0 
            idt[i].present = 1;
        }
        if(i == 33){            // idt specific initalizations for the keyboard which is irq1 = 0x21=33
            idt[i].dpl = 0;     // setup specific for the keyboard privelege
            idt[i].present = 1;
        }
        if(i == 40){            // idt specific initalizations for the rtc which is irq8 = 0x28 = 40
            idt[i].dpl = 0;     // setup specific for the rtc privelege
            idt[i].present = 1;
        }
    }

    lidt(idt_desc_ptr);     // used to load idt descripter pointer
}
