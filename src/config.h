#include "token.h"

struct config_struct{
	bool use_logger;
	int cache_token_timeout;
	int server_timeout;
	Token opener_token;
	Token server_token;
};
