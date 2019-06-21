// EPOS CPU Affinity Scheduler Component Implementation

#include <utility/scheduler.h>
#include <time.h>

__BEGIN_UTIL

// Class attributes
volatile unsigned int Scheduling_Criteria::Variable_Queue::_next_queue;


// The following Scheduling Criteria depend on Alarm, which is not available at scheduler.h
namespace Scheduling_Criteria {
    FCFS::FCFS(int p): Priority((p == IDLE) ? IDLE : Alarm::elapsed()) {}

    HRRN::HRRN(int p): Priority(p), _create_time(Alarm::_elapsed) {}
    HRRN::operator const volatile int() const volatile {
        switch(_priority){
        case IDLE: return IDLE;
        default:
            // max( (w+s)/s ) <=> max( w/s ) <=> min( s/w )
            Tick waiting_time = Alarm::_elapsed - _create_time +1; // avoid division by zero
            int ratio = _priority / waiting_time;
            return ratio;
        }
    }
};

__END_UTIL
