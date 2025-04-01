#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "file_systems.h"
#include "terminal.h"
#include "rtc.h"
#include "keyboard.h"
#include "systemCall.h"
#include "scheduling.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");


//putc((uint8_t)'#');

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* 
 * Test if we can divide by 0
 * Inputs: None
 * Outputs: Exception caught/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: exception_handlers.c
 */
int divisionErr(){
	TEST_HEADER;
	int i;
	int a = 1;
	int b = 0;
	i = a/b;
	return FAIL;
}
/* 
 * Test if we can dereference a null pointer
 * Inputs: None
 * Outputs: Exception caught/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageFaultNull(){
	TEST_HEADER;
	int* i = (int*)0;
	int a;
	a = *i;
	return FAIL;
}
/* 
 * Test if we can dereference a pointer that's set to memory that's not marked as present(3.9mb)
 * Inputs: None
 * Outputs: Exception caught/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageFaultBelow(){
	TEST_HEADER;
	int *below = (int*)0x390000;
	int a;
	a = *below;
	return FAIL;
}
/* 
 * Test if we can dereference a pointer that's set to memory that's not marked as present(8.1mb)
 * Inputs: None
 * Outputs: Exception caught/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageFaultAbove(){
	TEST_HEADER;
	int *above = (int*)0x810000;
	int a;
	a = *above;
	return FAIL;
}
/* 
 * Test if we can dereference a pointer that's set to memory that's in kernel(4mb - 8mb)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageFaultKernel(){
	TEST_HEADER;
	int *between = (int*)0x600000; //between 0x400000 and 0x800000
	int a;
	a = *between;
	return PASS;
}

/* 
 * Test if we can dereference a pointer that's set to memory lower bound in kernel(4mb)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageFaultLowBound(){
	TEST_HEADER;
	int *low = (int*)0x400000; //between 0x400000 and 0x800000
	int a;
	a = *low;
	return PASS;
}

/* 
 * Test if we can dereference a pointer that's set to memory upper bound in kernel(8mb)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageFaultUpBound(){
	TEST_HEADER;
	char *up = (char*)0x7FFFFF; //between 0x400000 and 0x800000
	char a;
	a = *up;
	return PASS;
}

/* 
 * Test if we can dereference a pointer that's set to memory lower bound in kernel(4mb-1)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageFaultLowOBound(){
	TEST_HEADER;
	char *low = (char*)0x3FFFFF; //between 0x400000 and 0x800000
	char a;
	a = *low;
	return FAIL;
}

/* 
 * Test if we can dereference a pointer that's set to memory upper bound in kernel(8mb+1)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageFaultUpOBound(){
	TEST_HEADER;
	char *up = (char*)0x800000; //between 0x400000 and 0x800000
	char a;
	a = *up;
	return FAIL;
}

/* 
 * Test if we can dereference a valid pointer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageValidPointer(){
	TEST_HEADER;
	int a = 69;
	int *ptr = &a;
	if(*ptr == a){
		return PASS;
	}
	else{
		assertion_failure();
		return FAIL;
	}
}
/* 
 * Test if we can derefernce a pointer which points to memory base set to present(video memory)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageVideoMemLow(){
	TEST_HEADER;
	//0xb8000 is video mem
	int *video_mem = (int*)0xB8000; //between 0x400000 and 0x800000
	int a;
	a = *video_mem;
	return PASS;
}
/* 
 * Test if we can derefernce a pointer which points to memory set to present(video memory end)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageVideoMemHigh(){
	TEST_HEADER;
	//0xb8000 is video mem
	char *video_mem = (char*)0xb8fff; //between 0x400000 and 0x800000
	char a;
	a = *video_mem;
	return PASS;
}
/* 
 * Test if we can derefernce a pointer which points to out of bottom lower limit of memory set to present(video memory-1)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageVideoMemOLow(){
	TEST_HEADER;
	//0xb8000 is video mem
	char *video_mem = (char*)0xB7FFF; //one lower than low bound
	char a;
	a = *video_mem;
	return FAIL;
}
/* 
 * Test if we can derefernce a pointer which points to memory set to present(video memory end+1)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pageVideoMemOHigh(){
	TEST_HEADER;
	//0xb8000 is video mem
	char *video_mem = (char*)0xb9000; //one higher than high bound
	char a;
	a = *video_mem;
	return FAIL;
}
/* 
 * Test if the kernel starting address set in index 1 of PDT is set correctly
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pagePDE(){
	TEST_HEADER;
	uint32_t a = page_directory[1].MB.pt_address;
	if(a == 1){
		return PASS;
	}
	else{
		assertion_failure();
		return FAIL;
	}
}

/* 
 * Test if the kernel starting address set in videomem index of pt is set correctly
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pagePTE(){
	TEST_HEADER;
	uint32_t a = page_table[((0xB8000)/4096)].p_address;
	if(a == ((0xB8000)/4096)){
		return PASS;
	}
	else{
		assertion_failure();
		return FAIL;
	}
}

/* 
 * Test if the specific page table entry which points to the video memory page, has its present bit set to 1
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialization and enabling
 * Files: paging.h
 */
