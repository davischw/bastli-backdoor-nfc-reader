#include "opener.h"

Opener::Opener(config_struct c, int cache_timeout, LockedQueue<std::string>* in):
	config(c),
	running(false){

	queue_in = in;

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
		if(queue_in->size() > 0){
			std::string token = queue_in->pop();
			/* try ask server for permission */
			/* no answer form server */
			for (auto entry : token_cache)
			{
				if (entry.token == token)
				{
					if(entry.timestamp < std::time(nullptr) - config.cache_token_timeout){
						/* Play Sound */
					}
				}
			}
			/* server answers */
			/* Ask server for permission */
			/* If yes */
				/* Store token to cache */
				/* Ask server for user data */
				/* Play Sound */
			/* if no search chache and delete token if found */
		}
		else{
			/* purge cache in sparetime */
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
