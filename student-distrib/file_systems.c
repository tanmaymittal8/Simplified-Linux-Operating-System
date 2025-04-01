#include "file_systems.h"
#include "systemCall.h"
#include "lib.h"
#include "scheduling.h"

struct boot_block_t *boot_block;
struct PCB_t *pcb;
struct file_descriptor_t files_array[8];
int32_t dir_open;

/* void init_file_systems(uint32_t file_system_start)
 * Inputs: start of the file systems array
 * Return Value: none
 * Function: initilizes file systems
 */
void init_file_systems(uint32_t* file_system_start){
    boot_block = (struct boot_block_t *)file_system_start; // point the boot of the file system at the starting address of the fs array
}

/*  int32_t read_dentry_by_name (const uint8_t* fname, struct dentry_t* dentry)
 * Inputs: file name and a dentry
 * Return Value: 0 to indicate sucess and -1 to indicate failure
 * Function: copies the dentry contest of the specific file found using the file name 
 */
int32_t read_dentry_by_name (const uint8_t* fname, struct dentry_t* dentry){
    int index = -1;                                         //initialize the index variable to -1 in case the file can't be found
    int i;                                                  //declare a looping variable
    if(strlen((const int8_t*)fname) > 32){ // if file name length is greater than 32 exit
        return -1;
    }
    //loop through the dentries
    for(i = 0;i<boot_block->num_directory_entries;i++){  // itterate through all the files in the boot block
        //compare the inputted file name across the current dentries name and see if theyre equal
        if(strncmp((const int8_t*)fname, (int8_t*)(&boot_block->directory_entries[i].filename), FILENAME_LEN) == 0){
            //set the index of the dentry if the filename was found
            index = i;
        }
    }
    //if the index = -1, meaning the file was never found, return -1
    if(index == -1){
        return -1;
    }
    //declare and initialize the return value from read_dentry_by_index
    int valid = 0;
    //call read_dentry_by_index with the dentry argument and index of the filename
    valid = read_dentry_by_index(index,dentry);
    //return the value returned by read_dentry_by_index
    return valid;
}

/* int32_t read_dentry_by_index (uint32_t index, struct dentry_t* dentry)
 * Inputs: file index and a dentry
 * Return Value: 0 to indicate sucess and -1 to indicate failure
 * Function: copies the dentry contest of the specific file found using the index 
 */
int32_t read_dentry_by_index (uint32_t index, struct dentry_t* dentry){
    //if the index argument is out of bounds, return -1
    if((index >= (boot_block->num_directory_entries))){
        return -1;
    }
    // strncpy the indexed filename in the argument dentry's filename
    strncpy((int8_t*)(&(dentry->filename)),(int8_t*)(&(boot_block->directory_entries[index].filename)),strlen((boot_block->directory_entries[index].filename)));
    // store the indexed filetype in the argument dentry's filetype
    dentry->filetype = boot_block->directory_entries[index].filetype;
    // store the indexed inode number in the argument dentry's inode number
    dentry->inode_num = boot_block->directory_entries[index].inode_num;

    return 0;
}

/* int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * Inputs: file index inode, offset, buf, and length
 * Return Value: -1 to indicate failure, else number of bytes read
 * Function: copies the data of the file into buf for later use
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    //caluclate a pointer to the start of the inodes
    struct inode_t* inodeStart = (struct inode_t*)(boot_block+1);
    //caulculate a pointer to the current inode based off of the inode index passed in
    struct inode_t* inodeCurr = (struct inode_t*)(inodeStart+inode);
    //calculate a pointer to the start of the data blocks
    uint8_t* dataStart = (uint8_t*)((struct inode_t*)(boot_block+1+boot_block->num_inodes));
    //if the inode is out of bounds, return -1
    if((inode >= boot_block->num_inodes)){ 
        return -1;
    }
    //if the offset makes is beyond the datas length, return 0 bytes read
    if((offset > inodeCurr->length)){ 
        return 0;
    }
    //initilalize i as our looping variable
    int i;
    //caluclate the index of data blocks of the inode to start at
    int indexOfdata_block_nums = offset/blockSize;
    //caluclate the byte within the first data block after offset to start reading at
    int addressInBlock = offset%blockSize;
    //initialize a variable for which actual data block number we are on out of the inode
    int DataBlockNum;
    //initialize bytes read to 0
    int bytesread=0;
    
    //set datablocknum after indexing the datablocknums list from the inode with the offset
    DataBlockNum = inodeCurr->data_block_nums[indexOfdata_block_nums];
    //loop through the length of the bytes to be read
    for(i = 0; i<length; i++){
        //if we should go to the next data block
        if((addressInBlock == blockSize)){
            //if we have read to the limit of the inodes length return the bytes read
            if(bytesread > (inodeCurr->length - offset)){
                return bytesread;     
            }
            //increment the index so we can access the next datablocknum
            indexOfdata_block_nums++;
            //use this index to index the datablocknums list in the inode
            DataBlockNum = inodeCurr->data_block_nums[indexOfdata_block_nums];
            //reset the addressinblock to be 0 so we start from the beginning of the block
            addressInBlock = 0;
        }
        //call memcpy to move the byte we are currently on into the buffer
        memcpy(buf + i, ((uint8_t*)(dataStart + blockSize*DataBlockNum+addressInBlock)), 1);
        //increment our variables for looping
        bytesread++;
        addressInBlock++;
    }
    return bytesread; // return #bytes copied
}

/* int32_t open_directory(const uint8_t* folder_name)
 * Inputs: folder name pointer
 * Return Value: 0 to indicate sucess and -1 to indicate failure
 * Function: sets directory flag to 0 meaning directory open
 */
