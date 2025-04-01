#include "terminal.h"
#include "keyboard.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "scheduling.h"
#include "systemCall.h"
#include "paging.h"

int termReadFlag[MAX_TERMINALS] = {0,0,0};  // initialize terminal flags for all terminals

/* Characters corresponding to keyboard scancodes based of of US QWEURTY keyboard with 54 elements*/
char scan_code_one_lower[54] = {
    0x00, 0x00, '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '-', '=', 0x08, ' ', 'q', 'w', 'e', 'r', 't', 'y', 
    'u', 'i', 'o', 'p', '[', ']', 0x0A, 0x00, 'a', 's', 'd',
    'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0x00, 0x00, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'
};

/* Characters corresponding to keyboard scancodes when capitals should be used on a US QWEURTY keyboard with 54 elements*/
char scan_code_one_upper[54] = {
    0x00, 0x00, '!', '@', '#', '$', '%', '^', '&', '*', '(',
    ')', '_', '+', 0x08, ' ', 'Q', 'W', 'E', 'R', 'T', 'Y', 
    'U', 'I', 'O', 'P', '{', '}', 0x0A, 0x00, 'A', 'S', 'D',
    'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 0x00, 0x00, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'
};

uint8_t terminal_print_flag = 0;    // initilize terminal print flag
int32_t curr_terminal_idx = 0;      // initilize curr terminal flag

/* int terminal_open()
 * Inputs: none
 * Return Value: none 
 * Function: Initializes terminal variables
 */
int terminal_open() {
    int i;
    for(i = 0; i < BUFSIZE; i++) {  // initially empty out terminal buffer
        term_buf[i] = '\0';
    }
    terminal_print_flag = 0;

    terminal_switch_array[curr_terminal_idx].is_terminal_running = OPEN_TERMINAL;   // set current terminal as active
    terminal_switch_array[curr_terminal_idx]._screen_x = 0;                         // update cursor x pos
    terminal_switch_array[curr_terminal_idx]._screen_y = 0;                         // update cursor y pos
    terminal_switch_array[curr_terminal_idx]._buf_idx = 0;                          // update buf index

    for(i = 0; i < sizeof(terminal_switch_array[curr_terminal_idx]._video_mem); i++) {
        terminal_switch_array[curr_terminal_idx]._video_mem[i] = '\0';                  // clear video mem of current terminal
    }
    for(i = 0; i < sizeof(terminal_switch_array[curr_terminal_idx]._keyboard_buf); i++) {
        terminal_switch_array[curr_terminal_idx]._keyboard_buf[i] = '\0';               // clear keyboard buf of current terminal
    }

    return 0;
}

/* int terminal_close()
 * Inputs: none
 * Return Value: none
 * Function: closes
 */
int terminal_close() {
    int i;
    for(i = 0; i < BUFSIZE; i++) {      // clear terminal buffer
        term_buf[i] = '\0';
    }

    terminal_switch_array[curr_terminal_idx].is_terminal_running = CLOSE_TERMINAL;      // reset cur terminal status

    return 0;
}


/* int terminal_read(unsigned char* buf, int nbytes)
 * Inputs: 
 * Return Value: Number of bytes read
 * Function: Reads from the keyboard buffer into buf
 */
int terminal_read(int fd, void* buf, int nbytes) {

    if(buf == NULL) {   // check if buffer empty
        return -1;
    }
    // buffer_clear_only();

    int bytes_read = 0;
    if(nbytes >= BUFSIZE) {
        nbytes = BUFSIZE - 1;
    }
    if(nbytes <= 0) {   // check if nbytes less than 0
        return -1;
    }
    termReadFlag[scheduled_term] = 1;       // set read flag for scheduled terminal
    while(_enter_flag[scheduled_term] != 1);    // while not entered
    termReadFlag[scheduled_term] = 0;       // reset term read flag for scheduled terminal
    _enter_flag[scheduled_term] = 0;        // reset enter flag for scheduled terminal
    int i;
    int newLineCheck = -1;
    for(i = 0; i <= nbytes; i++) {
        *((unsigned char *)(buf+i)) = term_buf[i];  // update buf
        if(newLineCheck == -1){     // cout the total number of bytes read before newline
            bytes_read++;
        }
        if(term_buf[i] == '\n'){    // check if newline requested
            newLineCheck = 1;
        }
    } 
    return bytes_read;
}

/* int terminal_write(unsigned char* buf, int nbytes)
 * Inputs: Pointer to first character to print in keyboard buffer and amount of bytes to write
 * Return Value: Amount of bytes written, otherwise -1 upon failure
 * Function: Output a character to the console
 */
int terminal_write(int fd, const void* buf, int nbytes) {

    if(buf == NULL) {   // check if buf empty
        return -1;
    }

    if(nbytes <= 0) {   // check if nbytes less than 0
        return -1;
    }

    int bytes_written = 0;
    uint8_t new_char;
    int i;
    for(i = 0; i < nbytes; i++) {
        new_char = *((unsigned char *)(buf+i));     // update new char from buf
        if(new_char != '\0') {
            putc(new_char);     // print new char if its not end of input special charactor
        }
        bytes_written++;        // update counter
    }
    return 0;
}
