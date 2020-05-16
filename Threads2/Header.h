#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <algorithm>

template <size_t N, class T>
class FixedThreadPool {
	std::mutex qm, qf;
	std::condition_variable done;
	std::deque<T> que;
	bool finished[N];

	void reset() {
		for (size_t i = 0; i < N; i++)
			finished[i] = false;
	}

	bool is_finished() {
		return std::all_of(finished, 
						   finished + N - 1, 
						   [](bool e) { return e; });
	}

	void thread_task(size_t id) {
		while (true) {
			std::unique_lock<std::mutex> lock(qm);
			if (que.empty()) {
				std::unique_lock<std::mutex> lock(qf);
				finished[id] = true;
				done.notify_all();
				return;
			}
			auto job = std::move(que.front());
			que.pop_front();
			lock.unlock();
			job();
		}
	}

public:

	void push(T&& job) {
		std::unique_lock<std::mutex> lock(qm);
		que.emplace_back(std::move(job));
	}

	void start() {
		reset();
		for (size_t i = 0; i < N; i++) {
			std::thread worker([this, i]() { thread_task(i); });
			worker.detach();
		}
	}

	void stop() {
		std::unique_lock<std::mutex> lock(qm);
		que.clear();
	}

	void wait_finished() {
		std::unique_lock<std::mutex> lock(qf);
		done.wait(lock, [this] {return is_finished(); });
	}
};