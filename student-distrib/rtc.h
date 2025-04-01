#ifndef _RTC_H
#define _RTC_H
#define IRQ8                        0x08
#include "lib.h"
#include "i8259.h"

// Port 0x70 is used to specify an index or "register number"
// Port 0x71 is used to read or write from/to that byte of CMOS configuration space
// define macros for these
#define RTC_PORT  0x70
#define CMOS_WRITE 0x71
#define FREQ_1024 1024
#define FREQ_512 512
#define FREQ_256 256
#define FREQ_128 128
#define FREQ_64 64
#define FREQ_32 32
#define FREQ_16 16
#define FREQ_8 8
#define FREQ_4 4
#define FREQ_2 2
#define FREQ_1 1
#define rtcInt 8
//function to re enable masked interrupts

//initializes rtc, by enabling irq8
void rtc_init();
// handles rtc interrupts
void rtc_handler();
// initializes rtc to 2hz
int rtc_open();
// returns once a specific number of rtc interrupts have occured. The freq_count variable determines how many rtc interrups we have to wait for in order for rtc_read to return
int rtc_read(int fd, void* buf, int nbytes);
// changes the rtc frequency
int rtc_write(int fd, const void* buf, int nbytes);
// closes rtc
int rtc_close();

#endif /* _RTC_H */
