// EPOS Application Scaffold and Application Component Implementation

#include <system.h>
#include <machine/cortex_a/machine.h>
#include <architecture/armv7/cache.h>
__BEGIN_SYS

// Application class attributes
char Application::_preheap[];
Heap * Application::_heap;

__END_SYS

__BEGIN_API

// Global objects
__USING_UTIL
OStream cout;
OStream cerr;

__END_API

extern "C" {
    void __pre_main() {
        enable_caches();
        EPOS::S::Machine::smp_barrier();
    }
}
