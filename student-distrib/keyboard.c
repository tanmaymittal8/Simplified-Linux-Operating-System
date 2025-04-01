#include "lib.h"
#include "keyboard.h"
#include "i8259.h"
#include "terminal.h"
#include "paging.h"
#include "scheduling.h"

#define CTRL_STATE      0   // ctrl_info index for the ctrl key state array, 0 not pressed and 1 is pressed
#define KEY_PRESS       1   // ctrl_info index for the ctrl key state array, stores the associated key press
#define EMPTY           0   // inital flag value
#define SET             1   // set flag values

/* All local helper functions */
void enter_helper();            // keyboard response to enter
void backspace_helper();        // keyboard response to backspace
void control_helper(int key_state);     // keyboard respone to control key
void space_helper();                    // keyboard response to space key
void tab_helper();                      // keyboard response to tab key
void capslock_helper();                 // keyboard response to capslock key
void shift_helper(int key_state);       // keyboard response to shift helper key
void alt_helper(int key_state);         // keyboard response to alt key
void f_helper(int key_state, int f_number);     
int32_t scancode_helper(int scan_data);     

/* Main keyboard buffer */
unsigned char keyboard_buf[BUFSIZE];
/* Keyboard buffers for each terminal */
unsigned char keyboard_buf1[BUFSIZE];
unsigned char keyboard_buf2[BUFSIZE];
unsigned char keyboard_buf3[BUFSIZE];

/* All keyboard flags, initialized to be 0 meaning empty */
uint8_t ctrl_info[2]    = {EMPTY, EMPTY};    // {state of ctrl key, key pressed while ctrl was pressed}
uint8_t shift_flag      = EMPTY;             // initialize shift key flag
uint8_t capslock_flag   = EMPTY;             // initialize capslock key flag
uint8_t capital_flag    = EMPTY;             // initalize capital key flag
int32_t alt_info        = EMPTY;            // initialize alt key flag
int32_t f1_info         = EMPTY;            // initalize terminal 1 switch flag
int32_t f2_info         = EMPTY;            // initalize terminal 2 switch flag
int32_t f3_info         = EMPTY;            // initalize terminal 3 switch flag

int32_t recentTermChoice[2] = {1,1}; // {prev choice, curr choice}, initialized to terminal 1
int32_t buf_idx = 0;                /* The current keyboard buffer offset from 0 */  

/* keyboard_switch
 * Inputs: The previous terminal and the current terminal number
 * Return Value: none
 * Function: Stores current keyboard buffer and switches to desired terminals keyboard buffer
 */
void keyboard_switch(uint32_t prev_terminal, uint32_t curr_terminal){
    switch (prev_terminal) { // save the data contents and flags of the previous keyboard buffer into a perv_terminal specific buffer
        case 1:                                             // terminal one
            memcpy(keyboard_buf1, keyboard_buf, BUFSIZE); 
            break;
        case 2:                                             // terminla two
            memcpy(keyboard_buf2, keyboard_buf, BUFSIZE);
            break;
        case 3:                                             // terminal three
            memcpy(keyboard_buf3, keyboard_buf, BUFSIZE);
            break;
        }

    switch (curr_terminal) { // copy data from a curr_term specific buffer into the active buffer 
        case 1:                                             // terminal one
            memcpy(keyboard_buf, keyboard_buf1, BUFSIZE);
            break;
        case 2:                                             // terminal two
            memcpy(keyboard_buf, keyboard_buf2, BUFSIZE);
            break;
        case 3:                                             // terminal three
            memcpy(keyboard_buf, keyboard_buf3, BUFSIZE);
            break;
    }
}


/* Characters corresponding to keyboard scancodes based of of US QWEURTY keyboard to 54 characters */
char scan_code_one[54] = {
    0x00, 0x00, '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '-', '=', 0x08, ' ', 'q', 'w', 'e', 'r', 't', 'y', 
    'u', 'i', 'o', 'p', '[', ']', NEW_LINE_ASCII, 0x00, 'a', 's', 'd',
    'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0x00, 0x00, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'
};

