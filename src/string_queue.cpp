#include "string_queue.h"

	StringQueue::StringQueue(){
		queue = std::queue<std::string>();
	}

	void StringQueue::push(std::string token){
		lock.lock();
		queue.push(token);
		lock.unlock();
	}

	std::string StringQueue::pop(){
		lock.lock();
		std::string return_value = queue.front();
		queue.pop();
		lock.unlock();
		return return_value;
	}

	int StringQueue::size(){
		lock.lock();
                auto val = queue.size();
		lock.unlock();
                return val;
	}
