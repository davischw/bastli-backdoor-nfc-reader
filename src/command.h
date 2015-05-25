#include "token.h"
#include "json.h"
#include <string>

namespace Command {
	Json::Value registr(Token auth_token);
	Json::Value unregister(Token auth_token);
	Json::Value unknown_token(Token auth_token, Token token);
	Json::Value deactivated_device(Token auth_token, Token token);
	Json::Value access(Token auth_token, Token token);
	Json::Value flash(Token auth_token, Token token, Token device_token);
	Json::Value flashed(Token auth_token, Token token);
	Json::Value ping(Token auth_token);
	Json::Value pong(Token auth_token);
}
