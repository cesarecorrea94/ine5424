// ------------------------------------------------------------
// Cortex-A MPCore - Global timer functions
// Header Filer
//
// Copyright ARM Ltd 2009. All rights reserved.
// ------------------------------------------------------------

#ifndef __cortex_a_global_timer_h
#define __cortex_a_global_timer_h

// Typical set of calls to enable Timer:
// init_global_timer( AUTO_INCREMENT>, INCREMENT_VALUE );
// set_global_timer_comparator( UPPER_32_BITS, LOWER_32_BITS );
// start_global_timer();


// Sets up the global timer
// r0:  IF 0 (AutoIncrement) ELSE (SingleShot)
// r1:  Increment value (ignored if auto_increment != 0)
void init_global_timer(unsigned int auto_increment, unsigned int increment_value);

// Sets the comparator value for this CPU 
void set_global_timer_comparator(unsigned int top, unsigned int bottom);

// Starts the global timer
void start_global_timer(void);

// Stops the global timer
void stop_global_timer(void);

// Reads the current value of the timer count register
// Returns bits 63:32 in *top, and bits 31:0 in *bottom
void read_global_timer(unsigned int* top, unsigned int* bottom);

// Clears the global timer interrupt
void clear_global_timer_irq(void);

#endif

// ------------------------------------------------------------
// End of MP_GlobalTimer.h
// ------------------------------------------------------------