/* Characters corresponding to keyboard scancodes when shift is pressed on a US QWEURTY keyboard to 54 characters */
char scan_code_one_shift[54] = {
    0x00, 0x00, '!', '@', '#', '$', '%', '^', '&', '*', '(',
    ')', '_', '+', 0x08, ' ', 'Q', 'W', 'E', 'R', 'T', 'Y', 
    'U', 'I', 'O', 'P', '{', '}', NEW_LINE_ASCII, 0x00, 'A', 'S', 'D',
    'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 0x00, 0x00, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'
};

/* Characters corresponding to keyboard scancodes when shift is pressed on a US QWEURTY keyboard to 54 characters */
char scan_code_one_caps[54] = {
    0x00, 0x00, '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '-', '=', 0x08, ' ', 'Q', 'W', 'E', 'R', 'T', 'Y', 
    'U', 'I', 'O', 'P', '[', ']', NEW_LINE_ASCII, 0x00, 'A', 'S', 'D',
    'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', 0x00, 0x00, '\\',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/'
};

/* Characters corresponding to keyboard scancodes when shift and capslock is pressed on a US QWEURTY keyboard to 54 characters */
char scan_code_one_shift_caps[54] = {
    0x00, 0x00, '!', '@', '#', '$', '%', '^', '&', '*', '(',
    ')', '_', '+', 0x08, ' ', 'q', 'w', 'e', 'r', 't', 'y', 
    'u', 'i', 'o', 'p', '{', '}', NEW_LINE_ASCII, 0x00, 'a', 's', 'd',
    'f', 'g', 'h', 'j', 'k', 'l', ':', '"', 0x00, 0x00, '|',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?'
};

/* void buffer_clear()
 * Inputs: nothing
 * Return Value: none
 * Function: Clears the keyboard buffer and resets the index
 */
void buffer_clear() {
    if(keyboard_flags[TERMINAL_ONE_IDX].ctrl_info[KEY_PRESS] != L_PRESS && recentTermChoice[CURR_TERM] == TERM_ONE) {   // check for clear command term 1
        keyboard_buf[buf_idx] = NEW_LINE_ASCII;
        keyboardputc(NEW_LINE_ASCII);    // update page table
    }
    if(keyboard_flags[TERMINAL_TWO_IDX].ctrl_info[KEY_PRESS] != L_PRESS && recentTermChoice[CURR_TERM] == TERM_TWO) {   // check for clear command in term 2
        keyboard_buf[buf_idx] = NEW_LINE_ASCII;
        keyboardputc(NEW_LINE_ASCII);       // update page table
    }
    if(keyboard_flags[TERMINAL_THREE_IDX].ctrl_info[KEY_PRESS] != L_PRESS && recentTermChoice[CURR_TERM] == TERM_THREE) {   // check for clear command in term 3
        keyboard_buf[buf_idx] = NEW_LINE_ASCII;
        keyboardputc(NEW_LINE_ASCII);       // update page table
    }
    buf_idx = 0;

    int x;
    for(x = 0; x < sizeof(keyboard_buf); x++) { // clears the keyboard buffer
        keyboard_buf[x] = '\0';                         // clears active keyboard buffer
        if(recentTermChoice[CURR_TERM] == TERM_ONE) {   // checks if scheduler is on terminal 1 and then clears term1 buffer
            keyboard_buf1[x] = '\0';                    // clear terminal
        }
        if(recentTermChoice[CURR_TERM] == TERM_TWO) {  // checks if scheduler is on terminal 2 and then clears term2 buffer
            keyboard_buf2[x] = '\0';                    // clear terminal
        }
        if(recentTermChoice[CURR_TERM] == TERM_THREE) {  // checks if scheduler is on terminal 3 and then clears term3 buffer
            keyboard_buf3[x] = '\0';                        // clear terminal
        }
    }
}

