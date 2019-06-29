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

const int THREADS = 16;
int heavy_io_start = 0;
int light_io_start = 0;
int heavy_io_end = 0;
int light_io_end = 0;
OStream cout;
MyLock unique;
Thread * task[THREADS];

int calc (int iterations) {
	int aux = 0;
	for (int i = 0; i < iterations; ++i)
	{
		for (int i = 0; i < iterations; ++i)
		{
			aux++;
		}
	}

	return aux;
}

int heterogeneous_io_bound()
{
	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] Perform MORE IO!" << endl;
	unique.unlock();

	calc(100);
	Delay io_call1(4000000); // thread release cpu
	calc(1000);
	Delay io_call2(6000000); // thread release cpu
	calc(100);

	return 0;
}

int heterogeneous_cpu_bound()
{
	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] Perform MORE CPU!" << endl;
	unique.unlock();

	calc(1000);
	Delay io_call(1000000); // thread release cpu
	calc(1000);

	return 0;
}

int homogeneus_io_bound(int io_time)
{
	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] START Homogeneus IO exec >> " << heavy_io_start << " <<" << endl;
	heavy_io_start++;
	unique.unlock();

	Delay io_call(io_time); // thread release cpu

	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] END Homogeneus IO exec >> " << heavy_io_end << " <<" << endl;
	heavy_io_end++;
	unique.unlock();

	return 0;
}

int homogeneus_cpu_bound(int iterations)
{
	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] START Homogeneus CPU performance!" << endl;
	unique.unlock();

	calc(iterations);

	unique.lock();
	cout << "CPU[" << Machine::cpu_id() << "] END Homogeneus Heavy CPU performance!" << endl;
	unique.unlock();

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
				task[tid] = new Thread(&heterogeneous_cpu_bound);
			}
			else{
				task[tid] = new Thread(&homogeneus_io_bound, 1000000);
			}			
		}
		else{
			if(tid % 3 == 0){
				task[tid] = new Thread(&heterogeneous_io_bound);
			}
			else{
				task[tid] = new Thread(&homogeneus_cpu_bound, 10000);
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