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
#define buffersize 40000

int32_t parent_pid = 0;//keep track of parent pid
int32_t pcbAlloc = (int32_t)(2*kernel4MB - 2*FourKB);
struct PCB_t *curr_pcb = (struct PCB_t*)(2*kernel4MB - 2*FourKB);   // calculate the starting address of pcb ie 8MB - 8KB if pid is 0
int32_t pid = 0;
//global buffer to hold arguments
uint8_t args[BUFSIZE];

int32_t pcb_arr[MAX_PCB] = {0,0,0,0,0,0}; //declare a pcb array to keep track of currently allocated PCBs on the kernel stack

struct PCB_t* getPCB(int inpid){
    return (struct PCB_t *)(MB8 - KB8*(inpid+1));
}
/* int32_t halt(uint8_t status)
 * Inputs: 
 * Return Value: The specified value to the parent process
 * Function: Terminates a process by restoring and clearing relevant fields
 */
int32_t halt(uint8_t status) {
    cli();
    //in case enter was typed during program execution reset the flag to 0
    _enter_flag[scheduled_term] = 0;

    //if we are halting the original process(shell) we want to keep running it, so in this if block we just iret(similar to the iret in execute)
    //where we jump back to the user memory corresponding to the original process(to re-execute it essentially)
    curr_pcb = getPCB(curr_pid_for_term[scheduled_term]);
    if(curr_pid_for_term[scheduled_term] == 0 || curr_pid_for_term[scheduled_term] == 1 || curr_pid_for_term[scheduled_term] == 2) {
        uint32_t eip_addr = curr_pcb->eip;
        uint32_t esp_addr = USR_MEM_VIRT_ADDR + MB4 - sizeof(uint32_t);
        asm volatile("cli       \n\
            pushl %0            \n\
            pushl %1            \n\
            pushfl              \n\
            popl %%ebx          \n\
            orl $0x200, %%ebx   \n\
            pushl %%ebx         \n\
            pushl %2            \n\
            pushl %3            \n\
            "
            :
            :"r"(USER_DS), "r"(esp_addr), "r"(USER_CS), "r"(eip_addr)
            :"eax", "ebx", "cc","memory"
        );
        asm volatile("iret");
        return 0; // This should not be reached
    }

    // Updating scheduling variables
    pcb_arr[curr_pid_for_term[scheduled_term]] = 0;
    curr_pid_for_term[scheduled_term] = parent_pid_for_term[scheduled_term]; //modify the book keeping arrays(change the current pid to be the parent pid for that terminal)
    
    // Restore parent data, context switching, similar to task_struct
    struct PCB_t *parent_pcb = getPCB(parent_pid_for_term[scheduled_term]);//get the parent pcb
    parent_pid_for_term[scheduled_term] = parent_pcb->parent; //reassign parent_pid
    
    //restore parent paging
    page_directory[USR_MEM_IDX].MB.pt_address = (MB8 + MB4*curr_pid_for_term[scheduled_term])>>22; //assign virtual memory at 128 mb to physical memory of 8mb + 4mb * parent_pid(this is where we will store our files in physical memory)
    tlbFlush(); // uWu

    // Close relevant FDs
    // Call close (do not need to clear because marked as closed is closed from the processor perspective) 
    curr_pcb->fdArray[0].jump_table.closeFunc(0);
    curr_pcb->fdArray[1].jump_table.closeFunc(1);
    int i;

    // Eight files including stdin and out so we set the stds initially then here we set files 3-8
    for(i = 2; (i < 8) && (curr_pcb->openIndexes[i - 2] != 0); i++){
        curr_pcb->fdArray[i].jump_table.closeFunc(i);
    }

    // Write parent process' info back into TSS (esp0)
    tss.esp0 = (MB8 - KB8*(curr_pid_for_term[scheduled_term]));
    tss.ss0 = KERNEL_DS;
    uint32_t p_ebp = parent_pcb->ebp;
    uint32_t p_esp = parent_pcb->esp;
   
   //set status properly based on if we are ending by exception or not
    uint32_t status_int32;
    if(status ==255){
        status_int32 = 256;
    }
    else{
        status_int32 = status;
    }
    
    // jump back to execute return(this is where we return to right before we do the iret context in execute)
    asm volatile("  movl %0, %%eax     \n\
                    movl %1, %%esp \n\
                    movl %2, %%ebp \n\
                    leave       \n\
                    ret                          \n\
    "
    :
    :"r"(status_int32), "r"(p_esp), "r"(p_ebp)
    : "eax", "esp", "ebp"
    );

    return -1; //should never be reached
}


