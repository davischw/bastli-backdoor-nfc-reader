#ifndef CONFIG_STRUCT_H
#define CONFIG_STRUCT_H

#include "token.h"

struct ConfigStruct{
	bool use_logger;
        std::string logger_path;

	int cache_token_timeout;
	int server_timeout;

	std::string hostname;
	std::string port; //socket library requires port as a string

	Token	      server_token;
	Token       client_token;
};
#endif //CONFIG_STRUCT_H
