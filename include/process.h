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
    friend class Thread_stats;          // for _scheduler.schedulables()

protected:
    static const bool smp = Traits<Thread>::smp;
    static const bool preemptive = Traits<Thread>::Criterion::preemptive;
    static const bool reboot = Traits<System>::reboot;

    static const unsigned int QUANTUM = Traits<Thread>::QUANTUM;
    static const unsigned int STACK_SIZE = Traits<Application>::STACK_SIZE;

    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Context Context;
    typedef Timer_Common::Tick Tick;

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

    // Migration
    struct Bound_stats {
        enum Bound { _CPU, _IO, _RDY };
    public:
        Bound_stats(): _elapsed_time_on{0,0} {}
        ~Bound_stats() {}
        void zero() {
            _elapsed_time_on[_CPU] = 0;
            _elapsed_time_on[_IO]  = 0;
            _elapsed_time_on[_RDY] = 0;
        }
        
        void inc_elapsed_time_on(Bound bnd, Tick val) volatile {
            _elapsed_time_on[bnd] += val;
        }
        double CPU_bound() const volatile { 
            return double(_elapsed_time_on[_CPU] +1) / (_elapsed_time_on[_CPU] + _elapsed_time_on[_IO] +2);
        }
        double RDY_bound() const volatile {
            return double(_elapsed_time_on[_RDY] +1) / (_elapsed_time_on[_CPU] + _elapsed_time_on[_IO] +2);
        }

        void operator-=(const Bound_stats &other) volatile {
            _elapsed_time_on[_CPU] -= other._elapsed_time_on[_CPU];
            _elapsed_time_on[_IO]  -= other._elapsed_time_on[_IO];
            _elapsed_time_on[_RDY] -= other._elapsed_time_on[_RDY];
        }
        void operator+=(const Bound_stats &other) volatile {
            _elapsed_time_on[_CPU] += other._elapsed_time_on[_CPU];
            _elapsed_time_on[_IO]  += other._elapsed_time_on[_IO];
            _elapsed_time_on[_RDY] += other._elapsed_time_on[_RDY];
        }

    private:
        Tick _elapsed_time_on[3];
    };

    struct Thread_stats: Bound_stats {
        static const unsigned int Q = Criterion::QUEUES;
    public:
        Thread_stats(Thread * self): _self(self), _time_reference(Alarm_elapsed()) {}
        ~Thread_stats() {
            queue_stats(rank().queue()) -= *this;
        }
        void finish() {
            queue_stats(rank().queue()) -= *this;
            this->zero();
        }
        
        volatile Criterion & rank() {
            return const_cast<volatile Criterion &>(_self->priority());
        }
        
        void inc_elapsed_time_on(Bound bnd, Tick val) {
            Bound_stats::inc_elapsed_time_on(bnd, val);
            queue_stats(rank().queue()).inc_elapsed_time_on(bnd, val);
        }
        Tick elapsed_time_reference() {
            Tick elapsed = Alarm_elapsed() - _time_reference;
            _time_reference = Alarm_elapsed();
            return elapsed;
        }

        static volatile Bound_stats & queue_stats(unsigned cpu) {
            return _queue_stats[cpu];
        }
        void check_migration() {
            unsigned min_index = 0;
            for(unsigned i = 1; i < Q; ++i)
                if(_queue_stats[i].RDY_bound()
                <  _queue_stats[min_index].RDY_bound()) {
                    min_index = i;
                }
            auto old_queue = rank().queue();
            if(// se a minha fila está tão cheia quanto a fila para qual quero migrar, e
                _queue_stats[old_queue].RDY_bound()
            >   _queue_stats[min_index].RDY_bound()
            && !(// se a fila para a qual quero migrar é CPU-Bound (tanto quanto a minha)
                   (queue_stats(old_queue).CPU_bound() < queue_stats(min_index).CPU_bound())
                // e a thread que quero migrar é IO-Bound (ou vice-versa)
                ^   (   ( queue_stats(old_queue).CPU_bound() + queue_stats(min_index).CPU_bound() ) /2
                    >   _self->_migration_stats.CPU_bound() ) )
            ) {
                queue_stats(old_queue)  -= *this;
                rank().queue(min_index);
                this->zero();
                elapsed_time_reference(); // renew
            }
        }

    private:
        Thread * _self;
        Tick _time_reference;
        
        static volatile Bound_stats _queue_stats[Q];
    };

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

    static Tick Alarm_elapsed();

protected:
    char * _stack;
    Context * volatile _context;
    volatile State _state;
    Queue * _waiting;
    Thread * volatile _joining;
    Queue::Element _link;
    Thread_stats _migration_stats;

    static volatile unsigned int _thread_count;
    static Scheduler_Timer * _timer;
    static Scheduler<Thread> _scheduler;
    static Spin _lock;
};


template<typename ... Tn>
inline Thread::Thread(int (* entry)(Tn ...), Tn ... an)
: _state(READY), _waiting(0), _joining(0), _link(this, NORMAL), _migration_stats(this)
{
    constructor_prologue(STACK_SIZE);
    _context = CPU::init_stack(0, _stack + STACK_SIZE, &__exit, entry, an ...);
    constructor_epilogue(entry, STACK_SIZE);
}

template<typename ... Tn>
inline Thread::Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an)
: _state(conf.state), _waiting(0), _joining(0), _link(this, conf.criterion), _migration_stats(this)
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