/* int32_t execute(const uint8_t* command)
 * Inputs: Takes in the command
 * Return Value:  returns -1 representing an error with execute and 0 to represent sucess
 * Function: Tries to load and execute a new program by giving the
 * processor to the new program until it terminates
 */
int32_t execute(const uint8_t* command) {
    cli();
    if(command == NULL) { // if command is NULL return -1
        return -1;
    }
    int i; //setup a looping variable
    //initialize a buffer for the command like ls
    uint8_t function[FILENAME_LEN];

    //get first word from command our "function"----------------------------------------------------
    for(i=0;i<FILENAME_LEN;i++){ // loop through the function array and initalize it to null terminators
        function[i] = '\0';
    }
    //flag to check if we encountered the space
    int spaceFound = 0;
    if((command[strlen((const int8_t*)command)-1]) == '\n' ){ // check if command has \n command avaliable
        for(i=0;i<FILENAME_LEN;i++){                        // loop through the function array
            if(i >= strlen((const int8_t*)command)-1){      // if i larger than command size
                break;                                      // exit the loop
                function[i] = '\0';                         // set end of file name tag 
            }
            else if(spaceFound ==0){                        
                if(i< strlen((const int8_t*)command) && (command[i] == ' ' || command[i] == '\n')){     // check if i less than size of command and space or \n command found
                    spaceFound = 1;     // set spaceFound to 1
                    function[i] = '\0'; // put end of file name tag
                }
                else{
                    function[i] = command[i];   // else copy command char into function
                }
            }
            else{
                function[i] = '\0';     // set end of file char
            }
        }
    }
    else{   // if file name dosen't have \n at the end of the file
        for(i=0;i<FILENAME_LEN;i++){    // loop through the function array
            if(spaceFound ==0){
                if(i< strlen((const int8_t*)command) && (command[i] == ' ' || command[i] == '\n')){   // check if i less than size of command and space or \n command found
                    spaceFound = 1;    // set spaceFound to 1
                    function[i] = '\0';  // put end of file name tag
                }
                else{
                    function[i] = command[i];   // else copy command char into function
                }
            }
            else{
                function[i] = '\0';   // set end of file char
            }
        }
    }
    for(i=0;i<BUFSIZE;i++){            //loop through the args buffer and set it to all null terminators
        args[i] = '\0';
    }
    //variable to find index of space
    int space = -1;
    for(i=0;i<strlen((int8_t*)command);i++){    // loop through the function array
            if(command[i] ==' '){
                space = i;                      //find the index of the first space
            }
    }
    if(space != -1){                            //if a space was found
        for(i = space+1; i <strlen((int8_t*)command);i++){
            if((i-(space+1))<BUFSIZE){
                args[i-(space+1)] = command[i];    //set args to be everything read from the buffer after the space
            }  
        }
    }

    //checking validity and if executable-----------------------------------------------------------
    int valid = 0;
    struct dentry_t currentDentry;
    valid = read_dentry_by_name(function, &currentDentry);  // populate currDentry with the file attributes
    if(valid == -1){
        return -1;          //retrun if invalid
    }

    int check = -1;
    // if we are at the max amount of programs return -1 and print that we can't open anything else
    for(i=0;i<MAX_PCB;i++){
        if(pcb_arr[i] == 0){
            check =1;
        }
    }
    if(check == -1){
        printf("Can't Open Any More Files\n");          //if nothing is empty return -1
        return 0;
    }

    uint8_t buf[4]; // size of buffer greater than file length
    int offset = 0;
    valid = read_data(currentDentry.inode_num, offset, buf, 4); // read the elf values from the file
    if(valid == -1){
        //return -1 if some issue reading data
        return -1;
    }

    if(buf[0] != elf0 || buf[1] != elf1 || buf[2] != elf2 || buf[3] != elf3){ // check if elif values are differend from the ones we read from the files using read_data
        return -1;  //not an executable, return -1
    }
    //find a new open pid we can fill
    int newest_pid;
    int temp_pid = -1;
    for(i=0; i<MAX_PCB; i++){
        if(pcb_arr[i] == 0){
            newest_pid = i;
            temp_pid = i;
            pcb_arr[i] = 1; //1 indicates that the pcb with pid of i is allocated on the stack
            break;
        }
    }
    if(temp_pid == -1){
        return -1;//already allocated the max number of PCBs on the kernel stack so we return -1(execute failed)
    }

    //used for scheduling to keep track of most recently running processes running in each terminal
    parent_pid_for_term[currentTerminal-1] = curr_pid_for_term[currentTerminal-1];
    curr_pid_for_term[currentTerminal-1] = newest_pid;
    
    //set up paging---------------------------------------------------------------------------------
    page_directory[USR_MEM_IDX].MB.pt_address = (MB8 + MB4*newest_pid)>>22; //assign virtual memory at 128 mb to physical memory of 8mb + 4mb * pid(this is where we will store our files in physical memory)

    tlbFlush();     //flush since we changed address
    //load file into memory-------------------------------------------------------------------------
    uint8_t * program_image = (uint8_t *)PROGRAM_IMG; //program_img buffer is points to the program image stored in virtual memory address 0x08048000
    valid = read_data(currentDentry.inode_num,0, program_image,0x400000); // file offset set to 0 and the data size is 0x400000
    if(valid == -1){
        return -1;
    }

    //create pcb------------------------------------------------------------------------------------
    curr_pcb = getPCB(newest_pid); // assign stack location to curr_pcb
    for (i=0;i<6;i++){          // loop through openIndexes
        curr_pcb->openIndexes[i] = 0;   // initalize openIndexes array
    }
    for(i=2; i<8; i++){     // loop through the fd array of size 8 as specified in documentation
        curr_pcb->fdArray[i].flags = empty;     // initalize fd array to be empty
    }

    //initialize pcb--------------------------------------------------------------------------------
    //if we are not a base shell
    if(newest_pid != 0 && newest_pid != 1 && newest_pid != 2){
        curr_pcb->parent = parent_pid_for_term[currentTerminal-1]; //set parent attribute to parent_pid
    }
    else{
        curr_pcb->parent = newest_pid; //if current pid is 0 parent_pid is also 0(no parents yet, just one PCB allocated currently)
    }

    curr_pcb->active = 1;   // assign active to 1 to suggest pcb is being assigned
    curr_pcb->id = newest_pid;     // assign pid
    curr_pcb->fdArray[0].flags = full;  // set fd 0 flag to full indicatin stdin
    curr_pcb->fdArray[0].inode_check = 0;   // inode set to 0 as its irrelevent
    curr_pcb->fdArray[0].jump_table.openFunc =terminal_open;    // assign terminal_open to jumptable open function for fd index 0
    curr_pcb->fdArray[0].jump_table.closeFunc =terminal_close;  // assign terminal_close to jumptable close function for fd index 0
    curr_pcb->fdArray[0].jump_table.readFunc =terminal_read;   // assign terminal_read to jumptable read function for fd index 0
    curr_pcb->fdArray[0].jump_table.writeFunc =terminal_write;            // assign null to jumptable write function for fd index 0 as fd 0 dosen't allow write capabilities

    curr_pcb->fdArray[1].flags = full;  // set fd 1 flag to full indicatin stdout
    curr_pcb->fdArray[1].inode_check = 0;  // inode set to 0 as its irrelevent
    curr_pcb->fdArray[1].jump_table.openFunc =terminal_open;  // assign terminal_open to jumptable open function for fd index 1
    curr_pcb->fdArray[1].jump_table.closeFunc =terminal_close; // assign terminal_close to jumptable close function for fd index 1
    curr_pcb->fdArray[1].jump_table.readFunc =terminal_read;   // assign null to jumptable read function for fd index 1 as fd 1 dosen't allow write capabilities
    curr_pcb->fdArray[1].jump_table.writeFunc =terminal_write; // assign terminal_write to jumptable write function for fd index 1 

    
    //TSS stuff for contex switching  --------------------------------------------------------------
    tss.esp0 = (MB8 - KB8*(newest_pid)); //set esp 0 equal to the bottom of the stack for the most recently allocated pcb
    tss.ss0 = KERNEL_DS;
    //IRET context----------------------------------------------------------------------------------
    uint8_t eip_buffer[4];
    eip_buffer[0] = program_image[24];  // store the 24th bit of loaded executable for eip to jump to
    eip_buffer[1] = program_image[25];  // store the 25th bit of loaded executable for eip to jump to
    eip_buffer[2] = program_image[26];  // store the 26th bit of loaded executable for eip to jump to
    eip_buffer[3] = program_image[27];  // store the 27th bit of loaded executable for eip to jump to
    
    uint32_t eip_addr = *((uint32_t*)eip_buffer); // assign the eip_addr to the eip_buffer pointer first index
    uint32_t esp_addr = USR_MEM_VIRT_ADDR + (MB4) - sizeof(uint32_t);  // calculate the esp address

    struct PCB_t * parent_pcb = getPCB(curr_pcb->parent);
    
    curr_pcb->eip = eip_addr;   // set the eip address to eip element in pcb
    curr_pcb->esp_program_start = esp_addr; // set the esp_addr to the esp element in pcb
    //if not a base shell
    if(newest_pid != 0 && newest_pid != 1 && newest_pid != 2){
        uint32_t ebp;
        uint32_t esp;
        //store ebp and esp to allow to jump back to parent process in halt
        asm("movl %%esp, %0     \n\
            movl %%ebp, %1     \n\
            "
            :"=r"(esp), "=r"(ebp)
            );
        parent_pcb->ebp = ebp;        // set ebp
        parent_pcb->esp = esp;
    }
    // the assembly code is the iret contex to ensure that after we finish execute command we go back to the user code switch privilige levels and start reading 
    // user level code from the write address in the stack. It is initallized as volatile since it can change multiple times by multiple processes hapening concurrently
    asm volatile("cli       \n\
        pushl %0            \n\
        pushl %1            \n\
        pushfl              \n\
        popl %%ebx          \n\
        orl $0x200, %%ebx   \n\
        pushl %%ebx         \n\
        pushl %2            \n\
        pushl %3            \n\
        "
        :
        :"r"(USER_DS), "r"(esp_addr), "r"(USER_CS), "r"(eip_addr)
        :"eax", "ebx", "cc","memory"
    );
    asm volatile("iret"); 
    return 0; 
}

