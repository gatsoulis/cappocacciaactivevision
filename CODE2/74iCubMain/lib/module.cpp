#include "module.h"



Module::Module(std::string name) {
  m_name = name;
}

Module::~Module(){
}

void Module::setName(std::string name) {
  m_name = name;
}

std::string Module::getName() {
  return m_name;
}

double Module::execute_profiled(int times) {
	clock_t tv1, tv2;
	tv1 = clock();
	int i;
	for (i=0; i<times; i++) execute();
	tv2 = clock();
	return (double)((tv2 - tv1))*1000.0/(double)CLOCKS_PER_SEC/(double)times;
}

long Module::execute_profiled_ms() {
	unsigned long int startTime = getMsTime();
	execute();
	startTime = getMsTime() - startTime;
	return startTime;
}

long Module::execute_profiled_ms_stdout() {
	long startTime = execute_profiled_ms();
	std::cout << getName() << " took " << startTime << " ms to execute.\n";
	return startTime;
}

unsigned long int Module::getMsTime() {
	ftime(&m_timeStruct);
	return m_timeStruct.time * 1000 + m_timeStruct.millitm;
}
