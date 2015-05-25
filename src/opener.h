#include <thread>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include "config.h"
#include "locked_queue.h"
#include "cache_entry.h"
#include <vector>
#include "json.h"
#include "token.h"

class Opener{
public:
	Opener(config_struct c, LockedQueue<Token>* queue_reader,
		LockedQueue<Json::Value>* queue_server_in, LockedQueue<Json::Value>* queue_server_out);
    ~Opener();

	config_struct config;
	
	void start();
	void stop();
	void open_to(std::string name);
	int display_text(std::string text);
	int display_ascii_art(std::string text);
    int play_sound(std::string sound_path);

private:
	std::thread opener;
	bool running;
	int logger;
	struct termios t_config;
	std::vector<cache_entry> token_cache;
	int cache_timeout;
	LockedQueue<Token>* queue_reader;
	LockedQueue<Json::Value>* queue_server_in;
	LockedQueue<Json::Value>* queue_server_out;
	std::vector<std::pair<Token, int>> tokens_waiting;
	
	void run();
};