/* int32_t read(int32_t fd, void* buf, int32_t nybtes)
 * Inputs: fd value, buffer array to store data and nbytes to tell the total number of bytes we want read
 * Return Value:  
 * Function: Reads data from a file, device (RTC, keyboard), or directory 
 */
int32_t read(int32_t fd, void* buf, int32_t nybtes) {
    //stdin cant read so return if we try to
    if(fd == 1){
        return -1; 
    }
    if(buf == NULL) { // check if there is any data given to be written
        return -1;
    }
    if (fd < 0 || fd >7){   // check if fd value is within range
        return -1;
    }
    sti();
    curr_pcb = (struct PCB_t*)(MB8 - (KB8)*(curr_pid_for_term[scheduled_term] + 1));    // find memory space for pcb

    int out_data;
    if(curr_pcb->fdArray[fd].flags == full){    // check if the given fd index has been filled with file data
        
        out_data = curr_pcb->fdArray[fd].jump_table.readFunc(fd, buf, nybtes);  // call the jump table stored in the fd array to read the contents of the file associated with the given fd
        return out_data; // returns the total number of bytes read
    }


    return -1; // error case

}

/* int32_t write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd value, buffer to store data and nbytes ie the total # of bytes we want to read
 * Return Value: -1 if there is an error or the number of data bytes written if sucessful
 * Function: Write data to the terminal or rtc
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    if(buf == NULL) { // check if there is any data given to be written
        return -1;
    }
    sti();
    if (fd <= 0 || fd >7){   // check if fd is within range
        return -1;
    }
    curr_pcb = (struct PCB_t*)(MB8 - (KB8)*(curr_pid_for_term[scheduled_term] + 1));  // find space for pcb on the stack add 1 to pid as pid starts at 0 rather than 1
    int data_out;
    if(curr_pcb->fdArray[fd].flags == full){    // check if the desired file has been opened and loaded into the passed fd
        data_out = curr_pcb->fdArray[fd].jump_table.writeFunc(fd, buf, nbytes);     // use the jumptable from the fd index to call the write function
        return data_out; // return the # of bytes read if a write terminal triggered, 0 if rtc got triggered sucessfuly and -1 if a file or directory was called to write
    }
    printf("\n");
    return -1; // error case
}


/* int32_t open(uint8_t* filename)
 * Inputs: enter the filename
 * Return Value: returns -1 on failure and the fd # assigned the file data upon sucess
 * Function: open the file provided and populates an empty fd with the file attributes and data
 */
