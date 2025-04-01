/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xff; /* IRQs 0-7  */
uint8_t slave_mask = 0xff;  /* IRQs 8-15 */

/* void i8259_init();
 * Inputs: nothing
 * Return Value: none
 * Function: initializes pic, by sending the proper interrupt control words to the master and slave pic
 * enables irq 2 to enable the secondary pic */
void i8259_init() {
    //send ICW1 to initialize primary pic
    outb(ICW1, MASTER_8259_PORT);
    
    outb(ICW1, SLAVE_8259_PORT);
  
    //set icw2 signal
    outb(ICW2_MASTER, MASTER_8259_PORT+1);// MASTER_8259_PORT+1: Primary PIC Interrupt Mask Register and Data Register   
    
    outb(ICW2_SLAVE, SLAVE_8259_PORT+1);// SLAVE_8259_PORT+1: Secondary (Slave) PIC Interrupt Mask Register and Data Register
   
    //set icw3 signal
    outb(ICW3_MASTER, MASTER_8259_PORT+1);// MASTER_8259_PORT+1: Primary PIC Interrupt Mask Register and Data Register 
    
    outb(ICW3_SLAVE, SLAVE_8259_PORT+1);// SLAVE_8259_PORT+1: Secondary (Slave) PIC Interrupt Mask Register and Data Register
    
    //set icw4 signal
    outb(ICW4, MASTER_8259_PORT+1);// MASTER_8259_PORT+1: Primary PIC Interrupt Mask Register and Data Register 
    
    outb(ICW4, SLAVE_8259_PORT+1);// SLAVE_8259_PORT+1: Secondary (Slave) PIC Interrupt Mask Register and Data Register
    
    //set masks back for master and slave
    outb(master_mask, MASTER_8259_PORT+1);// MASTER_8259_PORT+1: Primary PIC Interrupt Mask Register and Data Register 
    outb(slave_mask, SLAVE_8259_PORT+1);// SLAVE_8259_PORT+1: Secondary (Slave) PIC Interrupt Mask Register and Data Register
    enable_irq(2);                      //secondary connect to irq2
}
/* void enable_irq(unsigned char irq_num)
 * Inputs: unsigned char irq_num
 * Return Value: none
 * Function: Enable (unmask) the specified IRQ
 */
void enable_irq(unsigned char irq_num) {
    uint16_t port; // stores which port to write to
    uint8_t value;
    if(irq_num < 8){
        port = MASTER_8259_PORT + 1;// MASTER_8259_PORT+1: Primary PIC Interrupt Mask Register and Data Register 
    } else {
        port = SLAVE_8259_PORT + 1;// SLAVE_8259_PORT+1: Secondary (Slave) PIC Interrupt Mask Register and Data Register
        irq_num -= 8;
    }
    value = inb(port) & ~(1 << irq_num); //set value to the value which unmasks the proper irqs 
    outb(value, port); //write the masks the interrupt mask register and data register
}
/* void enable_irq(unsigned char irq_num)
 * Inputs: unsigned char irq_num
 * Return Value: none
 * Function:  Disable (mask) the specified IRQ
 */
void disable_irq(unsigned char irq_num) {
    uint16_t port;// stores which port to write to
    uint8_t value;

    if(irq_num < 8) {
        port = MASTER_8259_PORT + 1;// MASTER_8259_PORT+1: Primary PIC Interrupt Mask Register and Data Register 
    } else{
        port = SLAVE_8259_PORT + 1;// SLAVE_8259_PORT+1: Secondary (Slave) PIC Interrupt Mask Register and Data Register
        irq_num -= 8;               //since coming from secondary subtract 8 from to irq_num to set it to base
    }

    value = inb(port) | (1 << irq_num); //set value to the value which masks the proper irqs
    outb(value, port); //write the masks the interrupt mask register and data register
}
       
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(unsigned char irq_num) {
    // if out of bounds (>15)of irq numbers return
    if(irq_num > 15){
        return;
    }
    else if(irq_num >= 8){          //irq coming from secondary
        outb((EOI|(irq_num-8)), SLAVE_8259_PORT); //or eoi with irq_num - 8 cuz irq is connected to the slave pic
        outb((EOI|2), MASTER_8259_PORT);//or eoi with 2(ir line in the master pic connected to secondary pic) cuz irq is connected to the slave pic
        return;
    }
    else{
        outb(EOI | irq_num, MASTER_8259_PORT); //or eoi with irq_num cuz irq is connected to the master pic
        return;
    }
}
