#include <time.h>
#include <machine.h>
#include <synchronizer.h>
#include <process.h>
#include <machine.h>

using namespace EPOS;

struct MYLock: Spin
{
    void lock()   { Spin::acquire(); }
    void unlock() { Spin::release(); }
};

const unsigned int cpu_io_units[][2] = {
	{5,1},{4,2},{2,4},{1,5},
	{5,1},{4,2},{2,4},{1,5},
	{5,1},{4,2},{2,4},{1,5},
	{5,1},{4,2},{2,4},{1,5},
	{5,1},{4,2},{2,4},{1,5},
};
const unsigned second = 1000000;

const int nTasks = sizeof(cpu_io_units) / (2* sizeof(unsigned));
Thread * task[nTasks];

MYLock table;
OStream cout;

void CPU_bound(unsigned ticks) { while(ticks-- > 0) CPU::halt(); }
void IO_bound (unsigned ticks) { Delay(ticks * Alarm::timer_period()); }

void print(unsigned tid) {
	table.lock();
	Display::position(tid+1, 1);
	for(unsigned i = 0;  i < Machine::n_cpus(); ++i) cout << "                    ";
	Display::position(tid+1, Machine::cpu_id()*8 +3);
	switch (tid % 4)
	{
	case 0:	cout << "CPU";	break;
	case 1:	cout << "cpu";	break;
	case 2: cout << "i´o";	break;
	case 3: cout << "I/O";	break;
	default: break;
	}
	table.unlock();
}

int Thread_execution(unsigned tid) {
	while(true) {
		print(tid);
		CPU_bound(Alarm::ticks(second/nTasks * cpu_io_units[tid][0]));
		IO_bound (Alarm::ticks(second/nTasks * cpu_io_units[tid][1]));
	}
	return 0;
}

int main()
{
	Display::clear();

	for (unsigned tid = 0; tid < nTasks; tid++) {
		task[tid] = new Thread(&Thread_execution, tid);
	}

	for (unsigned tid = 0; tid < nTasks; tid++) {
		task[tid]->join();
		delete task[tid];
	}

	return 0;
}