int32_t open(const uint8_t* filename) {
    if(filename == NULL){   // check if filename not null
        return -1;
    }
    
    struct dentry_t dentry;
    int valid;
    valid = read_dentry_by_name(filename, &dentry); // populate dentry struct with the fiven file name data like inode, file position and so on
    if(valid == -1){
        return -1;
    }
    curr_pcb = (struct PCB_t*)(MB8 - (KB8)*(curr_pid_for_term[scheduled_term] + 1));
    // find an open index on the pcb
    int openindex = -1;
    int i;
    for(i=0;i<6;i++){ // check for an open fd
        if(curr_pcb->openIndexes[i] == empty ){  // openIndexes is a tracking array for us to check if an fd index is avaliable
            openindex = i+2;        // add 2 to the open index found in openIndexes array since the first two fd indeces are reserved for terminal use
            break;
        }
    }
    if(openindex == -1){    // exit if no open index found
        return -1;
    }
    //setup the index in the fd array
    curr_pcb->openIndexes[openindex-2] = full;

    curr_pcb->fdArray[openindex].flags = full;  // set flag to full to indicate that fd index is in use
    curr_pcb->terminal = currentTerminal;
    curr_pcb->fdArray[openindex].inode_check = dentry.inode_num;    // set the inode # from the populated dentry using read_dentry_by_name function
    curr_pcb->fdArray[openindex].file_position = 0;     // read data from the start of the file
    if(dentry.filetype == 0){                           // check if file type is rtc
        curr_pcb->fdArray[openindex].jump_table.openFunc = rtc_open;    // assign jump table open function to open rtc
        curr_pcb->fdArray[openindex].jump_table.closeFunc = rtc_close;  // assign jump table close function to close rtc
        curr_pcb->fdArray[openindex].jump_table.readFunc = rtc_read;    // assign jump table read functon to read rtc
        curr_pcb->fdArray[openindex].jump_table.writeFunc = rtc_write;  // assign jump table write function to write rtc
    }
    //directory file type check
    if(dentry.filetype == 1){
        curr_pcb->fdArray[openindex].jump_table.openFunc = open_directory;  // assign jump table open function to open directory
        curr_pcb->fdArray[openindex].jump_table.closeFunc = close_directory; // assign jump table close function to close directory
        curr_pcb->fdArray[openindex].jump_table.readFunc = read_directory;   // assign jump table read functon to read directory
        curr_pcb->fdArray[openindex].jump_table.writeFunc = write_directory; // assign jump table write function to write directory
    }
    //normal file type check
    if(dentry.filetype == 2){
        curr_pcb->fdArray[openindex].jump_table.openFunc = open_file;   // assign jump table open function to open file
        curr_pcb->fdArray[openindex].jump_table.closeFunc = close_file;  // assign jump table close function to close file
        curr_pcb->fdArray[openindex].jump_table.readFunc = read_file;     // assign jump table read functon to read file
        curr_pcb->fdArray[openindex].jump_table.writeFunc = write_file;   // assign jump table write function to write file
    }
    curr_pcb->fdArray[openindex].jump_table.openFunc(filename);     // call the open function from the jumptable for the specified filename
    return openindex;
}

