// EPOS Realview_PBX (ARM Cortex) MCU Initialization

#include <machine/main.h>

#ifdef __mmod_realview_pbx__

__BEGIN_SYS

void Realview_PBX::pre_init()
{
    db<Init, Machine>(TRC) << "Realview_PBX::pre_init()" << endl;
    //Pré inicializações do realview aqui se necessário!!!
}

void Realview_PBX::init()
{
    db<Init, Machine>(TRC) << "Realview_PBX::init()" << endl;
    //Inicializações do realview aqui se necessário!!!
}

__END_SYS

#endif
