extern "C" {
// ------------------------------------------------------------
// Cortex-A MPCore - SCU functions
//
// Copyright ARM Ltd 2009. All rights reserved.
// ------------------------------------------------------------

    /* PRESERVE8 */

    /* AREA  MP_SCU, CODE, READONLY */

// ------------------------------------------------------------
// Misc
// ------------------------------------------------------------

/* EXPORT get_base_addr */
__attribute__((naked)) unsigned int get_base_addr_asm(void){
    // Returns the base address of the private peripheral memory space
    /* get_base_addr PROC */
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address      (see DE593076)
    __asm("BX      lr");
    /* ENDP */
}

unsigned int *get_base_addr(void){
    unsigned int *periphbase;    // address periphbase

    __asm("MRC p15, 4, %0, c15, c0, 0" : "=r"(periphbase) : );  // Read periph base address

    return periphbase;
}
// ------------------------------------------------------------

unsigned int *get_actrl_addr(void){
    unsigned int *actlr_addr;

    __asm("MRC p15, 0, %0, c1, c0, 1": "=r"(actlr_addr) :); // Read Auxiliary Control Register (ACTLR)

    return actlr_addr;
}
// ------------------------------------------------------------

/* EXPORT get_cpu_id */
__attribute__((naked)) unsigned int get_cpu_id_asm(void){
    // Returns the CPU ID (0 to 3) of the CPU executed on
    /* get_cpu_id PROC */
    __asm("MRC     p15, 0, r0, c0, c0, 5");   // Read CPU ID register
    __asm("AND     r0, r0, #0x03");           // Mask off, leaving the CPU ID field
    __asm("BX      lr");
    /* ENDP */
}

unsigned int get_cpu_id(void){
    unsigned int *multiproc_aff_addr;    // address Multiprocessor Affinity Register

    __asm("MRC p15, 0, 0%, c0, c0, 5" : "=r"(multiproc_aff_addr) : );   // Read Multiprocessor Affinity Register

    return *multiproc_aff_addr & 0x03;

}
// ------------------------------------------------------------

/* EXPORT get_num_cpus */
__attribute__((naked)) unsigned int get_num_cpus_asm(void){
    // Returns the number of CPUs in the A9 Cluster
    /* get_num_cpus PROC */

    // Get base address of private perpherial space
    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address
    
    __asm("LDR     r0, [r0, #0x004]");        // Read SCU Configuration register
    __asm("AND     r0, r0, #0x3");            // Bits 1:0 gives the number of cores
    __asm("ADD     r0, #1");

    __asm("BX      lr");
    /* ENDP */
}

unsigned int get_num_cpus(void){
    unsigned int *periphbase = get_base_addr();         // address periphbase
    unsigned int *scu_config_addr;                      // address periphbase + 0x4
    unsigned int scu_config_value;                      // value of scu_config_addr
    unsigned int n_cores;                               // number of cores

    *scu_config_addr = *periphbase | (unsigned int)0x4; // SCU Configuration Register
    scu_config_value = *(scu_config_addr);              // Read SCU Configuration register
    n_cores = scu_config_value & 0x3;                   // Bits 1:0 gives the number of cores - 1

    return n_cores + 1;
}
// ------------------------------------------------------------

// /* EXPORT go_to_sleep */
// __attribute__((naked)) void go_to_sleep(void){
//     /* go_to_sleep PROC */
//     __asm("WFI");                             // Go into standby
//     __asm("B       go_to_sleep");             // Catch in case of rogue events
//     __asm("BX      lr");
//     /* ENDP */
// }

// ------------------------------------------------------------
// SCU
// ------------------------------------------------------------

    // SCU offset from base of private peripheral space --> 0x000

/* EXPORT  enable_scu */
__attribute__((naked)) void enable_scu_asm(void){
    // Enables the SCU
    /* enable_scu PROC */

    __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

    __asm("LDR     r1, [r0, #0x0]");          // Read the SCU Control Register
    __asm("ORR     r1, r1, #0x1");            // Set bit 0 (The Enable bit)
    __asm("STR     r1, [r0, #0x0]");          // Write back modifed value

    __asm("BX      lr");
    /* ENDP */
}

void enable_scu(void){
    unsigned int *scu_ctrl_addr = get_base_addr();    // periphbase == scu control
    unsigned int scu_ctrl_val = *scu_ctrl_addr;         // Read the SCU Control Register

    *scu_ctrl_addr = scu_ctrl_val | 0x1;                // Set bit 0 (The Enable bit) and write back
}
// ------------------------------------------------------------

/* EXPORT  join_smp */
__attribute__((naked)) void join_smp_asm(void){
    // Set this CPU as participating in SMP
    /* join_smp  PROC */

    // SMP status is controlled by bit 6 of the CP15 Aux Ctrl Reg

    __asm("MRC     p15, 0, r0, c1, c0, 1");   // Read ACTLR
    __asm("ORR     r0, r0, #0x040");          // Set bit 6
    __asm("MCR     p15, 0, r0, c1, c0, 1");   // Write ACTLR

    __asm("BX      lr");
    /* ENDP */
}

void join_smp(void){
    unsigned int *actlr_addr = get_actrl_addr();
    unsigned int actlr_val;

    actlr_val = *actlr_addr;                                  
    *actlr_addr = actlr_val | 0x040;                        // Set bit 6 and write
}


// ------------------------------------------------------------

//     /* EXPORT leave_smp */
//     __attribute__((naked)) void leave_smp(void){
//     // Set this CPU as NOT participating in SMP
// /* leave_smp PROC */

//     // SMP status is controlled by bit 6 of the CP15 Aux Ctrl Reg

//     __asm("MRC     p15, 0, r0, c1, c0, 1");   // Read ACTLR
//     __asm("BIC     r0, r0, #0x040");          // Clear bit 6
//     __asm("MCR     p15, 0, r0, c1, c0, 1");   // Write ACTLR

//     __asm("BX      lr");
//     /* ENDP */
// }
// ------------------------------------------------------------

//     /* EXPORT get_cpus_in_smp */
//     __attribute__((naked)) unsigned int get_cpus_in_smp(void){
//     // The return value is 1 bit per core:
//     // bit 0 - CPU 0
//     // bit 1 - CPU 1
//     // etc...
// /* get_cpus_in_smp PROC */

//     __asm("MRC     p15, 4, r0, c15, c0, 0");  // Read periph base address

//     __asm("LDR     r0, [r0, #0x004]");        // Read SCU Configuration register
//     __asm("MOV     r0, r0, LSR #4");          // Bits 7:4 gives the cores in SMP mode, shift then mask
//     __asm("AND     r0, r0, #0x0F");

//     __asm("BX      lr");
//     /* ENDP */
// }
// ------------------------------------------------------------

/* EXPORT enable_maintenance_broadcast */
__attribute__((naked)) void enable_maintenance_broadcast_asm(void){
    // Enable the broadcasting of cache & TLB maintenance operations
    // When enabled AND in SMP, broadcast all "inner sharable"
    // cache and TLM maintenance operations to other SMP cores
    /* enable_maintenance_broadcast PROC */
    __asm("MRC     p15, 0, r0, c1, c0, 1");   // Read Aux Ctrl register
    __asm("ORR     r0, r0, #0x01");           // Set the FW bit (bit 0)
    __asm("MCR     p15, 0, r0, c1, c0, 1");   // Write Aux Ctrl register

    __asm("BX      lr");
    /* ENDP */
}

void enable_maintenance_broadcast(void) {
    unsigned int *actlr_addr = get_actrl_addr();
    unsigned int actlr_val = *actlr_addr;

    *actlr_addr = actlr_val | 0x01;
}
// ------------------------------------------------------------

//     /* EXPORT disable_maintenance_broadcast */
//     __attribute__((naked)) void disable_maintenance_broadcast(void){
//     // Disable the broadcasting of cache & TLB maintenance operations
// /* disable_maintenance_broadcast PROC */
//     __asm("MRC     p15, 0, r0, c1, c0, 1");   // Read Aux Ctrl register
//     __asm("BIC     r0, r0, #0x01");           // Clear the FW bit (bit 0)
//     __asm("MCR     p15, 0, r0, c1, c0, 1");   // Write Aux Ctrl register

//     __asm("BX      lr");
//     /* ENDP */
// }
// ------------------------------------------------------------

/* EXPORT secure_SCU_invalidate */
__attribute__((naked)) void secure_SCU_invalidate_asm(unsigned int cpu, unsigned int ways){
    // cpu: 0x0=CPU 0 0x1=CPU 1 etc...
    // This function invalidates the SCU copy of the tag rams
    // for the specified core.  Typically only done at start-up.
    // Possible flow:
    // - Invalidate L1 caches
    // - Invalidate SCU copy of TAG RAMs
    // - Join SMP
    /* secure_SCU_invalidate PROC */
    __asm("AND     r0, r0, #0x03");           // Mask off unused bits of CPU ID
    __asm("MOV     r0, r0, LSL #2");          // Convert into bit offset (four bits per core)
    
    __asm("AND     r1, r1, #0x0F");           // Mask off unused bits of ways
    __asm("MOV     r1, r1, LSL r0");          // Shift ways into the correct CPU field

    __asm("MRC     p15, 4, r2, c15, c0, 0");  // Read periph base address

    __asm("STR     r1, [r2, #0x0C]");         // Write to SCU Invalidate All in Secure State
    
    __asm("BX      lr");

    /* ENDP */
}

void secure_SCU_invalidate(unsigned int cpu, unsigned int ways){
    unsigned int *periphbase = get_base_addr(); // Periphbase
    unsigned int *scu_invalid_addr;             // SCU Invalidate All Registers in Secure State

    scu_invalid_addr = *periphbase | (unsigned int)0x0C;
    cpu = (cpu & 0x03) * 4;
    ways = (ways & 0x0F) * cpu;
    *scu_invalid_addr = ways;
}
// ------------------------------------------------------------
// End of code
// ------------------------------------------------------------

    /* END */

// ------------------------------------------------------------
// End of MP_SCU.s
// ------------------------------------------------------------
}
