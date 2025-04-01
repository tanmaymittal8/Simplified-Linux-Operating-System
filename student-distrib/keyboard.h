#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"
#include "i8259.h"
#include "types.h"
#include "terminal.h"

//initialize some macros for ports
#define DATA_PORT                   0x60
#define READ_WRITE                  0x64
#define status_reg_init             0x00
#define command_reg_init            0x00

#define BUFSIZE                     128
#define IRQ1                        0x01
#define BACKSPACE_ASCII             0x08
#define NEW_LINE_ASCII              0x0A
#define SPACE_ASCII                 0x20
#define TAB_ASCII                   0x11
#define SCREEN_LENGTH               80
#define TAB_LENGTH                  4
#define KEY_PRESS                   1
#define KEY_RELEASE                 0
#define MAX_TERMINALS               3

/* Scancodes */
#define BACKSPACE_PRESS             0x0E 
#define BACKSPACE_RELEASE           0x8E
#define LEFT_CONTROL_RELEASE        0x9D
#define LEFT_CONTROL_PRESS          0x1D
#define L_PRESS                     0x26
#define ENTER_PRESS                 0x1C
#define ENTER_RELEASE               0x9C
#define LEFT_SHIFT_PUSH             0x2A
#define RIGHT_SHIFT_PUSH            0x36
#define LEFT_SHIFT_RELEASE          0xAA
#define RIGHT_SHIFT_RELEASE         0xB6
#define SPACE_PUSH                  0x39
#define TAB_PUSH                    0x0F
#define CAPSLOCK_PUSH               0x3A
#define CAPSLOCK_RELEASE            0xBA
#define ALT_PUSH                    0x38
#define ALT_RELEASE                 0xB8
#define F1_PUSH                     0x3B
#define F1_RELEASE                  0xBB
#define F2_PUSH                     0x3C
#define F2_RELEASE                  0xBC
#define F3_PUSH                     0x3D
#define F3_RELEASE                  0xBD

/* Struct containing keyboard flags */
typedef struct flags_t {
    uint8_t ctrl_info[2];
    uint8_t _shift_flag;
    uint8_t _capslock_flag;
    uint8_t _capital_flag;
    int32_t alt_info;
} flags_t;

struct flags_t keyboard_flags[MAX_TERMINALS]; // Array to keep track of keyboard flags across different terminals

volatile int _enter_flag[MAX_TERMINALS];    // Array to keep track of enter flags across terminals for use in terminal write/read

extern unsigned char keyboard_buf[BUFSIZE];
extern int buf_idx;
extern void putc(uint8_t c);
extern void terminal_changing_handler();
extern int32_t recentTermChoice[2]; 

/* void buffer_clear()
 * Inputs: nothing
 * Return Value: none
 * Function: Declares the function to clear the buffer and resest relevant vars
 */
void buffer_clear();
void buffer_clear_only();

/* void keyboard_handler()
 * Inputs: nothing
 * Return Value: none
 * Function: Declares function that handles keyboard inputs
 */
void keyboard_handler();

/* void keyboard_init();
 * Inputs: nothing
 * Return Value: none
 * Function: Declare function that initializes keyboard by enabling irq1
 */
void keyboard_init();

#endif /*_KEYBOARD_H */
