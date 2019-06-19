// EPOS Realview_PBX (ARM Cortex-A9) MCU Mediator Declarations

#ifndef __realview_pbx_h
#define __realview_pbx_h

#include <architecture/cpu.h>
#include <architecture/tsc.h>
#include <machine/rtc.h>
#include <system.h>

__BEGIN_SYS

class Realview_PBX
{
    friend class TSC;

protected:
    typedef CPU::Reg32 Reg32;
    typedef CPU::Log_Addr Log_Addr;

public:
    static const unsigned int IRQS = 96;
    static const unsigned int TIMERS = 4;
    static const unsigned int UARTS = 4;
    static const unsigned int GPIO_PORTS = 3;
    static const bool supports_gpio_power_up = false;

    // Base addresses for memory-mapped I/O devices
    enum {
        SCR_BASE        = 0x10000000,//-0x10000fff (prio 0, i/o): arm-sysctl
        I2C_BASE        = 0x10002000,//-0x10002fff (prio 0, i/o): versatile_i2c
        //                  0x10004000-0x10004fff (prio 0, i/o): pl041                // PrimeCell Advanced Audio CODEC
        //                  0x10005000-0x10005fff (prio 0, i/o): pl181                // PrimeCell MultiMedia Card Interface (MMCI)
        //                  0x10006000-0x10006fff (prio 0, i/o): pl050                // PrimeCell PS2 Keyboard/Mouse Interface
        //                  0x10007000-0x10007fff (prio 0, i/o): pl050                // PrimeCell PS2 Keyboard/Mouse Interface
        UART0_BASE      = 0x10009000,//-0x10009fff (prio 0, i/o): pl011                // PrimeCell UART0
        UART1_BASE      = 0x1000a000,//-0x1000afff (prio 0, i/o): pl011                // PrimeCell UART1
        UART2_BASE      = 0x1000b000,//-0x1000bfff (prio 0, i/o): pl011                // PrimeCell UART2
        UART3_BASE      = 0x1000c000,//-0x1000cfff (prio 0, i/o): pl011                // PrimeCell UART3
        //                  0x10011000-0x10011fff (prio 0, i/o): sp804                // ARM Dual-Timer Module
        //                  0x10012000-0x10012fff (prio 0, i/o): sp804                // ARM Dual-Timer Module
        GPIO0_BASE      = 0x10013000,//-0x10013fff (prio 0, i/o): pl061                // PrimeCell General Purpose Input/Output (GPIO)
        GPIO1_BASE      = 0x10014000,//-0x10014fff (prio 0, i/o): pl061                // PrimeCell General Purpose Input/Output (GPIO)
        GPIO2_BASE      = 0x10015000,//-0x10015fff (prio 0, i/o): pl061                // PrimeCell General Purpose Input/Output (GPIO)
        //                  0x10017000-0x10017fff (prio 0, i/o): pl031                // PrimeCell Real Time Clock (TRC)
        //                  0x10020000-0x10020fff (prio 0, i/o): pl110                // PrimeCell Color LCD Controller (Display)
        //                  0x10030000-0x10030fff (prio 0, i/o): pl080                // PrimeCell DMA Controller
        SCU_BASE        = 0x1f000000,//-0x1f0000ff (prio 0, i/o): a9-scu
        GIC_BASE        = 0x1f000100,//-0x1f0001ff (prio 0, i/o): gic_cpu
        GTIMER_BASE     = 0x1f000200,//-0x1f00021f (prio 0, i/o): a9gtimer shared
        PTIMER_BASE     = 0x1f000600,//-0x1f00061f (prio 0, i/o): arm_mptimer_timer
        PWDT_BASE       = 0x1f000620,//-0x1f00063f (prio 0, i/o): arm_mptimer_timer
        GICD_BASE       = 0x1f001000,//-0x1f001fff (prio 0, i/o): gic_dist
        //                  0x1f002000-0x1f002fff (prio 0, i/o): l2x0_cc
        //                  0x4e000000-0x4e0000ff (prio 0, i/o): lan9118-mmio
    };

protected:
    Realview_PBX() {}

    static void reboot() {
        // Reg32 val = scs(AIRCR) & (~((-1u / VECTKEY) * VECTKEY));
        // val |= 0x05fa * VECTKEY | SYSRESREQ;
        // scs(AIRCR) = val;
    }

    static const UUID & uuid() { return System::info()->bm.uuid; } // TODO: System_Info is not populated in this machine

    // Device enabling
    static void enable_uart(unsigned int unit) {
        // assert(unit < UARTS);
        // power_uart(unit, FULL);
        // gpio0(AFSEL) |= 3 << (unit * 2);                // Pins A[1:0] are multiplexed between GPIO and UART 0. Select UART.
        // gpio0(DEN) |= 3 << (unit * 2);                  // Enable digital I/O on Pins A[1:0]
    }

    // Power Management
    static void power_uart(unsigned int unit, const Power_Mode & mode) {
        // assert(unit < UARTS);
        // switch(mode) {
        // case ENROLL:
        // 	break;
        // case DISMISS:
        // 	break;
        // case SAME:
        // 	break;
        // case FULL:
        // 	break;
        // case LIGHT:
        // 	break;
        // case SLEEP:
        //     scr(RCGC1) |= 1 << unit;                   // Activate UART "unit" clock
        //     scr(RCGC2) |= 1 << unit;                   // Activate port "unit" clock
        //     break;
        // case OFF:
        //     scr(RCGC1) &= ~(1 << unit);                // Deactivate UART "unit" clock
        //     scr(RCGC2) &= ~(1 << unit);                // Deactivate port "unit" clock
        //     break;
        // }
    }

public:
    static volatile Reg32 & gdist(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GICD_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & gtimer(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GTIMER_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & ptimer(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(PTIMER_BASE)[o / sizeof(Reg32)]; }
    // static volatile Reg32 & scr(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(SCR_BASE)[o / sizeof(Reg32)]; }
    // static volatile Reg32 & scs(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GIC_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & gic(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GIC_BASE)[o / sizeof(Reg32)]; }
    // static volatile Reg32 & systick(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GTIMER_BASE)[o / sizeof(Reg32)]; }
    // static volatile Reg32 & tsc(unsigned int o)     { return reinterpret_cast<volatile Reg32 *>(TIMER0_BASE)[o / sizeof(Reg32)]; }
    // static volatile Reg32 & timer0(unsigned int o)  { return reinterpret_cast<volatile Reg32 *>(TIMER0_BASE)[o / sizeof(Reg32)]; }
    // static volatile Reg32 & timer1(unsigned int o)  { return reinterpret_cast<volatile Reg32 *>(TIMER2_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & gpio0(unsigned int o)   { return reinterpret_cast<volatile Reg32 *>(GPIO0_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & gpio1(unsigned int o)   { return reinterpret_cast<volatile Reg32 *>(GPIO1_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & gpio2(unsigned int o)   { return reinterpret_cast<volatile Reg32 *>(GPIO2_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & gpio(unsigned int port, unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GPIO0_BASE + 0x1000*(port))[o / sizeof(Reg32)]; }

protected:
    static void pre_init();
    static void init();
};

typedef Realview_PBX Machine_Model;

__END_SYS

#endif
