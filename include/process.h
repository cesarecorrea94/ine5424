// EPOS Thread Component Declarations

#ifndef __process_h
#define __process_h

#include <architecture.h>
#include <utility/queue.h>
#include <utility/handler.h>
#include <utility/scheduler.h>
#include <machine/timer.h>
#include <time.h>

extern "C" { void __exit(); }

__BEGIN_SYS

class Thread
{
    friend class Init_First;            // context->load()
    friend class Init_System;           // for init() on CPU != 0
    friend class Scheduler<Thread>;     // for link()
    friend class Synchronizer_Common;   // for lock() and sleep()
    friend class Alarm;                 // for lock()
    friend class System;                // for init()
    friend class IC;                    // for link() for priority ceiling

protected:
    static const bool smp = Traits<Thread>::smp;
    static const bool preemptive = Traits<Thread>::Criterion::preemptive;
    static const bool reboot = Traits<System>::reboot;

    static const unsigned int QUANTUM = Traits<Thread>::QUANTUM;
    static const unsigned int STACK_SIZE = Traits<Application>::STACK_SIZE;

    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Context Context;

public:
    // Thread State
    enum State {
        RUNNING,
        READY,
        SUSPENDED,
        WAITING,
        FINISHING
    };

    // Thread Priority
    typedef Scheduling_Criteria::Priority Priority;

    // Thread Scheduling Criterion
    typedef Traits<Thread>::Criterion Criterion;
    enum {
        HIGH    = Criterion::HIGH,
        NORMAL  = Criterion::NORMAL,
        LOW     = Criterion::LOW,
        MAIN    = Criterion::MAIN,
        IDLE    = Criterion::IDLE
    };

    // Thread Configuration
    struct Configuration {
        Configuration(const State & s = READY, const Criterion & c = NORMAL, unsigned int ss = STACK_SIZE)
        : state(s), criterion(c), stack_size(ss) {}

        State state;
        Criterion criterion;
        unsigned int stack_size;
    };

    // Thread Queue
    typedef Ordered_Queue<Thread, Criterion, Scheduler<Thread>::Element> Queue;

    // static unsigned Alarm_elapsed() { return 0; }
    // // Migration
    // struct Bound_stats {
    //     typedef unsigned Tick;
    //     enum Bound { _CPU, _IO };
    // public:
    //     Bound_stats(): _elapsed_time_on{1,1} {}
    //     ~Bound_stats() {}
        
    //     void inc_elapsed_time_on(Bound bnd, Tick val) {
    //         _elapsed_time_on[bnd] += val;
    //     }
    //     double CPU_bound() { 
    //         return _elapsed_time_on[_CPU] / (_elapsed_time_on[_CPU] + _elapsed_time_on[_IO]);
    //     }
    
    // /* protected: */
    //     void operator-=(const Bound_stats &other) {
    //         _elapsed_time_on[_CPU] -= other._elapsed_time_on[_CPU];
    //         _elapsed_time_on[_IO]  -= other._elapsed_time_on[_IO];
    //     }
    //     void operator+=(const Bound_stats &other) {
    //         _elapsed_time_on[_CPU] += other._elapsed_time_on[_CPU];
    //         _elapsed_time_on[_IO]  += other._elapsed_time_on[_IO];
    //     }

    // private:
    //     Tick _elapsed_time_on[2];
    // };

    // struct Thread_stats: Bound_stats {
    //     static const unsigned int Q = Criterion::QUEUES;
    //     using Bound_stats::Tick;
    // public:
    //     Thread_stats(): _time_reference(Alarm_elapsed()) {}
    //     ~Thread_stats() {}
        
    //     void inc_elapsed_time_on(Bound bnd, Tick val) {
    //         Bound_stats::inc_elapsed_time_on(bnd, val);
    //         queue_stats().inc_elapsed_time_on(bnd, val);
    //     }
    //     Tick update_time_reference() {
    //         Tick elapsed = Alarm_elapsed() - _time_reference;
    //         _time_reference = Alarm_elapsed();
    //         return elapsed;
    //     }

