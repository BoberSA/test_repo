#include "Header.h"

#include <string>
#include <iostream>
#include <cstdlib>

std::mutex cout_mutex;

void worker(char c, int ms) {
	std::unique_lock<std::mutex> lock(cout_mutex);
	std::cout << c << " [" << std::this_thread::get_id() << "] " << ms << " ms" << std::endl;
	lock.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int randint(int from, int to) {
	return int(double(rand()) / RAND_MAX * (to - from)) + from;
}

int main() {
	
	std::bind(worker, '#', 10)(); // <=> worker('#', 10);

	FixedThreadPool<4, std::function<void()>> pool;

	std::string jobs{"One string to rule them all!"};

	for (auto j : jobs) {
		pool.push(std::bind(worker, j, randint(10, 1000)));
	}

	pool.start();
	
//  Stop pool by clearing task queue after 1 second
//	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//	pool.stop();

	pool.wait_finished();
	
	system("pause");
	return 0;
}