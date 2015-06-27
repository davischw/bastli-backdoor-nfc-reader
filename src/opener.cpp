#include "opener.h"
#include "command.h"

#include <boost/format.hpp>

Opener::Opener(ConfigStruct c, LockedQueue<Token>* queue_reader,
	LockedQueue<Json::Value>* queue_server_in, BdClient& client):
	config(c),
	running(false),
	queue_reader(queue_reader),
	queue_server_in(queue_server_in),
        client(client)
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

	if (wiringPiSetupSys() == -1){
		printf("Configuration of wiringPi failed.\n");
		exit(1);
	}

        // Use the gpio command to export the pin
        // This allows non-root operation
	printf("Setting pin %i as output\n", DOOR_PIN);
        system((boost::format("gpio export %i out") % DOOR_PIN).str().c_str());
	pinMode(DOOR_PIN, OUTPUT);
}

Opener::~Opener() {
  if(opener.joinable())
	opener.join();
}

void Opener::run(){
    printf("Run()\n");

	while(running){

		if(queue_reader->size() > 0){
			printf("Got Job (queue_size: %i)!\n", queue_reader->size());
			auto token = queue_reader->pop();
			client.send(Command::access(config.client_token, token));
                        printf("Sent msg to bd_client (queue_size: %i)\n", queue_reader->size());
		}

		if(queue_server_in->size() > 0){
			/* new stuff from server */
			Json::Value request = queue_server_in->pop();
                        printf("New msg from server\n");

			if(request["cmd"]["method"] == "GRANT"){
				/* ask server for user info */
                                printf("Granting access...\n");
				cache_entry entry;
				entry.token = Token(request["cmd"]["params"][0].asString());
				entry.timestamp = std::time(nullptr);
				entry.sound_path = "";
				entry.name = "";
                                printf("Cache entry created\n");

				token_cache.push_back(entry);
				for(size_t i = 0; i < tokens_waiting.size(); i++){
					if (tokens_waiting[i].first == Token(request["params"][0].asString()))
					{
						tokens_waiting.erase(tokens_waiting.begin() + i);
					}
				}
				/* Display Text */
				/* Play Sound */
                                open_to("GRANT");
                                play_sound("");
                                printf("Access granted!\n");
			}
			else if(request["cmd"]["method"] == "DENY"){
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
			else if(request["cmd"]["method"] == "OPEN"){
				open_to("Opened by server request.");
				/* Display Standard Text */
				/* Play Standard Sound */
			}
		}
		//else{
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
		//}


            std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
	//system("sudo aplay /home/bastli/backdoor-nfc-reader/bin/mario_coin.wav");
	//system("sudo aplay /home/bastli/backdoor-nfc-reader/bin/mario_coin.wav");
	//system("sudo aplay /home/bastli/backdoor-nfc-reader/bin/mario_coin.wav");
	//system("sudo aplay /home/bastli/backdoor-nfc-reader/bin/mario_1up.wav");
	return 1;
}

void Opener::open_to(std::string user){
	printf("Open!\n");
        display_text("Opening door, welcome!"); 
	digitalWrite(DOOR_PIN, HIGH);
	sleep(1);
	digitalWrite(DOOR_PIN, LOW);
	printf("Opened!\n");
}
