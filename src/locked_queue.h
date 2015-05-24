#ifndef LOCKED_QUEUE_H
#define LOCKED_QUEUE_H

#include <thread>
#include <string>
#include <boost/thread/mutex.hpp>
#include <queue>

template <typename T>
class LockedQueue{
public:
	LockedQueue();

	void push(T);
	T pop();
	int size();
private:
	boost::mutex lock;
	std::queue<T> queue;
};

#include "locked_queue.cpp"

#endif
