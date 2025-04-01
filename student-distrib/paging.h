#ifndef _PAGING_H
#define _PAGING_H

/* Initialize the struct used for a 4kB sized page directory entry */
typedef struct __attribute__ ((packed,aligned(4))) pd_entryKB_t {
        // set the bit sizes and order according to ISA
        uint32_t present : 1;
        uint32_t readWrite : 1;
        uint32_t userSupervisor : 1;
        uint32_t writeThrough : 1;
        uint32_t cacheDisabled : 1;
        uint32_t Accessed : 1;
        uint32_t Reserved1 : 1;
        uint32_t PS : 1;
        uint32_t GlobalPage : 1;
        uint32_t AVL : 3;
        uint32_t pt_address : 20;               // Set 20 bits to hold the address of the relevant page table
} pd_entryKB_t;

/* Initialize the struct used for a 4MB sized page directory entry, which is used for the kernel */
typedef struct __attribute__ ((packed,aligned(4))) pd_entryMB_t {
        // set the bit sizes and order according to ISA
        uint32_t present : 1;
        uint32_t readWrite : 1;
        uint32_t userSupervisor : 1;
        uint32_t writeThrough : 1;
        uint32_t cacheDisabled : 1;
        uint32_t Accessed : 1;
        uint32_t Dirty : 1;
        uint32_t PS : 1;
        uint32_t GlobalPage : 1;
        uint32_t AVL : 3;
        uint32_t PageTableAttributeIndex : 1;
        uint32_t Reserved : 9;
        uint32_t pt_address : 10;               // Set 10 bits, so the last 12 can be mapped to physical map
} pd_entryMB_t;

/* Two types of page directory entries in a union*/
typedef union pd_entry_t {
    struct pd_entryMB_t MB;
    struct pd_entryKB_t KB;
} pd_entry_t;

/* Initialize the struct used for each page table entry, memory aligned to account for bytes (4 bits) */
typedef struct __attribute__ ((packed,aligned(4))) pt_entry_t {
        // set the bit sizes and order according to ISA
        uint32_t present: 1;
        uint32_t readWrite: 1;
        uint32_t userSupervisor: 1;
        uint32_t writeThrough: 1;
        uint32_t cacheDisabled: 1;
        uint32_t Accessed: 1;
        uint32_t Dirty: 1;
        uint32_t PageTableAttributeIndex: 1;
        uint32_t GlobalPage : 1;
        uint32_t AVL : 3;
        uint32_t p_address : 20;               // Set 20 bits to hold the address of the relevant page
} pt_entry_t;

/* The page directory and the page table have 1024 entries, each 4 bytes so memory aligned to 4096(4kB) */
extern pd_entry_t page_directory[1024] __attribute__((aligned (4096))); // 1024 page directory entries each entry corresponding to 4Mb in memory
extern struct pt_entry_t page_table[1024] __attribute__((aligned(4096))); // 1024 page table entries each entry corresponding to 4kB in memory
extern struct pt_entry_t video_page_table[1024] __attribute__((aligned(4096))); // page table used in vidmap

extern uint32_t currentTerminal;
//function to be called in kernel.c to initialize paging
void paging_init();
extern void terminal_changing();

// size of 4kb
#define FourKB 4096
//macro for kernel address at 4mb
#define kernel4MB 0x400000

#endif /* _PAGING_H */
