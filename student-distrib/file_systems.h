#ifndef FILE_SYSTEMS_H
#define FILE_SYSTEMS_H

#include "types.h"
#include "x86_desc.h"
#include "x86_desc.h"
#define blockSize 4096              // set blocksize size
#define bootBlockReservedLength 52  // boot reserved length is specified as 52 in documentation
#define numDentries 63              // total # of dentries avaliable
#define dentryReservedLength 24     // reserved space in dentry as specified in documentation
#define dentrySize 64               // dentry size
#define FILENAME_LEN 32             // file name size set to 32 bits
#define max_data_blocks 1023        // max size of data blocks according to docuentation
#define dentry_offset 16            // dentry offset to reach the file data
#define byte 8                      // length of a byte
#define empty 0                      // space avaliable on this struct
#define full 1                    // track which file is opened
#define EIGHT_MB 0x800000
#define EIGHT_KB 0x2000
#define MAX_PCB     6           // maximum number of pcb allowed for files
extern struct boot_block_t *boot_block;


// struct to setup the dentry for each file an folder in file systems
typedef struct __attribute__ ((packed,aligned(dentrySize))) dentry_t {
    int8_t filename[FILENAME_LEN];
    int32_t filetype;
    int32_t inode_num;
    int8_t reserved[dentryReservedLength];
} dentry_t;

// struct for the first block in the array that points to the dentries for the folder and files
typedef struct __attribute__ ((packed,aligned(blockSize))) boot_block_t {
    int32_t num_directory_entries;
    int32_t num_inodes;
    int32_t num_data_blocks;
    int8_t reserved[bootBlockReservedLength];
    dentry_t directory_entries[numDentries];
} boot_block_t;

// struct of innode that contains the address for the data of that file
typedef struct __attribute__ ((packed)) inode_t {
    int32_t length;
    int32_t data_block_nums[max_data_blocks];
} inode_t;

//make one type for file open, close, read, write functions
typedef int32_t (*openFuncPtr)(const uint8_t*);
typedef int32_t (*closeFuncPtr)(int32_t);
typedef int32_t (*readFuncPtr)(int32_t, void*, int32_t);
typedef int32_t (*writeFuncPtr)(int32_t, const void*, int32_t);

// jump table struct
typedef struct __attribute__ ((packed)) jtable_t {
    openFuncPtr openFunc;
    closeFuncPtr closeFunc;
    readFuncPtr readFunc;
    writeFuncPtr writeFunc;
} jtable_t;

// file descriptor array that contains info about opened files 
typedef struct __attribute__ ((packed)) file_descriptor_t {
    struct jtable_t jump_table;
    int32_t inode_check;
    int32_t file_position;
    int32_t flags;
} file_descriptor_t;

// used to keep track of open files
typedef struct __attribute__ ((packed)) PCB_t {
    int8_t active;
    int8_t parent;
    int8_t id;
    //scheduled esp ebp
    uint32_t scheduledEbp;
    uint32_t scheduledEsp;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t esp_program_start;
    struct file_descriptor_t fdArray[8];    // fd array is supposed to be 8 entries long according to documentation
    int32_t openIndexes[6];                 // fd indexes open for file operation while first 2 reserved for terminal
    int terminal;
} PCB_t;
struct PCB_t *PCB_array[MAX_PCB]; // Created bc needed in schedule.c

int32_t read_dentry_by_name (const uint8_t* fname, struct dentry_t* dentry);        // finds the dentry corresponding to the file name
int32_t read_dentry_by_index (uint32_t index, struct dentry_t* dentry);             // finds the dentry corresponding to the provided index
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length); // reads the data at the passed inode
void init_file_systems(uint32_t* file_system_start);                                // initialize the file system
int32_t open_directory(const uint8_t* folder_name);                                 // open a directory
int32_t close_directory(int32_t fd);                                                // close the directory that was already open
int32_t read_directory(int32_t fd, void* buf, int32_t nbytes);    // read all the files in the directory
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes);                  // write in directory, currently does nothing           
int32_t open_file(const uint8_t* filename);                                         // opens the file and updates the pcb struct array
int32_t close_file(int32_t fd);                                                     // closes the open file by removing the file from ht pcb struct aray
int32_t read_file(int32_t fd, void* buf, int32_t nbytes);                        // read the contents of the supplied file
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes);                       // write in file, currently does nothing

#endif /* FILE_SYSTEMS_H */