/* void buffer_clear_only()
 * Inputs: nothing
 * Return Value: none
 * Function: Clears the keyboard buffer and resets the index
 */
void buffer_clear_only() {
    buf_idx = 0;

    int x;
    for(x = 0; x < sizeof(keyboard_buf); x++) { // clears the keyboard buffer
        keyboard_buf[x] = '\0';                         // clears active keyboard buffer
        if(scheduled_term == TERM_ONE) {   // checks if scheduler is on terminal 1 and then clears term1 buffer
            keyboard_buf1[x] = '\0';                    // clear terminal
        }
        if(scheduled_term == TERM_TWO) {  // checks if scheduler is on terminal 2 and then clears term2 buffer
            keyboard_buf2[x] = '\0';                    // clear terminal
        }
        if(scheduled_term == TERM_THREE) {  // checks if scheduler is on terminal 3 and then clears term3 buffer
            keyboard_buf3[x] = '\0';                        // clear terminal
        }
    }
}

/* void keyboard_init();
 * Inputs: nothing
 * Return Value: none
 * Function: initializes keyboard variables, by enabling irq1
 */
void keyboard_init() {
    // initilize the flags to 0
    _enter_flag[0] = 0;     // initilize enter flag to 0 corresponding to term 1
    _enter_flag[1] = 0;     // initilize enter flag to 0 corresponding to term 2
    _enter_flag[2] = 0;     // initilize enter flag to 0 corresponding to term 3
    ctrl_info[0] = 0;       // initilize control key press flag to 0 
    ctrl_info[1] = 0;       // initalize control key plus another key pressed flag to 0        
    shift_flag = 0;   
    ctrl_info[1] = 0; 

    // initilize the control flags for all the terminals
    keyboard_flags[0].ctrl_info[0] = 0; 
    keyboard_flags[0].ctrl_info[1] = 0;     
    keyboard_flags[1].ctrl_info[0] = 0; 
    keyboard_flags[1].ctrl_info[1] = 0;
    keyboard_flags[2].ctrl_info[0] = 0; 
    keyboard_flags[2].ctrl_info[1] = 0;     

    // initilize the shift flags for all the terminals
    shift_flag = 0;     // active shift flag set to 0
    keyboard_flags[0]._shift_flag = 0;  // set term 1 shift flag to 0
    keyboard_flags[1]._shift_flag = 0;  // set term 2 shift flag to 0
    keyboard_flags[2]._shift_flag = 0;  // set term 3 shift flag to 0

    // initilize the capslock flags for all the terminals
    capslock_flag = 0;  // active capslock flag set to 0
    keyboard_flags[0]._capslock_flag = 0;
    keyboard_flags[1]._capslock_flag = 0;
    keyboard_flags[2]._capslock_flag = 0;

    // initilize the capital flags for all the terminals
    capital_flag = 0;
    keyboard_flags[0]._capital_flag = 0;
    keyboard_flags[1]._capital_flag = 0;
    keyboard_flags[2]._capital_flag = 0;

    buf_idx = 0;    // initilize keyboard buffer index to 0
    enable_irq(IRQ1);   // enable keyboard interrupt
}

/* keyboard_handler
 * Inputs: none
 * Return Value: none 
 * Function: Handles all keyboard inputs
 */
