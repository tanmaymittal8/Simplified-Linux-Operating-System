#include "systemCall.h"
#include "file_systems.h"
#include "rtc.h"
#include "terminal.h"
#include "types.h"
#include "paging.h"
#include "x86_desc.h"
#include "page_enable.h"
#include "scheduling.h"
#include "keyboard.h"

int32_t parent_pid_for_term[3] = {0,1,2};
int32_t curr_pid_for_term[3]= {0,1,2};
int scheduled_term = 0;
int terminalInstantiated[3] = {0,0,0};
int doneInstantiating=0;

//http://www.osdever.net/bkerndev/Docs/pit.htm
/* void PIT_init();
 * Inputs: nothing
 * Return Value: none
 * Function: initializes pit to channel 0 and mode 3. Sets the frequency to 50 hz, and connects pit to irq0 on the pic
 */
void pit_init(){
    uint32_t divisor = PIT_FREQ/FREQ; //calculate divisor value based on frequency(50 for now which is a 20 millisecond period)

    outb(COMMAND_BYTE, PIT_CMD_PORT);//sets PIT to channel 0 and mode 3(square wave generator)

    uint8_t low_byte = (uint8_t)(divisor & 0xff);
    uint8_t high_byte = (uint8_t)((divisor >> 8) & 0xff);
    
    outb(low_byte, PIT_CHANNEL0); //write low and high bytes of divisor to port 0x40 (channel 0)
    outb(high_byte, PIT_CHANNEL0);
    
    enable_irq(IRQ_0); //PIT is connected to irq 0 on the pic 
}

/* void set_paging_scheduling(int currTerm){
 * Inputs: nothing
 * Return Value: none
 * Function: sets the video memory baseed on the scheduled and visible terminal, modifies the video memory page table and the page table for vidmap
 */
void set_paging_scheduling(int currTerm){
    //check if terminal where this process is running is visible or not
    if(scheduled_term == currTerm-1){ //cuz currentTerminal isn't 0 indexed
        video_page_table[0].p_address = VIDEO/FourKB; //assign the pointer for the first page in the video page table to point to the video memory in physical memory, if the terminal scheduled is being displayed
        page_table[VIDEO/FourKB].p_address = VIDEO/FourKB;
    }
    //if scheduled terminal isn't visible then we have to reassign the video memory paging and the vidmap paging
    else{
        video_page_table[0].p_address = (VIDEO+(scheduled_term+1)*FourKB)/FourKB; //write to a page that isn't being displayed(terminals are 1 indexed, basically visible_term can be 1,2,3 not 0,1,2)
        page_table[VIDEO/FourKB].p_address = (VIDEO+(scheduled_term+1)*FourKB)/FourKB;
    }
    tlbFlush();
}


/* void pit_handler(){
 * Inputs: nothing
 * Return Value: none
 * Function: This function starts up the base shells for terminals 2 and 3, and at each consecutive pit interrupt handles scheduling(scheduling and jumping to a new program)
 */
void pit_handler(){
    send_eoi(IRQ_0);
    cli();
    //save terminal we are switching from
    int oldTerm = scheduled_term;

    scheduled_term = ((scheduled_term+1)%NUM_TERMS);//getting the next scheduled terminal
    
    //find the pcb we are leaving from
    struct PCB_t * leavingPCB = getPCB(curr_pid_for_term[oldTerm]); 
    uint32_t s_ebp;
    uint32_t s_esp;
    // save ebp and esp for the process we are about to leave(this is the ebp and esp we will reload when this process is scheduled again)
    asm("movl %%esp, %0     \n\
        movl %%ebp, %1     \n\
        "
        :"=r"(s_esp), "=r"(s_ebp)
        );
    leavingPCB->scheduledEbp = s_ebp;        // set ebp
    leavingPCB->scheduledEsp = s_esp;        // set esp

    set_paging_scheduling(currentTerminal);  // set the paging up for video memory based on the scheduled terminal
    // we run terminals 2 and 3 on the first and second pit interrupts
    // executes the second terminal(the first terminal executes on startup in kernel.c)
    if(scheduled_term == 1 && terminalInstantiated[1] == empty){
        //set the old terminal choice to the current, update the current to 2
        recentTermChoice[0] = recentTermChoice[1];
        recentTermChoice[1] = 2;
        terminal_changing_handler();
        //clear so we can get color correct
        clear();
        //set terminal as instantiated and execute
        terminalInstantiated[1] = 1;
        execute((const uint8_t*) "shell");
    }
    // executes the third terminal
    else if(scheduled_term == 2 && terminalInstantiated[2] == empty){
        //set the old terminal choice to the current, update the current to 3
        recentTermChoice[0] = recentTermChoice[1];
        recentTermChoice[1] = 3;
        terminal_changing_handler();
        //clear so we can get color correct
        clear();
        //set terminal as instantiated and execute
        terminalInstantiated[2] = 1;
        execute((const uint8_t*) "shell");
        
    }
    else{
        // makes sure we switch into the first terminal on startup(after first 2 pit interrupts) to actually allow the user to run commands in the first terminal 
        if(doneInstantiating ==0){
            //set the old terminal choice to the current, update the current to 1
            recentTermChoice[1] = 1;
            recentTermChoice[0] = 3;
            terminal_changing_handler();
            doneInstantiating = 1;
        }
        int32_t next_pid = curr_pid_for_term[scheduled_term]; //gets the currently running pid for the scheduled terminal

        pid = next_pid; //we set our pid variable here, since pid is a global variable that specifies which process is currently scheduled to execute
        struct PCB_t * next_pcb = getPCB(next_pid); //get next pcb 

        page_directory[USR_MEM_IDX].MB.pt_address = (MB8 + MB4*next_pid) >> 22;//assign virtual memory at 128 mb to physical memory of 8mb + 4mb * next_pid(this is where we will store our files in physical memory)
        tlbFlush();
        tss.esp0 = (MB8 - KB8*(next_pid)); // update tss for context switching
        tss.ss0 = KERNEL_DS;
        uint32_t n_ebp = next_pcb->scheduledEbp; //the scheduled ebp and esp are saved in the pit_handler whenever a pit interrupt occurs
        uint32_t n_esp = next_pcb->scheduledEsp;
        //reload ebp and esp in order to jump to the next scheduled process
        asm volatile("  movl %0, %%esp  \n\
                        movl %1, %%ebp  \n\
                        leave           \n\
                        ret             \n\
        "
        :
        :"r"(n_esp), "r"(n_ebp)
        : "esp", "ebp"
        );  
    }
}


