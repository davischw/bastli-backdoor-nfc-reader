#include "locked_queue.h"

template <typename T> LockedQueue<T>::LockedQueue(){
	queue = std::queue<T>();
}

template <typename T> void LockedQueue<T>::push(T token){
	lock.lock();
	queue.push(token);
	lock.unlock();

	object_added.notify_all();
}


template <typename T> T LockedQueue<T>::pop(){
	lock.lock();
	T return_value = queue.front();
	queue.pop();
	lock.unlock();
	return return_value;
}

template <typename T> int LockedQueue<T>::size(){
	lock.lock();
	auto val = queue.size();
	lock.unlock();
	return val;
}
