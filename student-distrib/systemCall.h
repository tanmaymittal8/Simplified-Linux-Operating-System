#ifndef _SYSTEMCALL
#define _SYSTEMCALL

#include "types.h"
#define MAX_PCB 6           //only 2 pcbs for checkpoint 3
#define empty 0
#define full 1
#define buffersize 40000
#define MB8 0x800000
#define MB4 0x400000
#define USR_MEM_IDX 32
#define VID_PAGE_IDX 33
#define USR_MEM_VIRT_ADDR 0x8000000
#define VIDEO 0xB8000
#define PROGRAM_IMG 0x08048000
#define KB8 8192
#define elf0 0x7f
#define elf1 0x45
#define elf2 0x4c
#define elf3 0x46

extern int32_t handler_return_helper(uint32_t status_int, uint32_t p_ebp, uint32_t p_esp);  // Function set in syscall_helper.S
extern int32_t halt (uint8_t status);
extern int32_t execute (const uint8_t* command);
extern int32_t read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open (const uint8_t* filename);
extern int32_t close (int32_t fd);
extern int32_t getargs (uint8_t* buf, int32_t nbytes);
extern int32_t vidmap (uint8_t** screen_start);
extern int32_t set_handler (int32_t signum, void* handler);
extern int32_t sigreturn (void);
extern struct PCB_t* getPCB(int pid);
extern int parent_pid;
extern int32_t pcb_arr[MAX_PCB];
extern int pid;

#endif /* _SYSTEMCALL */
