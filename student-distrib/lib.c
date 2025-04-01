/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "keyboard.h"
#include "paging.h"
#include "scheduling.h"
#include "page_enable.h"

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x0a

//initialize some variables
static char* video_mem = (char *)VIDEO;
//flag to see if putc should act from keyboard or scheduled process
int keyboardPutCFlag = 0;
//screenc and screen y for each of the terminals
int termScreenx[3] = {0,0,0};
int termScreeny[3] = {0,0,0};

/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    //save where the video memory is mapped to
    int oldAddr = page_table[VIDEO/FourKB].p_address;
    //maps the video memory page table to point to the actual video memory
    page_table[VIDEO/FourKB].p_address = VIDEO/FourKB;
    tlbFlush();
    int32_t i; //initialize a looping variable
    //loop through the screen
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        //clear the position
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
    //reset the screenx and screen y for the screen terminal to 0
    termScreenx[currentTerminal-1] = 0;
    termScreeny[currentTerminal-1] = 0;
    //update the cursor since we updated
    update_cursor();
    //restore the video memory page table mapping
    page_table[VIDEO/FourKB].p_address =oldAddr;
    tlbFlush();
}

/* void clear_help(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear_help() {
    termScreenx[currentTerminal-1] = 0 + buf_idx;
    termScreeny[currentTerminal-1] = 0;
    update_cursor();
}

int32_t getScreenX() {
    return termScreenx[currentTerminal-1];
}
int32_t getScreenY() {
    return termScreeny[currentTerminal-1];
}
int8_t* getVideoMem() {
    return video_mem;
}
void setScreenX(int32_t _screen_x) {
    termScreenx[currentTerminal-1] = _screen_x;
}
void setScreenY(int32_t _screen_y) {
    termScreeny[currentTerminal-1] = _screen_y;
}
// void setVideoMem(int8_t* _video_mem) {
//     video_mem = _video_mem;
// }  

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console */
void putc(uint8_t c) {
    //clear interrupts
    cli();
    //initialize the variable to the scheduled term
    int putTerminal = scheduled_term;
    if(keyboardPutCFlag == 1){
        //if this function was called from keyboardputc, set the variable to the current viewing terminal index
        putTerminal = currentTerminal-1;
    }
    
    //if backspace update screenx and screeny accordingly for the visible terminal
    if(c == 0x08){
        if(termScreenx[currentTerminal-1] == 0 && termScreeny[currentTerminal-1] == 0) {
            sti();
            return;
        }
        termScreenx[currentTerminal-1]--; //decrement screenx for the visible terminal
        c = ' ';    
        //get the character on the screen(row major order) and update the character to space, bc we're backspacing that character
        *(uint8_t *)(video_mem + ((NUM_COLS * termScreeny[currentTerminal-1] + termScreenx[currentTerminal-1]) << 1)) = c; 
        *(uint8_t *)(video_mem + ((NUM_COLS * termScreeny[currentTerminal-1] + termScreenx[currentTerminal-1]) << 1) + 1) = ATTRIB;
        termScreenx[currentTerminal-1] %= NUM_COLS;                             // if screenx goes over the number of columns we reset it
        if(!(termScreenx[currentTerminal-1] == 0 && buf_idx == 0)) { 
            termScreeny[currentTerminal-1] = (termScreeny[currentTerminal-1] + (termScreenx[currentTerminal-1] / NUM_COLS)) % NUM_ROWS; 
        }
        else {
            termScreeny[currentTerminal-1]--;
        }
        update_cursor();
        sti();
        return;
    }
    //if newline increment screeny and reset screenx to 0(left side of screen)
    if(c == '\n' || c == '\r') {
        termScreeny[putTerminal]++;
        termScreenx[putTerminal] = 0;
    } 
    //get the character on the screen(row major order) and update the character to the character inputted by keyboard
    else {
        *(uint8_t *)(video_mem + ((NUM_COLS * termScreeny[putTerminal] + termScreenx[putTerminal]) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * termScreeny[putTerminal] + termScreenx[putTerminal]) << 1) + 1) = ATTRIB;
        termScreenx[putTerminal]++;
        if(termScreenx[putTerminal] >= NUM_COLS) {
            termScreenx[putTerminal] = 0;
            termScreeny[putTerminal]++;
        }
    }
    // if screeny update goes below the screen, we have to move all the rows up by 1(scrolling)
    if(termScreeny[putTerminal] > NUM_ROWS - 1) {
            int i, j;
            //moving all the rows up by 1
            for(i = 0; i < NUM_ROWS - 1; i++) {
                for(j = 0; j < NUM_COLS; j++) {
                    *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1)) = *(uint8_t *)(video_mem + ((NUM_COLS * (i + 1) + j) << 1));
                    *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1) + 1) = ATTRIB;
                }
            }
            //set the new row to null terminators
            for(j = 0; j < NUM_COLS; j++) {
                *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1)) =  '\0';
                *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1) + 1) = ATTRIB;
            }
            termScreenx[putTerminal] = 0;
            //reset screeny to the bottom of the screen
            termScreeny[putTerminal] = NUM_ROWS - 1;
    }
    //update the cursor after
    update_cursor();
    //enable interrupts
    sti();
}

/* void keyboardputc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console when called from keyboard.c. a wrapper for putc specific to keyboard */
void keyboardputc(uint8_t c) {
    //if we are not inside of a running program
    if(termReadFlag[currentTerminal-1] == 1){
        //save where the video memory is mapped to
        int oldAddr = page_table[VIDEO/FourKB].p_address;
        //maps the video memory page table to point to the actual video memory
        page_table[VIDEO/FourKB].p_address = VIDEO/FourKB;
        tlbFlush();
        //set the keyboardputc flag high 
        keyboardPutCFlag = 1;
        putc(c);
        //lower the keybaordputc flag
        keyboardPutCFlag = 0;

        //restore the video memory page table mapping
        page_table[VIDEO/FourKB].p_address =oldAddr;
        tlbFlush();
    }
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

/* void update_cursor(int x, int y)
 * Inputs: void
 * Return Value: void
 * Function: this uupdates the cursor on the screen corresponding to the current view terminal */
void update_cursor(){
    //set y to the current terminals screeny
    int y = termScreeny[currentTerminal-1];
    //set x to the current terminals screenx
    int x = termScreenx[currentTerminal-1];
    //row major order for pos
	uint16_t pos = y * NUM_COLS + x;
    //print cursor out
	outb(0x0F, 0x3D4);
	outb((uint8_t) (pos & 0xFF), 0x3D5);
	outb(0x0E, 0x3D4);
	outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);
}
