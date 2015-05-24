#include <ctime>

struct cache_entry{
	std::string token;
	std::time_t timestamp;
	std::string sound_path;
};