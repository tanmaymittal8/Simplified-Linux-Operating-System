#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "types.h"
#include "scheduling.h"

//initialize all of our local variables, some of being an list of 3 with one for each terminal
int max_freq = 1024;                //
int curr_freq[3]={1024,1024,1024};  // the current frequency for each virtualized rtc for each terminal
int curr_count[3] = {1,1,1};        // count variables to keep track of number of rtc interrupts that have transpired(again for each terminal). We use this to virtualize
int freq_count[3] = {1,1,1};        // based on what the frequency is for each rtc, we have to keep track of how many interrupts have to occur to register as 1 virtualized rtc interrupt
int rate = 3;                       // used in rtc_init to set the rtc to the max frequency in the initialization
int flag[3] = {0,0,0};              //flag to indicate to preform action

/* void rtc_init();
 * Inputs: nothing
 * Return Value: none
 * Function: initializes rtc, by enabling irq8
 */
void rtc_init(){
    //initialize rtc
    char prev;
    outb( 0x8B,RTC_PORT);		// select register B, and disable NMI
    prev=inb(CMOS_WRITE);	// read the current value of register B
    outb(0x8B,RTC_PORT);		// set the index again (a read will reset the index to register D)
    outb( prev | 0x40,CMOS_WRITE);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
    outb(0x8A, RTC_PORT);		// set index to register A, disable NMI
    prev=inb(CMOS_WRITE);	// get initial value of register A
    outb(0x8A, RTC_PORT);		// reset index to A
    outb( (prev & 0xF0) | rate, CMOS_WRITE); //write only our rate to A. Note, we set rate to 3 since that makes the frequency 1024, which for our purposes is the max frequency
    enable_irq(IRQ8);  // enable the 8th post of the pic for the rtc
    // enable previously disable non maskable interrupts
    outb(inb(RTC_PORT) & 0x7f,RTC_PORT); // clears msb and retains the data for all other bits
    inb(CMOS_WRITE);
}

/* void rtc_handler();
 * Inputs: nothing
 * Return Value: none
 * Function: handles rtc interrupts
 */
void rtc_handler(){
    int regC;
    outb(0xC,RTC_PORT);     // get register c data
    regC = inb(CMOS_WRITE); // flush the data in register c
    int loop;               //setup the looping variable to loop through the 3 indexes of our variables
    for(loop = 0;loop<3;loop++){                    //for all the terminals variables
        if(curr_count[loop] == 0){                  //if counter reached 0, we should now set the flag high
            curr_count[loop] = freq_count[loop];    //reset the count to the frequency
            flag[loop] = 1;                         //set the flag high for rtc read
        }
        else{ //this decrements the count variable in order to keep track of how many interrupts have taken place, used in rtc_read
            curr_count[loop]--;
        }
    }
    send_eoi(rtcInt);            // end rtc interrupt
} 
/* int rtc_read();
 * Inputs: int fd, const void* buf, int nbytes
 * Return Value: always returns 0
 * Function: returns once a specific number of rtc interrupts have occured. The freq_count variable determines how many rtc interrups we have to wait for in order for rtc_read to return
 */
int rtc_read(int fd, void* buf, int nbytes){
    flag[scheduled_term] = 0;           //reset the flag to 0
    while(flag[scheduled_term] != 1){}  //empty while loop which waits for the count to hit 0 so that we know that the appropriate number of interrupts have taken place
    return 0;
}

/* int rtc_open();
 * Inputs: none
 * Return Value: always returns 0
 * Function: initializes rtc to 2hz
 */
int rtc_open(){
    //initialize frequency to 2hz
    //since the rate is set to 3 which is the max frequency(1024 hz), we have to set the count to be the appropriate value so that rtc_read works properly
    freq_count[scheduled_term] = 512; //since we set the frequency to 2 hz we set the count to 512(we wait for 512 interrupts in rtc_read before we let the program continue)
    curr_freq[scheduled_term] = 2; //set the current frequency to 2 hz
    return 0;
}
/* int rtc_write(int fd, const void* buf, int nbytes)
 * Inputs: int fd, const void* buf, int nbytes
 * Return Value: returns 0 if successfully rewrote frequency, returns -1 if not
 * Function: changes the rtc frequency. We first check if the passed in frequency is a power of 2, and if it is
 * then we set the global freq_count variable to the appropriate value. This determines how many interrupts rtc_read
 * has to wait for before returning, based on the currrently set frequency
 */
int rtc_write(int fd, const void* buf, int nbytes){
    if(buf == NULL){ // null checking
        return -1;
    }
    if(nbytes != sizeof(uint32_t)){ // checks if nbytes is a valid argument
        return -1;
    }
    //just take the first element in buffer
    int frequency = *((uint32_t *)buf);
    int temp = frequency;
    if(frequency < 1 || frequency > max_freq){ //bound checking frequencies
        return -1; //return -1 for invalid frequencies
    }
    //check whether passed in frequency is a power of 2
    while(temp % 2 ==0){
        temp /= 2;
    }
    if(temp == 1){ // this means that the passed in frequency is a power of 2
        curr_freq[scheduled_term] = frequency;
        //calculate the freq_count(the number of interrupts that have to pass based on the set frequency)
        if(curr_freq[scheduled_term] == FREQ_1024) // for example if we set frequency to 1024 hz we set the count to 1, so we count for one interrupt in order for rtc_read to finish executing
            freq_count[scheduled_term] = FREQ_1;
        else if(curr_freq[scheduled_term] == FREQ_512)
            freq_count[scheduled_term] = FREQ_2;
        else if(curr_freq[scheduled_term] == FREQ_256)
            freq_count[scheduled_term] = FREQ_4;
        else if(curr_freq[scheduled_term] == FREQ_128)
            freq_count[scheduled_term] = FREQ_8;
        else if(curr_freq[scheduled_term] == FREQ_64)
            freq_count[scheduled_term] = FREQ_16;
        else if(curr_freq[scheduled_term] == FREQ_32)
            freq_count[scheduled_term] = FREQ_32;
        else if(curr_freq[scheduled_term] == FREQ_16)
            freq_count[scheduled_term] = FREQ_64;
        else if(curr_freq[scheduled_term] == FREQ_8)
            freq_count[scheduled_term] = FREQ_128;
        else if(curr_freq[scheduled_term] == FREQ_4)
            freq_count[scheduled_term] = FREQ_256;
        else if(curr_freq[scheduled_term] == FREQ_2)
            freq_count[scheduled_term] = FREQ_512;
        else if(curr_freq[scheduled_term] == FREQ_1)
            freq_count[scheduled_term] = FREQ_1024;
        return 0;
    }
    else{
        return -1; //return -1 if the passed in frequency isn't a power of 2
    }
    return 0;
}

/* rtc_close(int fd);
 * Inputs: none
 * Return Value: always returns 0
 * Function: closes rtc
 */
int rtc_close(int fd){
    return 0;
}
