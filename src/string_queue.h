#include <thread>
#include <string>
#include <boost/thread/mutex.hpp>
#include <queue>

class StringQueue{
public:
	StringQueue();
	void push(std::string);
	std::string pop();
	int size();
private:
	boost::mutex lock;
	std::queue<std::string> queue;
};