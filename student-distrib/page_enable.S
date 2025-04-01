#define asm

# void loadPageDirectory(uint32_t page_directory);
# Inputs: base address of page_directory
# Return Value: none
# Function: enables paging and loads cr3 with base address of page_directory

.text
.globl loadPageDirectory
loadPageDirectory:
    pushl %ebp
    movl %esp, %ebp
    pushl %eax
    # get argument into eax
    movl 8(%ebp), %eax
    # store arg in cr3
    movl %eax, %cr3 # move base pointer to PDT to cr3
    # move cr4 into eax
    movl %cr4, %eax
    orl $0x00000010, %eax     # set cr4 to enable 4 mb pages
    # move eax into cr4
    movl %eax, %cr4

    # move cr0 into eax
    movl %cr0, %eax
    orl $0x80000001, %eax     # enable paging by changing the appropriate bits in cr0
    # move eax into cr0
    movl %eax, %cr0

    popl %eax
    movl %ebp, %esp
    popl %ebp
    ret

# void tlbFlush;
# Inputs: none
# Return Value: none
# Function: enables tlb Flush in paging
.global tlbFlush
.align 4
tlbFlush:
    movl %cr3, %eax
    movl %eax, %cr3 # move base pointer to PDT to cr3
    ret
