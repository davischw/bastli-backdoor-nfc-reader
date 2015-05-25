#include <ctime>
#include "token.h"

struct cache_entry{
	Token token;
	std::time_t timestamp;
	std::string sound_path;
	std::string name;
};
