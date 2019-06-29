#include <time.h>
#include <machine.h>
#include <synchronizer.h>
#include <process.h>
#include <machine.h>

using namespace EPOS;

struct MyLock: Spin
{
    void lock()   { Spin::acquire(); }
    void unlock() { Spin::release(); }
};

OStream cout;
MyLock unique;
const int THREADS = 16;
Thread * task[THREADS];

void io_bound(){
	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] Perform IO bound!" << endl;
	unique.unlock();

	Delay io_call(5000000);
}

void cpu_bound(){
	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] Perform CPU bound!" << endl;
	unique.unlock();

	CPU::halt();
}

int main()
{
	cout << "Migration test start!" << endl;
	cout << "Active CPUs: " << Machine::n_cpus() << endl;

	for (int tid = 0; tid < THREADS; tid++)
	{
		if((tid % 2) == 0){
			task[tid] = new Thread(&io_bound);
		}
		else{
			task[tid] = new Thread(&cpu_bound);
		}
		
	}

	for (int tid = 0; tid < THREADS; tid++)
	{
		task[tid]->join();
	}

	cout << "Migration test end!" << endl;
	return 0;
}