// EPOS Realview_PBX (ARM Cortex) MCU Initialization

#include <machine/main.h>
#include <machine/cortex_a/machine.h>
#include <machine/cortex_a/gic.h>

#ifdef __mmod_realview_pbx__

__BEGIN_SYS

void Realview_PBX::pre_init()
{
    db<Init, Machine>(TRC) << "Realview_PBX::pre_init()" << endl;
    //Pré inicializações do realview aqui se necessário!!!
    if(Machine::cpu_id() == 0){
        ASM("mcr p15, 0, %0, c12, c0, 0" : : "p"(Traits<Machine>::VECTOR_TABLE));
        // Machine::smp_init();
    }
}

void Realview_PBX::init()
{
    db<Init, Machine>(TRC) << "Realview_PBX::init()" << endl;
    //Inicializações do realview aqui se necessário!!!
    if(Machine::cpu_id() == 0)
        send_sgi(0x0, 0x0F, 0x01); // Wake the secondary CPUs by sending SGI (ID 0)
    Machine::smp_barrier();
}

__END_SYS

#endif