int32_t open_directory(const uint8_t* folder_name){
    return 0;                                              //return 0 since most work done in the systemCall.c open

}  


/* int32_t close_directory(int32_t fd)
 * Inputs: folder name
 * Return Value: 0 to indicate sucess and -1 to indicate failure
 * Function: sets directory flag to -1 meaning directory closed
 */
int32_t close_directory(int32_t fd){
    return 0;              //return 0 since most work done in the systemCall.c close

}

/* int32_t read_directory(int32_t fd, uint8_t* buf, int32_t nbytes)
 * Inputs: folder name, buff, nbytes
 * Return Value: 0 to indicate finish and -1 to indicate failure and characters written otherwise
 * Function: reads all the files, types, lengths in the specified directory and stores them in the argument buffer 
 */
int32_t read_directory(int32_t fd, void* buf, int32_t nbytes){
    //declare our looping variable
    int j; 
    struct PCB_t *curr_pcb = (struct PCB_t *)(MB8 - KB8*(curr_pid_for_term[scheduled_term]+1));       //calculate a pointer to the pcb
    //intitialize character written count to 0
    int charwritten=0;
    if(curr_pcb->fdArray[fd].flags == full){                        //if the fd is present in the pcb
        if(curr_pcb->fdArray[fd].file_position<0 || curr_pcb->fdArray[fd].file_position>=boot_block->num_directory_entries){        //if invalid position reset to 0 and return
            curr_pcb->fdArray[fd].file_position=0;
            return 0;
        }
        for(j=0;j<strlen(boot_block->directory_entries[curr_pcb->fdArray[fd].file_position].filename);j++){             //loop through the current filename and add it to the buffer
            if(j<FILENAME_LEN){
                ((unsigned char *)buf)[j] = (boot_block->directory_entries[curr_pcb->fdArray[fd].file_position].filename)[j];       //set each character in the buffer
                charwritten++;          //increment characters written count
            }
        }
        curr_pcb->fdArray[fd].file_position+=1;         //increment file position
        return charwritten;                             //return characters written
    }
    return -1; //return error if fd empty
}

/* int32_t write_directory(int32_t fd, uint8_t* buf, int32_t nbytes)
 * Inputs: file name, buf, nbytes
 * Return Value: -1 to indicate failure
 * Function: does nothing
 */
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes){
    return -1;          // set to -1 as we don't perform write operation on directory
}

/* int32_t open_file(const uint8_t* filename)
 * Inputs: file name
 * Return Value: 0 to indicate sucess
 * Function: adds the desired file to the pcb array indicating the file has been opened
 */
int32_t open_file(const uint8_t* filename){
    return 0;           //everything for this open is done in the systemCall.c open function, so just return 0
}

/* int32_t close_file(int32_t* fd)
 * Inputs: file name
 * Return Value: 0 to indicate sucess 
 * Function: removes the desired file to the pcb array indicating the file has been closed
 */
int32_t close_file(int32_t fd){ // it has file descriptor
    return 0;           //everything for this close is done in the systemCall.c close function, so just return 0
}

/* int32_t read_file(uint8_t* fd, uint8_t* buf, int32_t nbytes, uint32_t offset)
 * Inputs: file name, buf, nbytes
 * Return Value: return num of bytes read and -1 to indicate failure
 * Function: reads the contents of the specified file and writes them to buf for later use
 */
int32_t read_file(int32_t fd, void* buf, int32_t nbytes){
    //if invalid buffer return -1
    if(buf == NULL){
        return -1;
    }
    struct PCB_t *curr_pcb = (struct PCB_t *)(MB8 - KB8*(curr_pid_for_term[scheduled_term]+1));       //get a pointer to the pcb based off of pid
    int out_data = 0;
    struct inode_t* inode = (struct inode_t*)(boot_block+1+curr_pcb->fdArray[fd].inode_check);     //caluclate the inode pointer
    if(curr_pcb->fdArray[fd].flags == full){                        //if the fd is present in the pcb
        if(curr_pcb->fdArray[fd].file_position+nbytes>=inode->length){      //if the position plus bytes to write is greater than the size, set bytes to write to the remaining bytes in the file
            nbytes = inode->length - curr_pcb->fdArray[fd].file_position;   //set nbytes to remaining amount of bytes in file
        }
        out_data = read_data (curr_pcb->fdArray[fd].inode_check, curr_pcb->fdArray[fd].file_position, (uint8_t*)buf, nbytes);      //call read data on the given fd
        if(out_data == -1){                 // if there was any error in reading data return -1
            return -1;
        }
        curr_pcb->fdArray[fd].file_position+=out_data;      //increment file position with the amount of bytes read
        return out_data;
    }

    return -1;        //return -1 if fd empty
}

/* write_file(int32_t fd, uint8_t* buf, int32_t nbytes)
 * Inputs: file name, buf, nbytes
 * Return Value: 0 to indicate sucess and -1 to indicate failure
 * Function: does nothing
 */
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes){
    return -1;              // set to -1 as we don't perform write operation on files
}