/* int32_t close(int32_t fd)
 * Inputs: fd value
 * Return Value:  returns -1 on failure and the fd # assigned the file data upon sucess
 * Function: delete the data stored in the fd and mark it as free for other programs to populate it
 */
int32_t close(int32_t fd) {
    //find the curr_pcb for the currently shceduled process
    curr_pcb = (struct PCB_t*)(MB8 - (KB8)*(curr_pid_for_term[scheduled_term] + 1));
    if(fd < 2 || fd >7 || curr_pcb->fdArray[fd].flags == empty){   // check if the fd in range
        return -1;      //if not return
    }
    //close out the fd
    curr_pcb->openIndexes[fd-2] = empty;    // empty out the fd -2 index of openIndexes as openindex dosen't keep track of the first two terminal fd indexes
    curr_pcb->fdArray[fd].flags = empty;    // clear out the flags
    curr_pcb->fdArray[fd].file_position = 0;    // set the file position to the start of the file
    int retval = curr_pcb->fdArray[fd].jump_table.closeFunc(fd);      // call the close function from the jumptable
    return retval;
}


/* int32_t getargs(uint8_t* buf, int32_t nbytes)
 * Inputs: buf, and nbytes
 * Return Value: -1 for error
 * Function: stores the arguments in the input buffer
 */