void keyboard_handler() {
    cli();
    uint8_t keyboard_scan_data = inb(DATA_PORT);    // read keboard press data

    /* Handle flags and special key presses */
    switch(keyboard_scan_data) {
        case ENTER_PRESS:   // check if enter pressed
            enter_helper();
            return;
        case BACKSPACE_PRESS:   // check if backspace pressed
            backspace_helper();
            return;
        case LEFT_CONTROL_PRESS:    // check if left control pressed
            control_helper(KEY_PRESS);
            return;
        case LEFT_CONTROL_RELEASE:      // check if left control released
            control_helper(KEY_RELEASE);
            return;
        case SPACE_PUSH:            // check if space pressed
            space_helper();
            return;
        case TAB_PUSH:          // check if tab pressed
            tab_helper();
            return;
        case CAPSLOCK_PUSH:     // check if capslock pressed
            capslock_helper();
            return;
        case LEFT_SHIFT_PUSH:   // check if left shift pressed
            shift_helper(KEY_PRESS);
            return;
        case LEFT_SHIFT_RELEASE:    // check if left shift released
            shift_helper(KEY_RELEASE);
            return;
        case RIGHT_SHIFT_PUSH:      // check if right shift pressed
            shift_helper(KEY_PRESS);
            return;
        case RIGHT_SHIFT_RELEASE:   // checkif right shift released
            shift_helper(KEY_RELEASE);
            return;
        case ALT_PUSH:                  // check if alt pressed
            alt_helper(KEY_PRESS);
            return;
        case ALT_RELEASE:               // check if alt released
            alt_helper(KEY_RELEASE);
            return;
        case F1_PUSH:                   // check if f1 pressed
            f_helper(KEY_PRESS, 1);     // terminal 1
            return;
        case F1_RELEASE:                // check if f2 released
            f_helper(KEY_RELEASE, 1);   // terminal 1
            return;
        case F2_PUSH:                   // check if f2 pressed
            f_helper(KEY_PRESS, 2);     // terminal 2
            return;
        case F2_RELEASE:                // check if f2 released
            f_helper(KEY_RELEASE, 2);   // terminal 2
            return;
        case F3_PUSH:                   // check if f3 pressed
            f_helper(KEY_PRESS, 3);     // terminal 3
            return;
        case F3_RELEASE:                // check if f3 released
            f_helper(KEY_RELEASE, 3);   // terminal 3
            return;
        default:
            break;
    }

    /* Shortcut ctrl + l clears screen */ 
    if(ctrl_info[CTRL_STATE] == 1 && keyboard_scan_data == L_PRESS) { // check if control key and l pressed to clear screen
        clear();
        int i;
        for(i = 0; i < BUFSIZE; i++){   // loop throgh the entire keyboard buffer
            if(keyboard_buf[i] != '\0') {   // clear buffer
                keyboardputc(keyboard_buf[i]);  // update page table
            }
        }
        clear_help();   // update the cursor
        ctrl_info[KEY_PRESS] = L_PRESS; // set the second key pressed with control as l

        /* End interrupt */
        send_eoi(IRQ1); 
        sti();
        return;
    }

    /* If the max characters are reached*/
    /* The -1 is to accomadate for the newline ascii added at the end */
    if(buf_idx >= BUFSIZE - 1) {
        /* End interrupt */
        send_eoi(IRQ1);
        sti();
        return;
    }

    int32_t curr_scancode = scancode_helper(keyboard_scan_data);  // read keyboard input
    if(curr_scancode == -1) {
        // Upon unrecognized scancode, do nothing
    }
    else {
        keyboard_buf[buf_idx] = curr_scancode;          // populate the keyboard buffer
        keyboardputc((uint8_t) keyboard_buf[buf_idx]);
        buf_idx++;
    }
    /* End interrupt */
    send_eoi(IRQ1);
    sti();
    return;
}

/* enter_helper
 * Inputs: none
 * Return Value: none 
 * Function: Handles when enter is pressed by updating relevant flags and buffers
 */
void enter_helper() {
    if(ctrl_info[KEY_PRESS] == SET) {  // if control key pressed
        keyboardputc('\n');             // update paging table
    }
    ctrl_info[KEY_PRESS] = EMPTY;   // reset control key

    keyboard_buf[buf_idx] = 0x0A; // '\n'
    buf_idx++;
    _enter_flag[currentTerminal - 1] = SET; // currentTerminal is not 0 indexed
    int i;
    for(i = 0; i < BUFSIZE; i++){
        term_buf[i] = keyboard_buf[i];  // copy keyboard buffer into terminal buffer
    }
    buffer_clear();     // empty out keyboard buffer
    send_eoi(IRQ1);     // end of interrupt
    sti();
}

