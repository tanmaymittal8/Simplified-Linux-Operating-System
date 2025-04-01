#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#define PIT_CHANNEL0   0x40
#define PIT_CMD_PORT    0x43
#define PIT_FREQ    1193180
#define FREQ    50
#define COMMAND_BYTE    0x36 //this is used to set PIT to channel 0 and mode 3(square wave generator)
#define NUM_TERMS   3

#define IRQ_0   0x0

void pit_init();

void pit_handler();

extern void set_paging_scheduling(int currTerm);


extern int32_t scheduled_term; //initialize to 0 at startup(first scheluded terminal is terminal 1)
extern int oldTerm; // when scheduling a new terminal this keeps track of the terminal we are switching out off
extern int32_t parent_pid_for_term[3]; // don't really use this anymore, but was intended to store the parent pid for each terminal
extern int32_t curr_pid_for_term[3];// this array keeps track of the currently running pid for each terminal, initialized to {0,1,2}(all terminals running shells on startup) 
extern int terminalInstantiated[3];// array to keep track of the terminals which have been instantiated, used when starting up the 3 terminals on startup
// since we want to open a new shell when we switch into a new terminal

#endif /* _SCHEDULING_H */