    //     static Bound_stats & queue_stats(unsigned cpu=0/*  = Criterion::current_queue() */) {
    //         return _queue_stats[cpu];
    //     }
    //     static void upd_mean_time_on_ready_state(Tick val) {
    //         auto size = _scheduler.schedulables();
    //         Tick &mean_time = _mean_time_on_ready_state[0/* Criterion::current_queue() */];
    //         mean_time = (mean_time*size + val) / (size+1);
    //     }
    //     static void check_migration(Thread * self) {
    //         unsigned min_index = 0;
    //         for(unsigned i = 1; i < Q; ++i)
    //             if(_mean_time_on_ready_state[i] < _mean_time_on_ready_state[min_index]) {
    //                 min_index = i;
    //             }
    //         if(// se a minha fila está tão cheia quanto a para qual quero migrar, e
    //             _mean_time_on_ready_state[0/* Criterion::current_queue() */] > _mean_time_on_ready_state[min_index] *3/2
    //         && !(// se a fila para a qual quero migrar é CPU-Bound (tanto quanto a minha)
    //                (queue_stats().CPU_bound() < queue_stats(min_index).CPU_bound())
    //             // e a thread que quero migrar é IO-Bound (ou vice-versa)
    //             ^  (queue_stats().CPU_bound() > self->_migration_stats.CPU_bound()) )
    //         ) {
    //             self->criterion().queue(Criterion::current_queue());
    //             queue_stats(min_index)  -= self->_migration_stats;
    //             queue_stats()           += self->_migration_stats;
    //         }
    //     }

    // private:
    //     Tick _time_reference;
        
    //     // tempo de espera é dependente da fila, não algo característico da thread
    //     static Tick _mean_time_on_ready_state[Q];
    //     static Bound_stats _queue_stats[Q];
    // };
    // friend class Thread_stats;
    // friend class Bound_stats;

public:
    template<typename ... Tn>
    Thread(int (* entry)(Tn ...), Tn ... an);
    template<typename ... Tn>
    Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an);
    ~Thread();

    const volatile State & state() const { return _state; }

    const volatile Criterion & priority() const { return _link.rank(); }
    void priority(const Criterion & p);

    int join();
    void pass();
    void suspend() { suspend(false); }
    void resume();

    static Thread * volatile self() { return running(); }
    static void yield();
    static void exit(int status = 0);

protected:
    void constructor_prologue(unsigned int stack_size);
    void constructor_epilogue(const Log_Addr & entry, unsigned int stack_size);

    void state(State newState);

    Criterion & criterion() { return const_cast<Criterion &>(_link.rank()); }
    Queue::Element * link() { return &_link; }

    void suspend(bool locked);

    static Thread * volatile running() { return _scheduler.chosen(); }

    static void lock() {
        CPU::int_disable();
        if(smp)
            _lock.acquire();
    }

    static void unlock() {
        if(smp)
            _lock.release();
        CPU::int_enable();
    }

    static volatile bool locked() { return (smp) ? _lock.taken() : CPU::int_disabled(); }

    static void sleep(Queue * q);
    static void wakeup(Queue * q);
    static void wakeup_all(Queue * q);

    static void reschedule();
    static void reschedule(unsigned int cpu);
    static void rescheduler(const IC::Interrupt_Id & interrupt);
    static void time_slicer(const IC::Interrupt_Id & interrupt);

    static void dispatch(Thread * prev, Thread * next, bool charge = true);

    static int idle();

private:
    static void init();

protected:
    char * _stack;
    Context * volatile _context;
    volatile State _state;
    Queue * _waiting;
    Thread * volatile _joining;
    Queue::Element _link;
    // Thread_stats _migration_stats;

    static volatile unsigned int _thread_count;
    static Scheduler_Timer * _timer;
    static Scheduler<Thread> _scheduler;
    static Spin _lock;
};

template<typename ... Tn>
inline Thread::Thread(int (* entry)(Tn ...), Tn ... an)
:   _state(READY), _waiting(0), _joining(0), _link(this, NORMAL)
{
    constructor_prologue(STACK_SIZE);
    _context = CPU::init_stack(0, _stack + STACK_SIZE, &__exit, entry, an ...);
    constructor_epilogue(entry, STACK_SIZE);
}

template<typename ... Tn>
inline Thread::Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an)
:   _state(conf.state), _waiting(0), _joining(0), _link(this, conf.criterion)
{
    constructor_prologue(conf.stack_size);
    _context = CPU::init_stack(0, _stack + conf.stack_size, &__exit, entry, an ...);
    constructor_epilogue(entry, conf.stack_size);
}


// An event handler that triggers a thread (see handler.h)
class Thread_Handler : public Handler
{
public:
    Thread_Handler(Thread * h) : _handler(h) {}
    ~Thread_Handler() {}

    void operator()() { _handler->resume(); }

private:
    Thread * _handler;
};


// A Java-like Active Object
class Active: public Thread
{
public:
    Active(): Thread(Configuration(Thread::SUSPENDED), &entry, this) {}
    virtual ~Active() {}

    virtual int run() = 0;

    void start() { resume(); }

private:
    static int entry(Active * runnable) { return runnable->run(); }
};

__END_SYS

#endif