int32_t getargs(uint8_t* buf, int32_t nbytes) {
    // if null buffer return -1
    if(buf == NULL) {
        return -1;
    }
    int i;
    //if length of argument is 0 return -1
    if(strlen((int8_t*)args) == 0){
        return -1;
    }
    //loop through bytes to write and write the global args buffer into the input buffer
    for(i = 0;i<nbytes;i++){
        if(i<BUFSIZE){
            buf[i] = args[i];
        }
        else{
            buf[i] = '\0';
        }
    }
    return 0; //Temp
}

/* int32_t vidmap(uint8_t** screen_start)
 * Inputs: screen_start
 * Return Value: always returns 0
 * Function: maps the video memory into the address space of a user-level process. This allows the process to interact with video memory
 */
int32_t vidmap(uint8_t** screen_start) {
    //null checking
    if(screen_start == NULL) {
        return -1;
    }
    //check whether the address falls within the address range covered by the single user-level page
    if(screen_start < (uint8_t**)USR_MEM_VIRT_ADDR || screen_start > (uint8_t**)(USR_MEM_VIRT_ADDR + MB4)){
        return -1;
    }
    page_directory[VID_PAGE_IDX].KB.pt_address = ((unsigned int)video_page_table)>>12; //set the pointer in the PDE that corresponds to the 4mb page starting at virtual address of 132Mb to point to the page table for the video memory
    //find the terminal where the process scheduled is in, then based on whether that terminal is being displayed or not we assign the first page in the video page to the appropriate page in physical memory
    int32_t visible_term = currentTerminal-1;
    if(scheduled_term == visible_term)
        video_page_table[0].p_address = VIDEO/FourKB; //assign the pointer for the first page in the video page table to point to the video memory in physical memory, if the terminal scheduled is being displayed
    else
        video_page_table[0].p_address = (VIDEO+(scheduled_term+1)*FourKB)/FourKB; //write to a page that isn't being displayed(terminals are 1 indexed, basically visible_term can be 1,2,3 not 0,1,2)
    tlbFlush(); 
    *screen_start = (uint8_t*)(USR_MEM_VIRT_ADDR + MB4); //assign pointer to point to allocated 4kb video memory page allocated below the bottom of the program image
    if(*screen_start == NULL){ //null checking, to make sure the page was allocated properly
        return -1;
    }
    return 0; 
}

/* int32_t set_handler(int32_t signum, void* handler_address)
 * Inputs: signum, handler_address
 * Return Value:  
 * Function: 
 */
int32_t set_handler(int32_t signum, void* handler_address) {
    return -1; //havent done signals
}

/* int32_t sigreturn()
 * Inputs: none
 * Return Value:  
 * Function: 
 */
int32_t sigreturn() {
    return -1; //havent done signals
}