/* backspace_helper
 * Inputs: none
 * Return Value: none 
 * Function: When delete is pressed, remove a character
 */
void backspace_helper() {
    if(buf_idx > 0) {
        buf_idx--;      // decriment buffer index
        keyboardputc(BACKSPACE_ASCII);  // update page table
        keyboard_buf[buf_idx] = '\0';   // clear out backspaced space in keyboard buffer
    }
    send_eoi(IRQ1); // end of interrupt
    sti();
}

/* control_helper
 * Inputs: The press state of the control key
 * Return Value: none
 * Function: Updates control key related flags
 */
void control_helper(int key_state) {
    if(key_state == KEY_PRESS) {    // check if a key pressed
        ctrl_info[CTRL_STATE] = KEY_PRESS;  // update control array with key pressed
    }
    else if(key_state == KEY_RELEASE) {  // chekck if key released
        ctrl_info[CTRL_STATE] = KEY_RELEASE;    // update control array wiht key released
    }
    send_eoi(IRQ1);     // send end of interrupt
    sti();
}

/* space_helper
 * Inputs: none
 * Return Value: none
 * Function: Updates space key related flags
 */
void space_helper() {
    keyboard_buf[buf_idx] = SPACE_ASCII;    // adds space key ascii char to keyboard buffer
    keyboardputc((uint8_t) keyboard_buf[buf_idx]);  // update page table
    buf_idx++;
    send_eoi(IRQ1); // end of interrupt
    sti();
}

/* tab_helper
 * Inputs: none
 * Return Value: none
 * Function: When tab is pressed put four spaces onto terminal
 */
void tab_helper() {
    int i;
    for(i = 0; i < TAB_LENGTH; i++) { // loop through tab length
        /* The -1 accounts for the newline character */
        if(buf_idx >= BUFSIZE - 1) {    // check if tab applyed
            // buffer_clear();

            /* End interrupt */
            send_eoi(IRQ1);
            sti();
            return;
        }
        else{
            space_helper(); // add space
        }
    }
    send_eoi(IRQ1); // end of interrupt
    sti();
}

/* capslock_helper
 * Inputs: none
 * Return Value: none
 * Function: When capslock is pressed update relevant flags
 */
void capslock_helper() {
    if(capslock_flag == EMPTY) { // check if capslock flag not set
        capslock_flag = SET;    // set capslock flag
    }
    else {
        capslock_flag = EMPTY;  // reset caps lock flag
    }
    send_eoi(IRQ1); // end of interrupt
    sti();
}

/* shift_helper
 * Inputs: The press state of the shift key
 * Return Value: none
 * Function: When shift key is pressed or released update relevant flags
 */
void shift_helper(int key_state) {
    if(key_state == KEY_PRESS) {  // check if shift key pressed
        capital_flag = KEY_PRESS;   // set capital key flag
    }
    else if(key_state == KEY_RELEASE) { // check if shift key released
        capital_flag = KEY_RELEASE;     // reset capital key flag
    }
    send_eoi(IRQ1); // end of interrupt
    sti();
}

/* alt_helper
 * Inputs: The press state of the alt key
 * Return Value: none
 * Function: Update relevant flags when alt is pressed/released
 */
void alt_helper(int key_state) {
    if(key_state == KEY_PRESS) {    // check if alt key pressed
        alt_info = SET;         // set alt key
    }
    else if(key_state == KEY_RELEASE) {     // check if alt key released
        alt_info = EMPTY;       // reset alt key
    }
    send_eoi(IRQ1);     // end of interrupt
    sti();
}

