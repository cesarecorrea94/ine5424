// EPOS HRRN Scheduling Test Program

#include <machine/display.h>
#include <time.h>
#include <synchronizer.h>
#include <process.h>
#include <machine.h>

using namespace EPOS;

const int criterions[] = { // Sequence of created threads' criterions
    Thread::Criterion::LOW,
    Thread::Criterion::NORMAL,
    Thread::Criterion::HIGH,
    Thread::Criterion::LOW,
    Thread::Criterion::NORMAL,
    Thread::Criterion::HIGH,
    Thread::Criterion::LOW,
    Thread::Criterion::NORMAL,
    Thread::Criterion::HIGH,
    Thread::Criterion::LOW,
    Thread::Criterion::NORMAL,
    Thread::Criterion::HIGH,
};
const int nTasks = sizeof(criterions)/sizeof(int); // number of threads to be created (#criterions)
Thread * volatile task[nTasks];
const int first_burst = 3; // number of threads to be created at beginning
const int shift_ticks = (sizeof(int)-1)*8 -1; // shift on number of execution ticks per criterion (take less time and space on gantt)
const int creation_interval = (Thread::Criterion::HIGH >> (shift_ticks+1)) +1; // interval between threads creation

Spin table;
OStream cout;

enum pos {
    TID = 6,
    CT = 14,
    ST = 23,
    FT = 32,
};

template<typename T>
void info(int tid, pos p, T out) {
    // print information about a given thread (tid)
    table.acquire();
    Display::position(1+tid, p);
    cout << out;
    table.release();
}

void info() {
    // print the information table
    table.acquire();
    Display::position(1, 1);
    for(int tid = 0; tid < nTasks; tid++) {
        cout    << "TID:   ; " // thread id
                << "CT:    ; " // create time
                << "ST:    ; " // start time
                << "FT:    ; " // finish time
                << endl;
    }
    for(int tid = 0; tid < nTasks; tid++)
        info(tid, TID, tid);
    table.release();
}

void gantt(int tid, char c) {
    // plot on the gantt chart ('|' = creation ; 'X' = execution)
    table.acquire();
    Display::position(nTasks+1+tid, Alarm::elapsed() +1);
    cout << c;
    table.release();
}

int task_func(int tid, int ticks) {
    // thread function (simulates a thread running by 'ticks' ticks)
    info(tid, ST, Alarm::elapsed());
    for(int delay = 0; delay < ticks; delay++) {
        gantt(tid, '0'+Machine::cpu_id());
        CPU::halt();
    }
    info(tid, FT, Alarm::elapsed());
    return 0;
}

void create_thread() {
    // create a thread with its respective criterion, and print its creation time information
    static int tid = 0;
    Thread::Configuration conf(Thread::READY, criterions[tid]);
    task[tid] = new Thread(conf, &task_func, tid, criterions[tid] >> shift_ticks);
    info(tid, CT, Alarm::elapsed());
    gantt(tid, '|');
    tid++;
}

int main()
{
    Display::clear();
	CPU::halt();
	CPU::halt();
	CPU::halt();
    info();

    for(int i = 0; i < first_burst; i++) create_thread();
    Function_Handler f_handler(&create_thread);
    Alarm alarme(Alarm::timer_period() * creation_interval,
                &f_handler, nTasks -first_burst);

    for(int tid = 0; tid < nTasks; ++tid) {
        while(task[tid] == 0) CPU::halt();
        int ret = task[tid]->join();
        delete task[tid];
    }

    Display::position(2*nTasks+1, 1);
    return 0;
}

