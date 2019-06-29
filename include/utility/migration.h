// EPOS Migration Component Declarations

#ifndef __migration_h
#define __migration_h

#include <architecture.h>
#include <time.h>

__BEGIN_SYS

struct Migration_stats
{
    typedef Alarm::Tick Tick;

public:
    Migration_stats(): _elapsed_time_on{0,0} {}
    ~Migration_stats() {}
    
    void inc_elapsed_time_on(Bound bnd, Tick val) {
        _elapsed_time_on[bnd] += val;
    }
    Tick elapsed_time_on(Bound bnd) {
        return _elapsed_time_on[bnd];
    }
    Tick CPU_bound() {
        return (_elapsed_time_on[CPU] << (sizeof(Tick)*4) ) / (_elapsed_time_on[IO] +1);
    }
    void operator-=(Migration_stats other) {
        _elapsed_time_on[CPU] -= other._elapsed_time_on[CPU];
        _elapsed_time_on[IO]  -= other._elapsed_time_on[IO];
    }
    void operator+=(Migration_stats other) {
        _elapsed_time_on[CPU] += other._elapsed_time_on[CPU];
        _elapsed_time_on[IO]  += other._elapsed_time_on[IO];
    }

private:
    enum Bound { CPU, IO };
    Tick _elapsed_time_on[2];
};


struct Thread_stats: Migration_stats
{
public:
    Thread_stats(): _time_reference(0) {}
    ~Thread_stats() {}
    
    Tick update_time_reference(){
        Tick elapsed = Alarm::elapsed() - _time_reference;
        _time_reference = Alarm::elapsed();
        return elapsed;
    }

private:
    Tick _time_reference;
};

__END_SYS

#endif
