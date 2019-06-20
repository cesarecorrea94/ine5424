// EPOS Cortex-A SETUP

#include <system/config.h>
#include <machine/cortex_a/scu.h>
#include <machine/cortex_a/gic.h>

extern "C" { void _vector_table() __attribute__ ((used, naked, section(".init"))); }

// Interrupt Vector Table
void _vector_table()
{
    ASM("\t\n\
        b   _reset                                                           \t\n\
        b   _undefined_instruction                                           \t\n\
        b   _software_interrupt                                              \t\n\
        b   _prefetch_abort                                                  \t\n\
        b   _data_abort                                                      \t\n\
        nop                           // Reserved                            \t\n\
        b   _int_entry                // _irq                                \t\n\
        b   _fiq                                                             \t\n\
        ");
    __asm("_reset:");

    // Set a temporary Stack Pointer for INIT
    // Mains stack will be allocated by Thread::init()
    // "\n     ldr     r0, =__boot_stack__"
    // "\n     mov     sp, r0"
    __asm("bl      get_cpu_id");
    __asm("mul     r0, %0" : : "p"(EPOS::S::Traits<EPOS::S::Machine>::STACK_SIZE));
    __asm("ldr     sp, =__boot_stack__");
    __asm("sub     sp, r0");

    __asm("bl      get_cpu_id");
    __asm("cmp     r0, #0");            // Verifica se CPU0
    __asm("blne    read_irq_ack");      // Pega SGI ID (=0), se CPU != 0
    __asm("blne    write_end_of_irq");  // EOI (ID=0), se CPU != 0

    //
    // MMU Init
    // ---------

    //
    // Invalidate caches
    // ------------------
    __asm("BL      invalidate_caches");

    //
    // Clear Branch Prediction Array
    // ------------------------------
    __asm("MOV     r0, #0x0");
    __asm("MCR     p15, 0, r0, c7, c5, 6");     // BPIALL - Invalidate entire branch predictor array

    //
    // Invalidate TLBs
    //------------------
    __asm("MOV     r0, #0x0");
    __asm("MCR     p15, 0, r0, c8, c7, 0");     // TLBIALL - Invalidate entire Unifed TLB

    //
    // Set up Domain Access Control Reg
    // ----------------------------------
    // b00 - No Access (abort)
    // b01 - Client (respect table entry)
    // b10 - RESERVED
    // b11 - Manager (ignore access permissions)
    // Setting D0 to client, all others to No Access
    __asm("MOV     r0, #0x01");
    __asm("MCR     p15, 0, r0, c3, c0, 0");


    // Page tables
    // -------------------------
    // Each CPU will have its own L1 page table.  The
    // code reads the base address from the scatter file
    // the uses the CPUID to calculate an offset for each
    // CPU.
    //
    // The page tables are generated at boot time.  First
    // the table is zeroed.  Then the individual valid 
    // entries are written in
    //

    // Calculate offset for this CPU
    // __asm("LDR     r0, =||Image$$PAGETABLES$$ZI$$Base||");
    __asm("LDR     r0, %0" : : "p"(EPOS::S::Traits<EPOS::S::Machine>::STACK_SIZE));
    __asm("MRC     p15, 0, r1, c0, c0, 5");     // Read Multiprocessor Affinity Register
    __asm("ANDS    r1, r1, #0x03");             // Mask off, leaving the CPU ID field
    __asm("MOV     r1, r1, LSL #14");           // Convert core ID into a 16K offset (this is the size of the table)
    __asm("ADD     r0, r1, r0");                // Add offset to current table location to get dst

    // Fill table with zeros
    __asm("MOV     r2, #1024");                 // Set r3 to loop count (4 entries per iteration, 1024 iterations)
    __asm("MOV     r1, r0");                    // Make a copy of the base dst
    __asm("MOV     r3, #0");
    __asm("MOV     r4, #0");
    __asm("MOV     r5, #0");
    __asm("MOV     r6, #0");
    __asm("ttb_zero_loop:");
    __asm("STMIA   r1!, {r3-r6}");              // Store out four entries
    __asm("SUBS    r2, r2, #1");                // Decrement counter
    __asm("BNE     ttb_zero_loop");

    //
    // STANDARD ENTRIES
    //

    // Entry for VA 0x0
    // This region must be coherent
    __asm("LDR     r1, =PABASE_VA0");           // Physical address
    __asm("LDR     r2, =TTB_COHERENT");         // Descriptor template
    __asm("ORR     r1, r1, r2");                // Combine address and template
    __asm("STR     r1, [r0]");

    // Entry for VA 0x0010,0000
    // Each CPU stores private data in this address range
    // Using the MMU to map to different PA on each CPU.
    // 
    // CPU 0 - PA Base
    // CPI 1 - PA Base + 1MB
    // CPU 2 - PA Base + 2MB
    // CPU 3 - PA Base + 3MB

    __asm("MRC     p15, 0, r1, c0, c0, 5");     // Re-read Multiprocessor Affinity Register
    __asm("AND     r1, r1, #0x03");             // Mask off, leaving the CPU ID field
    __asm("MOV     r1, r1, LSL #20");           // Convert core ID into a MB offset
    
    __asm("LDR     r3, =PABASE_VA1");           // Base PA
    __asm("ADD     r1, r1, r3");                // Add CPU offset to PA
    __asm("LDR     r2, =TTB_NONCOHERENT");      // Descriptor template
    __asm("ORR     r1, r1, r2");                // Combine address and template
    __asm("STR     r1, [r0, #4]");

    // Entry for private address space
    // Needs to be marked as Device memory
    __asm("MRC     p15, 4, r1, c15, c0, 0");    // Get base address of private address space
    __asm("LSR     r1, r1, #20");               // Clear bottom 20 bits, to find which 1MB block its in
    __asm("LSL     r2, r1, #2");                // Make a copy, and multiply by four.  This gives offset into the page tables
    __asm("LSL     r1, r1, #20");               // Put back in address format

    __asm("LDR     r3, =TTB_DEVICE");           // Descriptor template
    __asm("ORR     r1, r1, r3");                // Combine address and template
    __asm("STR     r1, [r0, r2]");
    
    //
    // OPTIONAL ENTRIES
    // You will need additional translations if:
    // - No RAM at zero, so cannot use flat mapping
    // - You wish to retarget
    //

    // If not flat mapping, you need a page table entry covering
    // the physical address of the boot code.
    //LDR     r1, =PABASE_VA0           // Physical address
    //LSR     r2, r1, #18               // Make a copy of PA, and convert in table offset
    //LDR     r3, =TTB_COHERENT         // Descriptor template
    //ORR     r1, r1, r3                // Combine address and template
    //STR     r1, [r0, r2]

    // If you wish to output to stdio to a UART you will need
    // an additional entry
    //LDR     r1, =PABASE_UART          // Physical address of UART
    //LSR     r1, r1, #20               // Mask off bottom 20 bits to find which 1MB it is within
    //LSL     r2, r1, #2                // Make a copy and multiply by 4 to get table offset
    //LSL     r1, r1, #20               // Put back into address format
    //LDR     r3, =TTB_DEVICE           // Descriptor template
    //ORR     r1, r1, r3                // Combine address and template
    //STR     r1, [r0, r2]

    //
    // Barrier
    // --------
    __asm("DSB");

    //
    // Set location of level 1 page table
    //------------------------------------
    // 31:14 - Base addr 0x8400,0000
    // 13:5  - 0x0
    // 4:3   - RGN 0x0 (Outer Noncachable)
    // 2     - P   0x0
    // 1     - S   0x0 (Non-shared)
    // 0     - C   0x0 (Inner Noncachable)
    __asm("MCR     p15, 0, r0, c2, c0 ,0");


    // Enable MMU
    //-------------
    // 0     - M, set to enable MMU
    // Leaving the caches disabled until after scatter loading.
    __asm("MRC     p15, 0, r0, c1, c0, 0");     // Read current control reg
    __asm("ORR     r0, r0, #0x01");             // Set M bit
    __asm("MCR     p15, 0, r0, c1, c0, 0");     // Write reg back

    //
    // MMU now enable - Virtual address system now active
    //

    //
    // Branch Prediction Init
    // -----------------------
    __asm("BL      enable_branch_prediction");

    //
    // SMP initialization 
    // -------------------
    __asm("MRC     p15, 0, r0, c0, c0, 5");     // Read CPU ID register
    __asm("ANDS    r0, r0, #0x03");             // Mask off, leaving the CPU ID field
    __asm("BLEQ    primary_cpu_init");
    __asm("BLNE    secondary_cpus_init");

    // ------------------------------------------------------------
    // Initialization for PRIMARY CPU
    // ------------------------------------------------------------
    __asm("primary_cpu_init:");
    //
    // Enable the SCU
    // ---------------
    __asm("BL      enable_scu");
    
    //
    // Join SMP
    // ---------
    __asm("MOV     r0, #0x0");                  // Move CPU ID into r0
    __asm("MOV     r1, #0xF");                  // Move 0xF (represents all four ways) into r1
    __asm("BL      secure_SCU_invalidate");
    __asm("BL      join_smp");
    __asm("BL      enable_maintenance_broadcast");
    
    ASM(
    // Clear the BSS
    "\n     eor     r0, r0"
    "\n     ldr     r1, =__bss_start__"
    "\n     ldr     r2, =__bss_end__"
    "\n .L1:"
    "\n     str     r0, [r1]"
    "\n     add     r1, #4"
    "\n     cmp     r1, r2"
    "\n     blt     .L1"
    "\n");

    //
    // GIC Init
    // ---------
    __asm("BL      enable_GIC");
    __asm("BL      enable_gic_processor_interface");

    //
    // Branch to C lib code
    // ----------------------
    // __asm("B       __main");
    __asm("b _start");

    // ------------------------------------------------------------
    // Initialization for SECONDARY CPUs
    // ------------------------------------------------------------
    __asm("secondary_cpus_init:");
    //
    // GIC Init
    // ---------
    __asm("BL      enable_gic_processor_interface");

    __asm("MOV     r0, #0x1F");                 // Priority
    __asm("BL      set_priority_mask");

    __asm("MOV     r0, #0x0");                  // ID
    __asm("BL      enable_irq_id");

    __asm("MOV     r0, #0x0");                  // ID
    __asm("MOV     r1, #0x0");                  // Priority
    __asm("BL      set_irq_priority");

    //
    // Join SMP
    // ---------
    __asm("MRC     p15, 0, r0, c0, c0, 5");     // Read CPU ID register
    __asm("ANDS    r0, r0, #0x03");             // Mask off, leaving the CPU ID field
    __asm("MOV     r1, #0xF");                  // Move 0xF (represents all four ways) into r1
    __asm("BL      secure_SCU_invalidate");
    
    __asm("BL      join_smp");
    __asm("BL      enable_maintenance_broadcast");

    // //
    // // Holding Pen
    // // ------------
    // __asm("MOV     r2, #0x00");                 // Clear r2
    // __asm("CPSIE   i");                         // Enable interrupts

    // __asm("holding_pen:");
    // __asm("CMP     r2, #0x0");                  // r2 will be set to 0x1 by IRQ handler on receiving SGI
    // __asm("WFIEQ");
    // __asm("BEQ     holding_pen");
    // __asm("CPSID   i");                         // IRQs not used in reset of example, so mask out interrupts

    //
    // Branch to C lib code
    // ----------------------
    // __asm("B       __main");
    __asm("b _start");
}
