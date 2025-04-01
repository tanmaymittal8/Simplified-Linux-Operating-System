#ifndef _PAGE_ENABLE_H
#define _PAGE_ENABLE_H

//function to call in paging.c which will turn on paging to use our paging.c
extern void loadPageDirectory(uint32_t page_directory);
extern void tlbFlush();

#endif /* _PAGE_ENABLE_H */
