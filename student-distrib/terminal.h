#ifndef _TERMINAL
#define _TERMINAL

#include "types.h"
#include "lib.h"

#define backspace_scan  0x0E
#define BUFSIZE         128
#define NEWLINE_ASCII   0x0A

#define TERMINAL_ONE_IDX    0
#define TERMINAL_TWO_IDX    1
#define TERMINAL_THREE_IDX  2
#define TERM_ONE        1
#define TERM_TWO        2
#define TERM_THREE      3
#define CLOSE_TERMINAL  0
#define OPEN_TERMINAL   1
#define CURR_TERM       1
#define PREV_TERM       0

unsigned char term_buf[BUFSIZE];
extern int termReadFlag[3];     // flag for all three terminals

/* int terminal_open()
 * Inputs: none
 * Return Value: none 
 * Function: Initializes terminal variables
 */
int terminal_open();

/* int terminal_close()
 * Inputs: none
 * Return Value: none
 * Function: closes
 */
int terminal_close();

/* int terminal_read(unsigned char* buf, int nbytes)
 * Inputs: 
 * Return Value: Number of bytes read
 * Function: Reads from the keyboard buffer into buf
 */
int terminal_read(int fd,void* buf, int nbytes);

/* int terminal_write(unsigned char* buf, int nbytes)
 * Inputs: Pointer to first character to print in keyboard buffer and amount of bytes to write
 * Return Value: Amount of bytes written, otherwise -1 upon failure
 * Function: Output a character to the console
 */
int terminal_write(int fd, const void* buf, int nbytes);

// Struct to store relevant terminal info when switching terminals
typedef struct terminal_info_t {
    int32_t is_terminal_running;
    // Video info
    int32_t _screen_x;
    int32_t _screen_y;
    int8_t _video_mem[(NUM_COLS * NUM_ROWS * 2) / 8]; // Times 2 to account for attrib and div 8 because want bits, not bytes  
    // Keyboard info
    int32_t _buf_idx;
    uint8_t _keyboard_buf[BUFSIZE];

    uint8_t _term_buf[BUFSIZE];
} terminal_info_t;

// Store the data for up to 3 different terminals
struct terminal_info_t terminal_switch_array[3];
extern int32_t curr_terminal_idx;


#endif /* _TERMINAL */