int pagePTEPresent(){
	TEST_HEADER;
	uint32_t a = page_table[((0xB8000)/4096)].present;
	if(a == 1){
		return PASS;
	}
	else{
		assertion_failure();
		return FAIL;
	}
}

/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException1(){
	asm volatile ("int $1"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException2(){
	asm volatile ("int $2"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException3(){
	asm volatile ("int $3"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException4(){
	asm volatile ("int $4"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException5(){
	asm volatile ("int $5"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException6(){
	asm volatile ("int $6"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException7(){
	asm volatile ("int $7"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException8(){
	asm volatile ("int $8"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException9(){
	asm volatile ("int $9"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException10(){
	asm volatile ("int $10"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException11(){
	asm volatile ("int $11"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException12(){
	asm volatile ("int $12"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException13(){
	asm volatile ("int $13"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException14(){
	asm volatile ("int $14"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException16(){
	asm volatile ("int $16"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException17(){
	asm volatile ("int $17"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException18(){
	asm volatile ("int $18"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException19(){
	asm volatile ("int $19"); 
	return FAIL;
}
/* 
 * Test if the specific vector number after the $ is triggered
 * Inputs: None
 * Outputs: Nothing(exception caught)/FAIL
 * Side Effects: None
 * Coverage: exception1
 */
int testException128(){
	asm volatile ("int $128"); 
	return FAIL;
}
/* Checkpoint 2 tests */
/* 
 * Test if we can read all the files in the directory
 * Inputs: None
 * Outputs: prints file names
 * Side Effects: None
 * Coverage: check directory readability
 */
int readindDirect(){
	TEST_HEADER;
	int32_t name = (int32_t)(".");
	uint8_t buf[2016];	// 63 directories*32 characters
	int32_t nbytes = 63;	// number of files and directory
	read_directory(name,buf, nbytes); 
	
	return PASS;
}

/* 
 * Test if we can read from a small file
 * Inputs: None
 * Outputs: prints the data in the file
 * Side Effects: None
 * Coverage: check file readability
 */
int readfiletest(){
	TEST_HEADER;
	//23 for shell
	//38 for

	uint8_t buf[200]; // size of buffer greater than file length
	read_file(2, buf, 187);  // 187 length of file
	// int i;
	// for(i = 0;i<187;i++){ // print the data on the screen
	// 	printf("%c",buf[i]);
	// }
	return PASS;
}



/* 
 * Test if we can read from a small file
 * Inputs: None
 * Outputs: prints the data in the file
 * Side Effects: None
 * Coverage: check file readability
 */
// small file 2
int read_small_file2(){
	TEST_HEADER;
	//init_file_systems(uint32_t* file_system_start)
	printf("\n");
	int8_t* name= "frame1.txt";
	int check;
	printf("start of open file");
	check = open_file((uint8_t*)name);
	printf("----------------done---------");
	if (check == -1){
		printf("inside fail");
		return FAIL;
	}
	uint8_t buf[200]; // size of buffer greater than file length
	//int32_t nbytes = 62;
	read_file(2, buf, 172); // 172 length of file
	int i;
	for(i = 0;i<172;i++){
		printf("%c",buf[i]);
	}
	 close_file(check);
	return PASS;
}

/* 
 * Test if we can read from a executable file
 * Inputs: None
 * Outputs: prints the data in the file
 * Side Effects: None
 * Coverage: check file readability
 */
// executable 1
int read_executable_1(){
	TEST_HEADER;
	int8_t* name= "grep";
	int check;
	check = open_file((uint8_t*)name);
	if (check == -1){
		return FAIL;
	}
	uint8_t buf[buffersize]; // size of buffer greater than file length
	int i;
	for(i = 0;i<buffersize;i++){
		buf[i] = 0;
	}
	read_file(2, buf, buffersize);
	for(i = 0;i<buffersize;i++){
		if(buf[i] =='\0'){
			continue;
		}
		putc(buf[i]);
	}
	return PASS;
}

/* 
 * Test if we can read from a executable file
 * Inputs: None
 * Outputs: prints the data in the file
 * Side Effects: None
 * Coverage: check file readability
 */
// executable 2
int read_executable_2(){
	TEST_HEADER;
	int8_t* name= "ls";
	int check;
	check = open_file((uint8_t*)name);
	if (check == -1){
		return FAIL;
	}
	uint8_t buf[buffersize]; // size of buffer greater than file length
	int i;
	for(i = 0;i<buffersize;i++){
		buf[i] = 0;
	}
	read_file(2, buf, buffersize); 
	for(i = 0;i<buffersize;i++){
		if(buf[i] =='\0'){
			continue;
		}
		putc(buf[i]);
	}
	return PASS;
}

/* 
 * Test if we can read from a large file
 * Inputs: None
 * Outputs: prints the data in the file
 * Side Effects: None
 * Coverage: check file readability
 */
// large files
int read_large_file1(){
	TEST_HEADER;
	int8_t* name= "fish";
	int check;
	check = open_file((uint8_t*)name);
	if (check == -1){
		return FAIL;
	}
	uint8_t buf[buffersize]; // size of buffer greater than file length
	int i;
	for(i = 0;i<buffersize;i++){
		buf[i] = 0;
	}
	read_file(2, buf, buffersize); 
	for(i = 0;i<buffersize;i++){
		if(buf[i] =='\0'){
			continue;
		}
		putc(buf[i]);
	}
	return PASS;
}

/* 
 * Test if we can read from a large file
 * Inputs: None
 * Outputs: prints the data in the file
 * Side Effects: None
 * Coverage: check file readability
 */
// large file 2
int read_large_file2(){
	TEST_HEADER;
	int8_t* name= "verylargetextwithverylongname.tx";
	int check;
	printf("start");
	check = open_file((uint8_t*)name);
	printf("open passed");
	if (check == -1){
		return FAIL;
	}
	uint8_t buf[buffersize]; // size of buffer greater than file length
	int i;
	for(i = 0;i<buffersize;i++){
		buf[i] = 0;
	}
	printf("read started");
	read_file(2, buf, buffersize); 
	printf("read passed");
	for(i = 0;i<buffersize;i++){
		if(buf[i] =='\0'){
			continue;
		}
		putc(buf[i]);
	}
	return PASS;
}

/* 
 * Test if we can read from a large file
 * Inputs: None
 * Outputs: prints the data in the file
 * Side Effects: None
 * Coverage: check file readability
 */
// large file 2 with error
int read_large_file2_error(){
	TEST_HEADER;
	int8_t* name= "verylargetextwithverylongname.txt";
	int check;
	check = open_file((uint8_t*)name);
	if (check == -1){
		return PASS;
	}
	uint8_t buf[buffersize]; // size of buffer greater than file length
	int i;
	for(i = 0;i<buffersize;i++){
		buf[i] = 0; // offset set to 0
	}
	read_file(2, buf, buffersize);
	for(i = 0;i<buffersize;i++){
		if(buf[i] =='\0'){
			continue;
		}
		putc(buf[i]);
	}
	return FAIL;
}

/* 
 * Test if we can partiall read from a small file
 * Inputs: None
 * Outputs: prints the data in the file
 * Side Effects: None
 * Coverage: check if we can read a part of the file
 */
// int partial_read_file_test(){
// 	TEST_HEADER;
// 	int8_t* name= "frame0.txt";
// 	int check;
// 	check = open_file((uint8_t*)name);
// 	if (check == -1){
// 		return FAIL;
// 	}
// 	uint8_t buf[buffersize]; // size of buffer greater than file length
// 	int i;
// 	for(i = 0;i<buffersize;i++){
// 		buf[i] = 0; 
// 	}
// 	uint32_t offset = 50; // offset set to 0
// 	read_file(2, buf, buffersize, offset);
// 	for(i = 0;i<buffersize;i++){
// 		if(buf[i] =='\0'){
// 			continue;
// 		}
// 		putc(buf[i]);
// 	}
// 	return PASS;
// }

/* 
 * Test if we can read from a small file with inital offset and partial read
 * Inputs: None
 * Outputs: prints the data in the file
 * Side Effects: None
 * Coverage: check file readability with offset and partial read
 */
// int offset_partial_read_file_test(){
// 	TEST_HEADER;
// 	int8_t* name= "frame0.txt";
// 	int check;
// 	check = open_file((uint8_t*)name);
// 	if (check == -1){
// 		return FAIL;
// 	}
// 	uint8_t buf[buffersize]; // size of buffer greater than file length
// 	int i;
// 	for(i = 0;i<buffersize;i++){
// 		buf[i] = 0;
// 	}
// 	uint32_t offset = 100; // offset set to 100
// 	read_file(2, buf, buffersize, offset); // 40000 length of file
// 	for(i = 0;i<buffersize;i++){
// 		if(buf[i] =='\0'){
// 			continue;
// 		}
// 		putc(buf[i]);
// 	}
// 	return PASS;
// }

/* 
 * Test if we can read from a small file with inital offset and partial read
 * Inputs: None
 * Outputs: prints the data in the file
 * Side Effects: None
 * Coverage: check file readability with offset and partial read
 */
int readdentryname(){
	TEST_HEADER;
	struct dentry_t currentDentry;
    int valid = 0;
	int8_t* name = "fish";
    valid = read_dentry_by_name((uint8_t*)name, &currentDentry); // updates dentry with the fish data
	if(valid == -1){	// check if file fish was found
		return FAIL;
	}
	int i;
	for(i = 0;i<32;i++){
		printf("%c",currentDentry.filename[i]); // print the data
		// printf("%c",name[i]);
	}
	return PASS;
}

/* 
 * Test if we can open a file
 * Inputs: None
 * Outputs: returns Pass or Fail
 * Side Effects: None
 * Coverage: checks if file is added to the pcb struct
 */
int test_file_open1(){
	TEST_HEADER;
	printf("\n");
    int valid = 0;
	int8_t* name = "frame0.txt";
    valid = open_file((uint8_t*)name); // checks if the file can be opened
	if(valid == -1){
		return FAIL;
	}
	return PASS;
}

// /* 
//  * Test if we can read all the files in the directory
//  * Inputs: None
//  * Outputs: prints file names
//  * Side Effects: None
//  * Coverage: check directory readability
//  */
// int readindDirect(){
// 	TEST_HEADER;
// 	int32_t name = (int32_t)(".");
// 	uint8_t buf[2016];	// 63 directories*32 characters
// 	int32_t nbytes = 63;	// number of files and directory
// 	uint32_t buffsize[63]; // buffer to store file size
// 	uint32_t bufftype[63];	 // buffer to store file type
// 	uint32_t bufflen;		// value to store length
// 	read_directory(name,buf, nbytes, buffsize, bufftype, &bufflen); 
// 	int j,i;
// 	printf("\n ");
// 	for(i=0;i<bufflen;i++){
		
// 			printf("file_name: ");
// 			for(j=0;j<32;j++){	// loop through all the 32 charactors
// 				printf("%c",buf[i*32+j]); // used row major order to print the data
// 			}
		
// 			printf(", file_type: %d",bufftype[i]);
// 			printf(", file_size: %d \n",buffsize[i]);
// 	}
// 	return PASS;
// }

// /* 
//  * Test if we can read from a small file
//  * Inputs: None
//  * Outputs: prints the data in the file
//  * Side Effects: None
//  * Coverage: check file readability
//  */
// int readfiletest(){
// 	TEST_HEADER;
// 	int8_t* name= "frame0.txt";
// 	uint8_t buf[200]; // size of buffer greater than file length
// 	uint32_t offset = 0;
// 	read_file((uint8_t*)name, buf, 187, offset);  // 187 length of file
// 	int i;
// 	for(i = 0;i<187;i++){ // print the data on the screen
// 		printf("%c",buf[i]);
// 	}
// 	return PASS;
// }



// /* 
//  * Test if we can read from a small file
//  * Inputs: None
//  * Outputs: prints the data in the file
//  * Side Effects: None
//  * Coverage: check file readability
//  */
// // small file 2
// int read_small_file2(){
// 	TEST_HEADER;
// 	printf("\n");
// 	int8_t* name= "frame1.txt";
// 	int check;
// 	check = open_file((uint8_t*)name);
// 	if (check == -1){
// 		return FAIL;
// 	}
// 	uint8_t buf[200]; // size of buffer greater than file length
// 	//int32_t nbytes = 62;
// 	uint32_t offset = 0; // offset set to 0
// 	read_file((uint8_t*)name, buf, 172, offset); // 172 length of file
// 	int i;
// 	for(i = 0;i<172;i++){
// 		printf("%c",buf[i]);
// 	}
// 	close_file((int32_t*)name);
// 	return PASS;
// }

// /* 
//  * Test if we can read from a executable file
//  * Inputs: None
//  * Outputs: prints the data in the file
//  * Side Effects: None
//  * Coverage: check file readability
//  */
// // executable 1
// int read_executable_1(){
// 	TEST_HEADER;
// 	int8_t* name= "grep";
// 	int check;
// 	check = open_file((uint8_t*)name);
// 	if (check == -1){
// 		return FAIL;
// 	}
// 	uint8_t buf[buffersize]; // size of buffer greater than file length
// 	int i;
// 	for(i = 0;i<buffersize;i++){
// 		buf[i] = 0;
// 	}
// 	uint32_t offset = 0; // offset set to 0
// 	read_file((uint8_t*)name, buf, buffersize, offset);
// 	for(i = 0;i<buffersize;i++){
// 		if(buf[i] =='\0'){
// 			continue;
// 		}
// 		putc(buf[i]);
// 	}
// 	return PASS;
// }

// /* 
//  * Test if we can read from a executable file
//  * Inputs: None
//  * Outputs: prints the data in the file
//  * Side Effects: None
//  * Coverage: check file readability
//  */
// // executable 2
// int read_executable_2(){
// 	TEST_HEADER;
// 	int8_t* name= "ls";
// 	int check;
// 	check = open_file((uint8_t*)name);
// 	if (check == -1){
// 		return FAIL;
// 	}
// 	uint8_t buf[buffersize]; // size of buffer greater than file length
// 	int i;
// 	for(i = 0;i<buffersize;i++){
// 		buf[i] = 0;
// 	}
// 	uint32_t offset = 0; // offset set to 0
// 	read_file((uint8_t*)name, buf, buffersize, offset); 
// 	for(i = 0;i<buffersize;i++){
// 		if(buf[i] =='\0'){
// 			continue;
// 		}
// 		putc(buf[i]);
// 	}
// 	return PASS;
// }

// /* 
//  * Test if we can read from a large file
//  * Inputs: None
//  * Outputs: prints the data in the file
//  * Side Effects: None
//  * Coverage: check file readability
//  */
// // large files
// int read_large_file1(){
// 	TEST_HEADER;
// 	int8_t* name= "fish";
// 	int check;
// 	check = open_file((uint8_t*)name);
// 	if (check == -1){
// 		return FAIL;
// 	}
// 	uint8_t buf[buffersize]; // size of buffer greater than file length
// 	int i;
// 	for(i = 0;i<buffersize;i++){
// 		buf[i] = 0;
// 	}
// 	uint32_t offset = 0; // offset set to 0
// 	read_file((uint8_t*)name, buf, buffersize, offset); 
// 	for(i = 0;i<buffersize;i++){
// 		if(buf[i] =='\0'){
// 			continue;
// 		}
// 		putc(buf[i]);
// 	}
// 	return PASS;
// }

// /* 
//  * Test if we can read from a large file
//  * Inputs: None
//  * Outputs: prints the data in the file
//  * Side Effects: None
//  * Coverage: check file readability
//  */
// // large file 2
// int read_large_file2(){
// 	TEST_HEADER;
// 	int8_t* name= "verylargetextwithverylongname.tx";
// 	int check;
// 	check = open_file((uint8_t*)name);
// 	if (check == -1){
// 		return FAIL;
// 	}
// 	uint8_t buf[buffersize]; // size of buffer greater than file length
// 	int i;
// 	for(i = 0;i<buffersize;i++){
// 		buf[i] = 0;
// 	}
// 	uint32_t offset = 0; // offset set to 0
// 	read_file((uint8_t*)name, buf, buffersize, offset); 
// 	for(i = 0;i<buffersize;i++){
// 		if(buf[i] =='\0'){
// 			continue;
// 		}
// 		putc(buf[i]);
// 	}
// 	return PASS;
// }

// /* 
//  * Test if we can read from a large file
//  * Inputs: None
//  * Outputs: prints the data in the file
//  * Side Effects: None
//  * Coverage: check file readability
//  */
// // large file 2 with error
// int read_large_file2_error(){
// 	TEST_HEADER;
// 	int8_t* name= "verylargetextwithverylongname.txt";
// 	int check;
// 	check = open_file((uint8_t*)name);
// 	if (check == -1){
// 		return PASS;
// 	}
// 	uint8_t buf[buffersize]; // size of buffer greater than file length
// 	int i;
// 	for(i = 0;i<buffersize;i++){
// 		buf[i] = 0; // offset set to 0
// 	}
// 	uint32_t offset = 0;
// 	read_file((uint8_t*)name, buf, buffersize, offset);
// 	for(i = 0;i<buffersize;i++){
// 		if(buf[i] =='\0'){
// 			continue;
// 		}
// 		putc(buf[i]);
// 	}
// 	return FAIL;
// }

// /* 
//  * Test if we can partiall read from a small file
//  * Inputs: None
//  * Outputs: prints the data in the file
//  * Side Effects: None
//  * Coverage: check if we can read a part of the file
//  */
// int partial_read_file_test(){
// 	TEST_HEADER;
// 	int8_t* name= "frame0.txt";
// 	int check;
// 	check = open_file((uint8_t*)name);
// 	if (check == -1){
// 		return FAIL;
// 	}
// 	uint8_t buf[buffersize]; // size of buffer greater than file length
// 	int i;
// 	for(i = 0;i<buffersize;i++){
// 		buf[i] = 0; 
// 	}
// 	uint32_t offset = 50; // offset set to 0
// 	read_file((uint8_t*)name, buf, buffersize, offset);
// 	for(i = 0;i<buffersize;i++){
// 		if(buf[i] =='\0'){
// 			continue;
// 		}
// 		putc(buf[i]);
// 	}
// 	return PASS;
// }

// /* 
//  * Test if we can read from a small file with inital offset and partial read
//  * Inputs: None
//  * Outputs: prints the data in the file
//  * Side Effects: None
//  * Coverage: check file readability with offset and partial read
//  */
// int offset_partial_read_file_test(){
// 	TEST_HEADER;
// 	int8_t* name= "frame0.txt";
// 	int check;
// 	check = open_file((uint8_t*)name);
// 	if (check == -1){
// 		return FAIL;
// 	}
// 	uint8_t buf[buffersize]; // size of buffer greater than file length
// 	int i;
// 	for(i = 0;i<buffersize;i++){
// 		buf[i] = 0;
// 	}
// 	uint32_t offset = 100; // offset set to 100
// 	read_file((uint8_t*)name, buf, buffersize, offset); // 40000 length of file
// 	for(i = 0;i<buffersize;i++){
// 		if(buf[i] =='\0'){
// 			continue;
// 		}
// 		putc(buf[i]);
// 	}
// 	return PASS;
// }

// /* 
//  * Test if we can read from a small file with inital offset and partial read
//  * Inputs: None
//  * Outputs: prints the data in the file
//  * Side Effects: None
//  * Coverage: check file readability with offset and partial read
//  */
// int readdentryname(){
// 	TEST_HEADER;
// 	struct dentry_t currentDentry;
//     int valid = 0;
// 	int8_t* name = "fish";
//     valid = read_dentry_by_name((uint8_t*)name, &currentDentry); // updates dentry with the fish data
// 	if(valid == -1){	// check if file fish was found
// 		return FAIL;
// 	}
// 	int i;
// 	for(i = 0;i<32;i++){
// 		printf("%c",currentDentry.filename[i]); // print the data
// 		// printf("%c",name[i]);
// 	}
// 	return PASS;
// }

// /* 
//  * Test if we can open a file
//  * Inputs: None
//  * Outputs: returns Pass or Fail
//  * Side Effects: None
//  * Coverage: checks if file is added to the pcb struct
//  */
// int test_file_open1(){
// 	TEST_HEADER;
// 	printf("\n");
//     int valid = 0;
// 	int8_t* name = "frame0.txt";
//     valid = open_file((uint8_t*)name); // checks if the file can be opened
// 	if(valid == -1){
// 		return FAIL;
// 	}
// 	return PASS;
// }


/* 
 * Test if rtc_open sets the frequency to 2hz and test rtc_close
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: rtc driver and virtualization
 * Files: rtc.h
 */
int rtc_open_close_test(){
	TEST_HEADER;
	rtc_open(); //set frequency to 2hz
	//print 0 through 9 where each number gets printed at 2hz
	int i;
	for(i=0; i<10; i++){
		rtc_read(1, NULL, 0); //only returns once the specific number of interrupts have passed
		printf("%d", i);
	}
	rtc_close(1);
	return PASS;
}
/* 
 * Test if rtc_write can write the frequencies properly
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: rtc driver and virtualization
 * Files: rtc.h
 */
int rtc_write_test(){
	TEST_HEADER;
	int i;
	for(i=0; i<10; i++){
		int j;
		uint32_t frequency = (1 << (i+1)); //this operation sets frequency to 2^(i+1) so frequency ranges from 2hz to 1024hz
		uint32_t buf[1] = {frequency};
		printf("%dhz: ", frequency);
		rtc_write(1, buf, 4); //call rtc_write to set the new frequency in the rtc
		for(j = 0; j<24; j++){
			rtc_read(1, NULL, 0); //only returns once the specific number of interrupts have passed
			printf("1"); //prints 24 1s at different frequencies
		}
		printf("\n");
	}
	return PASS;
}
/* 
 * Test if rtc_write will return -1 if we pass in a frequency which is too high
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: None
 * Coverage: rtc driver and virtualization
 * Files: rtc.h
 */
int rtc_invalid_freq_high(){
	TEST_HEADER;
	uint32_t frequency = 2048; //this frequency is above the max frequency of 1024
	uint32_t buf[1] = {frequency};
	int ret_val = rtc_write(1, buf, 4); //call rtc_write to set the new frequency in the rtc
	if(ret_val == -1)
		return PASS;
	else
		return FAIL;
}
/* 
 * Test if rtc_write will return -1 if we pass in a frequency which is too low
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: None
 * Coverage: rtc driver and virtualization
 * Files: rtc.h
 */
int rtc_invalid_freq_low(){
	TEST_HEADER;
	uint32_t frequency = -1; //this frequency is below 0 so is invalid
	uint32_t buf[1] = {frequency};
	int ret_val = rtc_write(1, buf, 4); //call rtc_write to set the new frequency in the rtc
	if(ret_val == -1)
		return PASS;
	else
		return FAIL;
}
/* 
 * Test if rtc_write will return -1 if we pass in a frequency which is not a power of 2
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: None
 * Coverage: rtc driver and virtualization
 * Files: rtc.h
 */
int rtc_invalid_freq_power_of_2(){
	TEST_HEADER;
	uint32_t frequency = 7; //this frequency is not a power of 2 so is invalid
	uint32_t buf[1] = {frequency};
	int ret_val = rtc_write(1, buf, 4); //call rtc_write to set the new frequency in the rtc
	if(ret_val == -1)
		return PASS;
	else
		return FAIL;
}

void testTerminalReadWrite() {
	int i = 0;
	unsigned char test[128];
	while(i != 1) {
		int temp = terminal_read(0, test, 128);
		temp = terminal_write(0, test, temp);
		if(DATA_PORT == 0x52) { 
			i = 1; 
		}
	}
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */
int testPIT(){
	pit_init();
	return PASS;
}


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("pit_test", testPIT());
	//TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("division_test", divisionErr());
	// TEST_OUTPUT("pageFaultNull", pageFaultNull());
	// TEST_OUTPUT("pageFaultBelow", pageFaultBelow());
	// TEST_OUTPUT("pageFaultAbove", pageFaultAbove());
	// TEST_OUTPUT("pageFaultKernel", pageFaultKernel());
	// TEST_OUTPUT("pageFaultLowBound", pageFaultLowBound());
	// TEST_OUTPUT("pageFaultUpBound", pageFaultUpBound());
	// TEST_OUTPUT("pageFaultUpOBound", pageFaultUpOBound());
	// TEST_OUTPUT("pageFaultLowOBound", pageFaultLowOBound());
	// TEST_OUTPUT("pageValidPointer", pageValidPointer());
	// TEST_OUTPUT("pageVideoMemLow", pageVideoMemLow());
	// TEST_OUTPUT("pageVideoMemHigh", pageVideoMemHigh());
	// TEST_OUTPUT("pageVideoMemOLow", pageVideoMemOLow());
	// TEST_OUTPUT("pageVideoMemOHigh", pageVideoMemOHigh());
	// TEST_OUTPUT("pagePDE", pagePDE());
	// TEST_OUTPUT("pagePTE", pagePTE());
	// TEST_OUTPUT("pagePTEPresent", pagePTEPresent());
	// TEST_OUTPUT("exception1", testException1());
	// TEST_OUTPUT("exception128", testException128());
	// TEST_OUTPUT("readindDirect", readindDirect());
	// TEST_OUTPUT("read_small_file2", read_small_file2());
	// TEST_OUTPUT("readfiletest", readfiletest());

	// reading files
	//TEST_OUTPUT("readfiletest", readfiletest()); // read entire file
	//TEST_OUTPUT("partial_read_file_test", partial_read_file_test()); // read part of the file
	//TEST_OUTPUT("offset_partial_read_file_test", offset_partial_read_file_test()); // read entire file

	//  TEST_OUTPUT("read_small_file2", read_small_file2()); // read small file
	//TEST_OUTPUT("read_executable_1", read_executable_1()); // read executable file
	//TEST_OUTPUT("read_executable_2", read_executable_2()); // read execuatble file
	// TEST_OUTPUT("read_large_file1", read_large_file1()); // read large file
	// read_large_file1();
	// TEST_OUTPUT("read_large_file2", read_large_file2()); // read large file
	// TEST_OUTPUT("read_large_file2_error", read_large_file2_error()); // read large file name too long
	// TEST_OUTPUT("test_file_open1", test_file_open1()); //  check if file open

	//TEST_OUTPUT("rtc_open_close_test", rtc_open_close_test());
	// TEST_OUTPUT("rtc_write_test", rtc_write_test());
	// TEST_OUTPUT("rtc_invalid_freq_high", rtc_invalid_freq_high());
	// TEST_OUTPUT("rtc_invalid_freq_low", rtc_invalid_freq_low());
	// TEST_OUTPUT("Rtc_invalid_freq_power_of_2", rtc_invalid_freq_power_of_2());

	//testTerminalReadWrite();
}
