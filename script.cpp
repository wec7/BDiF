#include <iostream>
#include <thread>

static const int NUM_THREADS = 10;

// Thread function. When a thread is launched, this is the code

// that gets executed.

void ThreadFunction(int threadID) {
	std::cout << "Hello from thread #" << threadID << std::endl;
}

int main() {
	std::thread thread[NUM_THREADS];

	// Launch threads.

	for (int i = 0; i < NUM_THREADS; ++i) {
		thread[i] = std::thread(ThreadFunction, i);
	}
	std::cout << NUM_THREADS << " threads launched." << std::endl;

	// Join threads to the main thread of execution.

	for (int i = 0; i < NUM_THREADS; ++i) {
		thread[i].join();
	}
	return 0;
	// abcd
}