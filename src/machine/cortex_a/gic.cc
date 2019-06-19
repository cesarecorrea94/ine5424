extern "C" {
// ------------------------------------------------------------
// Cortex-A MPCore - Interrupt Controller functions
//
// Copyright ARM Ltd 2009. All rights reserved.
// ------------------------------------------------------------

    /* PRESERVE8 */

    /* AREA  MP_GIC, CODE, READONLY */

// ------------------------------------------------------------
// GIC
// ------------------------------------------------------------

    // CPU Interface offset from base of private peripheral space --> 0x0100
    // Interrupt Distributor offset from base of private peripheral space --> 0x1000

    // Typical calls to enable interrupt ID X:
    // enable_irq_id(X)                  <-- Enable that ID
    // set_irq_priority(X, 0)            <-- Set the priority of X to 0 (the max priority)
    // set_priority_mask(0x1F)           <-- Set CPU's priority mask to 0x1F (the lowest priority)
    // enable_GIC()                      <-- Enable the GIC (global)
    // enable_gic_processor_interface()  <-- Enable the CPU interface (local to the CPU)


    /* EXPORT  enable_GIC */
__attribute__((naked)) void enable_GIC(void) {
    // Global enable of the Interrupt Distributor
/* enable_GIC PROC */

    // Get base address of private perpherial space
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address
    __asm("ADD     r0, r0, #0x1000");         // Add the GIC offset

    __asm("LDR     r1, [r0]");                // Read the GIC's Enable Register  (ICDDCR)
    __asm("ORR     r1, r1, #0x01");           // Set bit 0, the enable bit
    __asm("STR     r1, [r0]");                // Write the GIC's Enable Register  (ICDDCR)

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT disable_GIC */
    __attribute__((naked)) void disable_GIC(void) {
    // Global disable of the Interrupt Distributor
/* disable_GIC PROC */

    // Get base address of private perpherial space
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address
    __asm("ADD     r0, r0, #0x1000");         // Add the GIC offset

    __asm("LDR     r1, [r0]");                // Read the GIC's Enable Register  (ICDDCR)
    __asm("BIC     r1, r1, #0x01");           // Set bit 0, the enable bit
    __asm("STR     r1, [r0]");                // Write the GIC's Enable Register  (ICDDCR)

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT  enable_irq_id */
    __attribute__((naked)) void enable_irq_id(unsigned int ID){
    // Enables the interrupt source number ID
/* enable_irq_id PROC */

    // Get base address of private perpherial space
    __asm("MOV     r1, r0");                  // Back up passed in ID value
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    // Each interrupt source has an enable bit in the GIC.  These
    // are grouped into registers, with 32 sources per register
    // First, we need to identify which 32 bit block the interrupt lives in
    __asm("MOV     r2, r1");                  // Make working copy of ID in r2
    __asm("MOV     r2, r2, LSR #5");          // LSR by 5 places, affective divide by 32
                                              // r2 now contains the 32 bit block this ID lives in
    __asm("MOV     r2, r2, LSL #2");          // Now multiply by 4, to covert offset into an address offset (four bytes per reg)

    // Now work out which bit within the 32 bit block the ID is
    __asm("AND     r1, r1, #0x1F");           // Mask off to give offset within 32bit block
    __asm("MOV     r3, #1");                  // Move enable value into r3
    __asm("MOV     r3, r3, LSL r1");          // Shift it left to position of ID

    __asm("ADD     r2, r2, #0x1100");         // Add the base offset of the Enable Set registers to the offset for the ID
    __asm("STR     r3, [r0, r2]");            // Store out  (ICDISER)

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------
    
    /* EXPORT  disable_irq_id */
    __attribute__((naked)) void disable_irq_id(unsigned int ID){
    // Disables the interrupt source number ID
/* disable_irq_id PROC */

    // Get base address of private perpherial space
    __asm("MOV     r1, r0");                  // Back up passed in ID value
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    // First, we need to identify which 32 bit block the interrupt lives in
    __asm("MOV     r2, r1");                  // Make working copy of ID in r2
    __asm("MOV     r2, r2, LSR #5");          // LSR by 5 places, affective divide by 32
                                              // r2 now contains the 32 bit block this ID lives in
    __asm("MOV     r2, r2, LSL #2");          // Now multiply by 4, to covert offset into an address offset (four bytes per reg)

    // Now work out which bit within the 32 bit block the ID is
    __asm("AND     r1, r1, #0x1F");           // Mask off to give offset within 32bit block
    __asm("MOV     r3, #1");                  // Move enable value into r3
    __asm("MOV     r3, r3, LSL r1");          // Shift it left to position of ID in 32 bit block

    __asm("ADD     r2, r2, #0x1180");         // Add the base offset of the Enable Clear registers to the offset for the ID
    __asm("STR     r3, [r0, r2]");            // Store out (ICDICER)

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT set_irq_priority */
    __attribute__((naked)) void set_irq_priority(unsigned int ID, unsigned int priority){
    // Sets the priority of the specifed ID
    // r0 = ID
    // r1 = priority
/* set_irq_priority PROC */

    // Get base address of private perpherial space
    __asm("MOV     r2, r0");                  // Back up passed in ID value
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    // r0 = base addr
    // r1 = priority
    // r2 = ID
    
    // Make sure that priority value is only 5 bits, and convert to expected format
    __asm("AND     r1, r1, #0x1F");
    __asm("MOV     r1, r1, LSL #3");

    // Find which priority register this ID lives in
    __asm("BIC     r3, r2, #0x03");           // Make a copy of the ID, clearing off the bottom two bits
                                    // There are four IDs per reg, by clearing the bottom two bits we get an address offset
    __asm("ADD     r3, r3, #0x1400");         // Now add the offset of the Priority Level registers from the base of the private peripheral space
    __asm("ADD     r0, r0, r3");              // Now add in the base address of the private peripheral space, giving us the absolute address


    // Now work out which ID in the register it is
    __asm("AND     r2, r2, #0x03");           // Clear all but the bottom four bits, leaves which ID in the reg it is (which byte)
    __asm("MOV     r2, r2, LSL #3");          // Multiply by 8, this gives a bit offset

    // Read -> Modify -> Write
    __asm("MOV     r12, #0xFF");              // Mask (8 bits)
    __asm("MOV     r12, r12, LSL r2");        // Move mask into correct bit position
    __asm("MOV     r1, r1, LSL r2");          // Also, move passed in priority value into correct bit position

    __asm("LDR     r3, [r0]");                // Read current value of the Priority Level register (ICDIPR)
    __asm("BIC     r3, r3, r12");             // Clear appropiate field
    __asm("ORR     r3, r3, r1");              // Now OR in the priority value
    __asm("STR     r3, [r0]");                // And store it back again  (ICDIPR)

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT enable_gic_processor_interface */
    __attribute__((naked)) void enable_gic_processor_interface(void){
    // Enables the processor interface
    // Must been done one each CPU seperately
/* enable_gic_processor_interface PROC */

    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    __asm("LDR     r1, [r0, #0x100]");        // Read the Processor Interface Control register   (ICCICR/ICPICR)
    __asm("ORR     r1, r1, #0x03");           // Bit 0: Enables secure interrupts, Bit 1: Enables Non-Secure interrupts
    __asm("STR     r1, [r0, #0x100]");        // Write the Processor Interface Control register  (ICCICR/ICPICR)

    __asm("BX      lr");
    /* ENDP */
}

// ------------------------------------------------------------

    /* EXPORT disable_gic_processor_interface */
    __attribute__((naked)) void disable_gic_processor_interface(void){
    // Disables the processor interface
    // Must been done one each CPU seperately
/* disable_gic_processor_interface PROC */

    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    __asm("LDR     r1, [r0, #0x100]");        // Read the Processor Interface Control register   (ICCICR/ICPICR)
    __asm("BIC     r1, r1, #0x03");           // Bit 0: Enables secure interrupts, Bit 1: Enables Non-Secure interrupts
    __asm("STR     r1, [r0, #0x100]");        // Write the Processor Interface Control register  (ICCICR/ICPICR)

    __asm("BX      lr");
    /* ENDP */
}

// ------------------------------------------------------------

    /* EXPORT set_priority_mask */
    __attribute__((naked)) void set_priority_mask(unsigned int priority){
    // Sets the Priority mask register for the CPU run on
    // The reset value masks ALL interrupts!
/* set_priority_mask PROC */

    // Get base address of private perpherial space
    __asm("MOV     r1, r0");                  // Back up passed in ID value
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address
    
    __asm("STR     r1, [r0, #0x0104]");       // Write the Priority Mask register (ICCPMR/ICCIPMR)

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT  set_binary_port */
    __attribute__((naked)) void set_binary_port(unsigned int priority){
    // Sets the Binary Point Register for the CPU run on
/* set_binary_port PROC */

    // Get base address of private perpherial space
    __asm("MOV     r1, r0");                  // Back up passed in ID value
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    __asm("STR     r1, [r0, #0x0108]");       // Write the Binary register   (ICCBPR/ICCBPR)

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT  read_irq_ack */
    __attribute__((naked)) unsigned int read_irq_ack(void){
    // Returns the value of the Interrupt Acknowledge Register
/* read_irq_ack PROC */
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address
    __asm("LDR     r0, [r0, #0x010C]");       // Read the Interrupt Acknowledge Register  (ICCIAR)
    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------

    /* EXPORT  write_end_of_irq */
    __attribute__((naked)) void write_end_of_irq(unsigned int ID){
    // Writes ID to the End Of Interrupt register
/* write_end_of_irq PROC */

    // Get base address of private perpherial space
    __asm("MOV     r1, r0");                  // Back up passed in ID value
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    __asm("STR     r1, [r0, #0x0110]");       // Write ID to the End of Interrupt register (ICCEOIR)

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------
// SGI
// ------------------------------------------------------------

    /* EXPORT send_sgi */
    __attribute__((naked)) void send_sgi(unsigned int ID, unsigned int target_list, unsigned int filter_list){
    // Send a software generate interrupt
/* send_sgi PROC */

    __asm("AND     r3, r0, #0x0F");           // Mask off unused bits of ID, and move to r3
    __asm("AND     r1, r1, #0x0F");           // Mask off unused bits of target_filter
    __asm("AND     r2, r2, #0x0F");           // Mask off unused bits of filter_list

    __asm("ORR     r3, r3, r1, LSL #16");     // Combine ID and target_filter
    __asm("ORR     r3, r3, r2, LSL #24");     // and now the filter list

    // Get the address of the GIC
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address
    __asm("ADD     r0, r0, #0x1F00");         // Add offset of the sgi_trigger reg

    __asm("STR     r3, [r0]");                // Write to the Software Generated Interrupt Register  (ICDSGIR)

    __asm("BX      lr");
    /* ENDP */
}
// ------------------------------------------------------------
// End of code
// ------------------------------------------------------------

    /* END */

// ------------------------------------------------------------
// End of MP_GIC.s
// ------------------------------------------------------------
}
