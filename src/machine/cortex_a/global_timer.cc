extern "C" {
// ------------------------------------------------------------
// Cortex-A MPCore - Global timer functions
//
// Copyright ARM Ltd 2009. All rights reserved.
// ------------------------------------------------------------

    /* PRESERVE8 */

    /* AREA  MP_PrivateTimer, CODE, READONLY */

    // PPI ID 27


    // Typical set of calls to enable Timer:
    // init_global_timer()
    // set_global_timer_comparator()
    // start_global_timer()

// ------------------------------------------------------------

    /* EXPORT init_global_timer */
    __attribute__((naked)) void init_global_timer(unsigned int auto_increment, unsigned int increment_value){
    // Initializes the Global Timer, but does NOT set the enable bit
    // r0: IF 0 (AutoIncrement) ELSE (SingleShot)
    // r1: increment value
/* init_global_timer PROC */

    // Get base address of private perpherial space
    __asm("MRC     p15, 4, r2, c15, c0, 0");  // Read periph base address

    // Control register bit layout
    // Bit 0 - Timer enable
    // Bit 1 - Comp enable
    // Bit 2 - IRQ enable
    // Bit 3 - Auto-increment enable

    // Ensure the timer is disabled
    __asm("LDR     r3, [r2, #0x208]");        // Read control reg
    __asm("BIC     r3, r3, #0x01");           // Clear enable bit
    __asm("STR     r3, [r2, #0x208]");        // Write control reg

    // Form control reg value
    __asm("CMP     r0, #0");                  // Check whether to enable auto-reload
    __asm("MOVNE   r0, #0x00");               // No auto-reload
    __asm("MOVEQ   r0, #0x04");               // With auto-reload
    __asm("STR     r0, [r2, #0x208]");        // Store to control register

    // Store increment value
    __asm("STREQ   r1, [r2, #0x218]");
    
    // Clear timer value
    __asm("MOV     r0, #0x0");
    __asm("STR     r0, [r2, #0x0]");
    __asm("STR     r0, [r2, #0x4]");

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT set_global_timer_comparator */
    __attribute__((naked)) void set_global_timer_comparator(unsigned int top, unsigned int bottom){
    // Writes the comparator registers, and enable the comparator bit in the control register
    // r0: 63:32 of the comparator value
    // r1: 31:0  of the comparator value
/* set_global_timer_comparator PROC */

    __asm("MRC     p15, 4, r2, c15, c0, 0");  // Read periph base address

    // Disable comparator before updating register
    __asm("LDR     r1, [r2, #0x208]");        // Read control reg
    __asm("BIC     r3, r3, #0x02");           // Clear comparator enable bit
    __asm("STR     r3, [r2, #0x208]");        // Write modified value back
    
    // Write the comparator registers
    __asm("STR     r1, [r2, #0x210]");        // Write lower 32 bits
    __asm("STR     r0, [r2, #0x214]");        // Write upper 32 bits
    __asm("DMB");
    
    // Re-enable the comparator
    __asm("ORR     r3, r3, #0x02");           // Set comparator enable bit
    __asm("STR     r3, [r2, #0x208]");        // Write modified value back

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT start_global_timer */
    __attribute__((naked)) void start_global_timer(void){
    // Starts the global timer
/* start_global_timer PROC */

    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    __asm("LDR     r1, [r0, #0x208]");        // Read control reg
    __asm("ORR     r1, r1, #0x01");           // Set enable bit
    __asm("STR     r1, [r0, #0x208]");        // Write modified value back

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT stop_global_timer */
    __attribute__((naked)) void stop_global_timer(void){
    // Stops the private timer
/* stop_global_timer PROC */

    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    __asm("LDR     r1, [r0, #0x208]");        // Read control reg
    __asm("BIC     r1, r1, #0x01");           // Clear enable bit
    __asm("STR     r1, [r0, #0x208]");        // Write modified value back

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT get_global_timer_count */
    __attribute__((naked)) void read_global_timer(unsigned int* top, unsigned int* bottom){
    // Reads the current value of the timer count register
    // r0: Address of unsigned int for bits 63:32
    // r1: Address of unsigned int for bits 31:0
/* get_global_timer_count PROC */
    __asm("get_global_timer_count_loop:");

    __asm("MRC     p15, 4, r2, c15, c0, 0");  // Read periph base address

    __asm("LDR     r12,[r2, #0x04]");         // Read bits 63:32
    __asm("LDR     r3, [r2, #0x00]");         // Read bits 31:0
    __asm("LDR     r2, [r2, #0x04]");         // Re-read bits 63:32

    __asm("CMP     r2, r12");                 // Have the top bits changed?
    __asm("BNE     get_global_timer_count_loop");

    // Store result out to pointers
    __asm("STR     r2, [r0]");
    __asm("STR     r3, [r1]");

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT clear_global_timer_irq */
    __attribute__((naked)) void clear_global_timer_irq(void){
    // Clears the global timer interrupt
/* clear_global_timer_irq PROC */
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    // Clear the interrupt by writing 0x1 to the Timer's Interrupt Status register
    __asm("MOV     r1, #1");
    __asm("STR     r1, [r0, #0x20C]");

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------
// End of code
// ------------------------------------------------------------

    /* END */

// ------------------------------------------------------------
// End of MP_GlobalTimer.s
// ------------------------------------------------------------
}
