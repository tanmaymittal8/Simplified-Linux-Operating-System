#include "types.h"
#include "paging.h"
#include "lib.h"
#include "page_enable.h"
#include "systemCall.h"
#include "keyboard.h"

//video address from lib
#define VIDEO 0xB8000
#define MB8 0x800000
#define MB4 0x400000

//initiliaze the direcotry, table to length 1024 and aligned to 4kb size
pd_entry_t page_directory[1024] __attribute__((aligned (FourKB)));
struct pt_entry_t page_table[1024] __attribute__((aligned(FourKB)));
struct pt_entry_t video_page_table[1024] __attribute__((aligned(FourKB)));
uint32_t currentTerminal = 1;

/* void paging_init()
 * Inputs: none
 * Return Value: none 
 * Function: Initializes paging, maps virutal memory to physical memory
 */
void paging_init(){
    //initialize looping variable
    int i =0;
    /* Loop throught the page directory, setting the relevant bits at each directory entry */
    for(i=0; i< 1024; i++){
        /* Set relevant initial entry bits */
        if(i == 0){                 //entry for video memory to a 4kib page table
            //set present and rw to 1 for this entry for video memory
            page_directory[i].KB.present = 1;
            page_directory[i].KB.readWrite = 1;
            //set the rest to 0
            page_directory[i].KB.userSupervisor = 0;
            page_directory[i].KB.writeThrough = 0;
            page_directory[i].KB.cacheDisabled = 0;
            page_directory[i].KB.Accessed = 0;
            page_directory[i].KB.Reserved1 = 0;
            page_directory[i].KB.PS = 0;
            page_directory[i].KB.GlobalPage = 0;
            page_directory[i].KB.AVL = 0;
            // set the address to the top 20 bits of the page table address
            page_directory[i].KB.pt_address = ((unsigned int)page_table)>>12;           // since the whole value is 32 long, we shift by 12 to make it 20 long
        }
        /* set kernel bits accordingly */
        if(i == 1){                  //entry for kernel to a 4mib page
            //set ps,gp,p,rw to 1
            page_directory[i].MB.PS = 1;
            page_directory[i].MB.GlobalPage = 1;
            page_directory[i].MB.present = 1;
            page_directory[i].MB.readWrite = 1;
            // set the rest to 0
            page_directory[i].MB.userSupervisor = 0;
            page_directory[i].MB.writeThrough = 0;
            page_directory[i].MB.cacheDisabled = 0;
            page_directory[i].MB.Accessed = 0;
            page_directory[i].MB.Dirty = 0;
            page_directory[i].MB.AVL = 0;
            page_directory[i].MB.PageTableAttributeIndex = 0;
            page_directory[i].MB.Reserved = 0;
            //set the address to the top 10 bits of the kernel address which is 4mb
            page_directory[i].MB.pt_address = kernel4MB>>22;  //right shit by 22 since 32-22=10
        }
        if(i == 32){                  //entry for a 4mib page
            //set ps,gp,p,rw,us to 1
            page_directory[i].MB.PS = 1;
            page_directory[i].MB.GlobalPage = 1;
            page_directory[i].MB.present = 1;
            page_directory[i].MB.readWrite = 1;
            page_directory[i].MB.userSupervisor = 1;
            // set the rest to 0
            page_directory[i].MB.writeThrough = 0;
            page_directory[i].MB.cacheDisabled = 0;
            page_directory[i].MB.Accessed = 0;
            page_directory[i].MB.Dirty = 0;
            page_directory[i].MB.AVL = 0;
            page_directory[i].MB.PageTableAttributeIndex = 0;
            page_directory[i].MB.Reserved = 0;
            //set the address to the top 10 bits of the program page address which is 4mb
        }
        if(i == 33){ //corresponds to 4mb page starting at virtual address of 132mb for vidmap
            page_directory[i].KB.present = 1;
            page_directory[i].KB.readWrite = 1;
            //set the rest to 0
            page_directory[i].KB.userSupervisor = 1;
            page_directory[i].KB.writeThrough = 0;
            page_directory[i].KB.cacheDisabled = 0;
            page_directory[i].KB.Accessed = 0;
            page_directory[i].KB.Reserved1 = 0;
            page_directory[i].KB.PS = 0;
            page_directory[i].KB.GlobalPage = 0;
            page_directory[i].KB.AVL = 0;
            page_directory[VID_PAGE_IDX].KB.pt_address = ((unsigned int)video_page_table)>>12; // since the whole value is 32 long, we shift by 12 to make it 20 long
            //set the pointer to the video page table in vidmap system call
        }
    }

    /* Loop through the page table, setting the relevant bits of each entry */
    for(i=0; i< 1024; i++){
        /* Relevant inital entry set for video memory*/
        if(i == VIDEO/FourKB){                            //if we are at the correct index in the page table for video memory(address right shifted by 12)
            //set the present,rw,cachedisabled to 1
            page_table[i].present = 1;
            page_table[i].readWrite = 1;
            page_table[i].cacheDisabled = 1;
            // set the rest to 0
            page_table[i].userSupervisor = 0;
            page_table[i].writeThrough = 0;
            page_table[i].Accessed = 0;
            page_table[i].Dirty = 0;
            page_table[i].PageTableAttributeIndex = 0;
            page_table[i].GlobalPage = 0;
            page_table[i].AVL = 0;
            //set the address pointer to video memory
            page_table[i].p_address = VIDEO/FourKB;      //base address video memory

        }
        else if(i == (VIDEO+FourKB)/FourKB){                            //terminal 1 pages
            //set the present,rw,cachedisabled to 1
            page_table[i].present = 1;
            page_table[i].readWrite = 1;
            page_table[i].cacheDisabled = 1;
            // set the rest to 0
            page_table[i].userSupervisor = 0;
            page_table[i].writeThrough = 0;
            page_table[i].Accessed = 0;
            page_table[i].Dirty = 0;
            page_table[i].PageTableAttributeIndex = 0;
            page_table[i].GlobalPage = 0;
            page_table[i].AVL = 0;
            //set the address pointer to video memory
            page_table[i].p_address = (VIDEO+FourKB)/FourKB;      //base address video memory

        }
        else if(i == (VIDEO+2*FourKB)/FourKB){                            //terminal 2 pages
            //set the present,rw,cachedisabled to 1
            page_table[i].present = 1;
            page_table[i].readWrite = 1;
            page_table[i].cacheDisabled = 1;
            // set the rest to 0
            page_table[i].userSupervisor = 0;
            page_table[i].writeThrough = 0;
            page_table[i].Accessed = 0;
            page_table[i].Dirty = 0;
            page_table[i].PageTableAttributeIndex = 0;
            page_table[i].GlobalPage = 0;
            page_table[i].AVL = 0;
            //set the address pointer to video memory
            page_table[i].p_address =(VIDEO+2*FourKB)/FourKB;      //base address video memory

        }
        else if(i == (VIDEO+3*FourKB)/FourKB){                            //terminal 3 pages
            //set the present,rw,cachedisabled to 1
            page_table[i].present = 1;
            page_table[i].readWrite = 1;
            page_table[i].cacheDisabled = 1;
            // set the rest to 0
            page_table[i].userSupervisor = 0;
            page_table[i].writeThrough = 0;
            page_table[i].Accessed = 0;
            page_table[i].Dirty = 0;
            page_table[i].PageTableAttributeIndex = 0;
            page_table[i].GlobalPage = 0;
            page_table[i].AVL = 0;
            //set the address pointer to video memory
            page_table[i].p_address = (VIDEO+3*FourKB)/FourKB;      //base address video memory

        }
    }
    //loop through page table for video memory being set in vidmap 
    //this the page table where one of the pages corresponds to the the video memory page getting assigned in vidmap
    for(i=0; i< 1024; i++){
        if(i == 0){
            video_page_table[i].present = 1;
            video_page_table[i].readWrite = 1;
            video_page_table[i].cacheDisabled = 1;
            // set the rest to 0
            video_page_table[i].userSupervisor = 1;
            video_page_table[i].writeThrough = 0;
            video_page_table[i].Accessed = 0;
            video_page_table[i].Dirty = 0;
            video_page_table[i].PageTableAttributeIndex = 0;
            video_page_table[i].GlobalPage = 0;
            video_page_table[i].AVL = 0;
            video_page_table[i].p_address = VIDEO/FourKB;
            //reassign the pointer in vidmap system call
        }
        else{
            // want everything to be not present and with rw to 1
            video_page_table[i].present = 0;
            video_page_table[i].readWrite = 1;
            video_page_table[i].userSupervisor = 0;
            video_page_table[i].writeThrough = 0;
            video_page_table[i].cacheDisabled = 0;
            video_page_table[i].Accessed = 0;
            video_page_table[i].Dirty = 0;
            video_page_table[i].PageTableAttributeIndex = 0;
            video_page_table[i].GlobalPage = 0;
            video_page_table[i].AVL = 0;
            video_page_table[i].p_address = i;
        }
    }
    // call the assembly function from page_enable.h to enable paging
    loadPageDirectory((unsigned int)page_directory);
}

/* void terminal_changing()
 * Inputs: none
 * Return Value: none 
 * Function: Saves and restores the screen memory when switching terminals with alt +f
 */
void terminal_changing(){
    // save where the video memory is mapped to 
    int temp = page_table[VIDEO/FourKB].p_address;
    // maps the video memory page table to point to the acutal video memory
    page_table[VIDEO/FourKB].p_address = VIDEO/FourKB;
    tlbFlush();

    if(recentTermChoice[1] != currentTerminal){                 //if we actually changed terminals
        int nextAddress = (VIDEO + recentTermChoice[1]*FourKB);  // address of the video memory save to restore to
        int prevAddress = (VIDEO + currentTerminal*FourKB);      // address of the video memory save to save from
        memcpy((void*)prevAddress, (void*)(VIDEO), FourKB);      // memcpy the current video memory into the save of the terminal we leave
        memcpy((void*)(VIDEO), (void*)nextAddress, FourKB);      // memcpy the video memory save of the terminal we are switching from to the actual video memory
        currentTerminal = recentTermChoice[1];                   // update currentTerminal which is the terminal we are viewing
    }

    //restore the video memory page table mapping
    page_table[VIDEO/FourKB].p_address =temp;
    tlbFlush();
}
