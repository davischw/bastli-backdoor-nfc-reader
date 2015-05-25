#include "opener.h"
#include "command.h"

Opener::Opener(config_struct c, LockedQueue<Token>* queue_reader,
	LockedQueue<Json::Value>* queue_server_in, LockedQueue<Json::Value>* queue_server_out):
	config(c),
	running(false),
	queue_reader(queue_reader),
	queue_server_in(queue_server_in),
	queue_server_out(queue_server_out)
	{

	if(c.use_logger == true){
		logger = open("/dev/ttyUSB0", O_RDWR);
		if(logger < 0){
			printf("Could not open /dev/ttyUSB0.\n");
			exit(1);
		}

		if(tcgetattr(logger, &t_config) < 0) {
			printf("Getting current deviceconfig failed.\n");
			exit(1);
		}

		if(cfsetispeed(&t_config, B38400) < 0 || cfsetospeed(&t_config, B38400) < 0) {
			printf("Could not set baudrate to 38400.\n");
			exit(1);
		}

		if(tcsetattr(logger, TCSAFLUSH, &t_config) < 0) {
			printf("Configuration of file failed.\n");
			exit(1);
		}

}

	if (wiringPiSetup() == -1){
		printf("Configuration of wiringPi failed.\n");
		exit(1);
	}

	printf("Setting pin 29 as output\n");
	pinMode(29, OUTPUT);
}

Opener::~Opener() {
  if(opener.joinable())
	opener.join();
}

void Opener::run(){
    printf("Run()\n");

	while(running){
		if(queue_reader->size() > 0){
			printf("Got Job!");
			auto token = queue_reader->pop();
			queue_server_out->push(Command::access(config.opener_token, token));
		}
		else if(queue_server_in->size() > 0){
			/* new stuff from server */
			Json::Value request = queue_server_in->pop();

			if(request["cmd"] == "GRANT"){
				/* ask server for user info */
				cache_entry entry;
				entry.token = Token(request["params"][0].asString());
				entry.timestamp = std::time(nullptr);
				entry.sound_path = "";
				entry.name = "";
				token_cache.push_back(entry);
				for(size_t i = 0; i < tokens_waiting.size(); i++){
					if (tokens_waiting[i].first == Token(request["params"][0].asString()))
					{
						tokens_waiting.erase(tokens_waiting.begin() + i);
					}
				}
				/* Display Text */
				/* Play Sound */
			}
			else if(request["cmd"] == "DENY"){
				for(size_t i = 0; i < tokens_waiting.size(); i++){
					if (tokens_waiting[i].first == Token(request["params"][0].asString()))
					{
						tokens_waiting.erase(tokens_waiting.begin() + i);
					}
				}
				for (size_t i = 0; i < token_cache.size(); i++){
					if (token_cache[i].token == Token(request["params"][0].asString())){
						/* remove token from cache */
						token_cache.erase(token_cache.begin() + i);
					}
				}
			}
			else if(request["cmd"] == "OPEN"){
				open_to("Opened by server request.");
				/* Display Standard Text */
				/* Play Standard Sound */
			}
		}
		else{
			/* No stuff to do, check for timeout on server and check cache */
			for(size_t j = 0; j < tokens_waiting.size(); j++){
				if(tokens_waiting[j].second < config.server_timeout){
					for (size_t i = 0; i < token_cache.size(); i++){
						if (token_cache[i].token == tokens_waiting[j].first){
							if(token_cache[i].timestamp < std::time(nullptr) - config.cache_token_timeout){
								open_to("Opened by server request.");
								/* Play Sound */
							}
							else{
								/* remove token from cache */
								token_cache.erase(token_cache.begin() + i);
							}
							break;
						}
					}
					tokens_waiting.erase(tokens_waiting.begin() + j);
					break;
				}
			}
		}
	}
    printf("Finished running\n");
}

void Opener::start(){
	running = true;
	opener = std::thread(&Opener::run, this);
}

void Opener::stop(){
	running = false;
}

int Opener::display_ascii_art(std::string text){
	if(config.use_logger == true){	
		std::string figlet = std::string("figlet -w 80 ") + text;
		FILE *pipe = popen(figlet.c_str(), "r");
		if (!pipe){
			return 0;
		}
		char buffer[128];
		while(!feof(pipe)) {
			if(fgets(buffer, 128, pipe) != NULL){
				write(logger, buffer, strlen(buffer));
			}
		}
		pclose(pipe);
		return 1;
	}
	return -1;
}

int Opener::display_text(std::string text){
	if(config.use_logger == true){
		write(logger, (text + "\r\n").c_str(), text.length() + 2);
		return 1;
	}
	return 0;
}

int Opener::play_sound(std::string sound_path){
	return 1;
}

void Opener::open_to(std::string user){
	printf("Open!\n");
	digitalWrite(29, HIGH);
	sleep(1);
	digitalWrite(29, LOW);
	printf("Opened!\n");
}
