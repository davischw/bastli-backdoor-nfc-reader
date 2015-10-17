#ifndef LOCKED_QUEUE_H
#define LOCKED_QUEUE_H

#include <thread>
#include <string>
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T> class LockedQueue{
public:
	LockedQueue();

	void push(T);
	T pop();
	int size();

	std::mutex lock;
        std::condition_variable object_added;
private:
	std::queue<T> queue;
};

#include "locked_queue.cpp"

#endif
