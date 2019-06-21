// EPOS Realview_PBX (ARM Cortex) MCU Initialization

#include <machine/main.h>
#include <machine/cortex_a/ic.h>
#include <machine/cortex_a/machine.h>
#include <machine/cortex_a/gic.h>

#ifdef __mmod_realview_pbx__

__BEGIN_SYS

void Realview_PBX::pre_init()
{
    db<Init, Machine>(TRC) << "Realview_PBX::pre_init()" << endl;
    //Pré inicializações do realview aqui se necessário!!!
    ASM("mcr p15, 0, %0, c12, c0, 0" : : "p"(Traits<Machine>::VECTOR_TABLE));
    if(Machine::cpu_id() == 0){
        volatile unsigned int * SYS_FLAGSSET = (volatile unsigned int *) 0x10000030;
        *SYS_FLAGSSET = Traits<Machine>::VECTOR_TABLE;
        IC::enable(0);
        send_sgi(0x0, 0x0F, 0x01); // Wake the secondary CPUs by sending SGI (ID 0)
        // eoi após sgi (em cada CPUs)
        //IC::disable(0);
        // Machine::smp_init(get_num_cpus());
    }
     Machine::smp_barrier();
}

void Realview_PBX::init()
{
    db<Init, Machine>(TRC) << "Realview_PBX::init()" << endl;
    //Inicializações do realview aqui se necessário!!!
}

__END_SYS

#endif
