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

int composed_io_cpu_bound()
{
	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] Perform MORE IO!" << endl;
	unique.unlock();

	Delay io_call1(4000000); // thread release cpu???
	CPU::halt();
	Delay io_call2(6000000);

	return 0;
}

int composed_cpu_io_bound()
{
	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] Perform MORE CPU!" << endl;
	unique.unlock();

	CPU::halt();
	Delay io_call(1000000);
	CPU::halt();

	return 0;
}

int io_bound()
{
	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] Perform ONLY IO!" << endl;
	unique.unlock();

	Delay io_call(7000000);

	return 0;
}

int cpu_bound()
{
	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] Perform ONLY CPU!" << endl;
	unique.unlock();

	CPU::halt();

	return 0;
}

int main()
{
	cout << "Migration test start!" << endl;
	cout << "Active CPUs: " << Machine::n_cpus() << endl;

	for (int tid = 0; tid < THREADS; tid++)
	{
		if((tid % 2) == 0){
			if(tid % 3 == 0){
				task[tid] = new Thread(&composed_cpu_io_bound);
			}
			else{
				task[tid] = new Thread(&io_bound);
			}			
		}
		else{
			if(tid % 3 == 0){
				task[tid] = new Thread(&composed_io_cpu_bound);
			}
			else{
				task[tid] = new Thread(&cpu_bound);
			}
		}		
	}

	for (int tid = 0; tid < THREADS; tid++)
	{
		task[tid]->join();
	}

	cout << "Migration test end!" << endl;
	return 0;
}