/* f_helper
 * Inputs: The press state of the f# function keys and which f# was pressed
 * Return Value: none
 * Function: Update relevant flags and update scheduling related variables to switch terminals with on key press/release
 */
void f_helper(int key_state, int f_number) {
    if(key_state == KEY_PRESS) {

        switch(f_number) {
            case TERM_ONE:
                recentTermChoice[PREV_TERM] = recentTermChoice[CURR_TERM];  // set prev term to curr term int term 1
                recentTermChoice[CURR_TERM] = TERM_ONE;                     // update curr term
                break;
            case TERM_TWO:
                recentTermChoice[PREV_TERM] = recentTermChoice[CURR_TERM];  // set prev term to curr term int term 2
                recentTermChoice[CURR_TERM] = TERM_TWO;                     // update curr term
                break;
            case TERM_THREE:
                recentTermChoice[PREV_TERM] = recentTermChoice[CURR_TERM];  // set prev term to curr term int term 3
                recentTermChoice[CURR_TERM] = TERM_THREE;                   // update curr term
                break;
        }

        if(alt_info){
            terminal_changing_handler();    // switch terminal
        }
    }
    else if(key_state == KEY_RELEASE) {     // reset terminal switch flags as soon as released
        if(f_number == TERM_ONE) {
            f1_info = EMPTY;
        }
        else if(f_number == TERM_TWO) {
            f2_info = EMPTY;
        }
        else if(f_number == TERM_THREE) {
            f3_info = EMPTY;
        }
    }
    send_eoi(IRQ1);     // end of interrupt
    sti();
}

/* scancode_helper
 * Inputs: The current scancode data value from keyboard
 * Return Value: The ascii value for each scancode
 * Function: Based on the current flags, returns the character from the proper scancode table
 */
int32_t scancode_helper(int scan_data) {

    if(((scan_data) > sizeof(scan_code_one))){
        return -1;
    }

    int32_t keyboard_character = EMPTY;
    if (capslock_flag == SET && capital_flag == EMPTY){     // if only capslock flag set
        
       keyboard_character = scan_code_one_caps[scan_data];  // read upper case letters
    }
    else if(capital_flag == SET && capslock_flag == EMPTY) {    // check if only capital flag set
        
       keyboard_character = scan_code_one_shift[scan_data]; // data with capital and special chars
    }
    else if (capital_flag == SET && capslock_flag == SET){      // check if both capital and caps lock flag on
        
       keyboard_character = scan_code_one_shift_caps[scan_data];    // data with lowercase letters and special chars
    }
    else {
        
       keyboard_character = scan_code_one[scan_data];   // data with lowercase letters
    }
    return keyboard_character;
}

/* terminal_changing_handler
 * Inputs: none
 * Return Value: none 
 * Function: Changes the current viewing terminal and related variables upon terminal switch initiation
 */
void terminal_changing_handler() {
    set_paging_scheduling(recentTermChoice[CURR_TERM]); // set scheduling for current terminal
    terminal_changing();                                // switch terminals
    keyboard_switch(recentTermChoice[PREV_TERM], recentTermChoice[CURR_TERM]);  // save data of prev terminal and load data for curr terminal
    memcpy(terminal_switch_array[recentTermChoice[PREV_TERM]-1]._term_buf, term_buf, BUFSIZE);  // save data from old terminal
    terminal_switch_array[recentTermChoice[PREV_TERM]-1]._buf_idx = buf_idx;                    // save old buf index
    update_cursor(termScreenx[recentTermChoice[CURR_TERM]-1],termScreeny[recentTermChoice[CURR_TERM]-1]);   // set new cursor val
    memcpy(term_buf, terminal_switch_array[recentTermChoice[CURR_TERM]-1]._term_buf, BUFSIZE);  // load any old data for curr terminal
    buf_idx = terminal_switch_array[recentTermChoice[CURR_TERM]-1]._buf_idx;                    // load saved buffer index
}
