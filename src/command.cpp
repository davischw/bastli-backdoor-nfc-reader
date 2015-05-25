#include "command.h"

namespace Command { 

Json::Value registr(Token auth_token){
    Json::Value query;
    query["auth"]["token"] = auth_token.to_string();
    query["cmd"]["method"] = "REGISTER";
    return query;
}

Json::Value _unregister(Token auth_token){
    Json::Value query;
    query["auth"]["token"] = auth_token.to_string();
    query["cmd"]["method"] = "UNREGISTER";
    return query;
}

Json::Value unknown_token(Token auth_token, Token token){
    Json::Value query;
    query["auth"]["token"] = auth_token.to_string();
    query["cmd"]["method"] = "UNKNOWN";
    query["cmd"]["params"] = Json::arrayValue;
    query["cmd"]["params"].append(token.to_string());
    return query;
}

Json::Value deactivated_device(Token auth_token, Token token){
    Json::Value query;
    query["auth"]["token"] = auth_token.to_string();
    query["cmd"]["method"] = "DEACTIVATED";
    query["cmd"]["params"] = Json::arrayValue;
    query["cmd"]["params"].append(token.to_string());
    return query;
}

Json::Value access(Token auth_token, Token token){
    Json::Value query;
    query["auth"]["token"] = auth_token.to_string();
    query["cmd"]["method"] = "ACCESS";
    query["cmd"]["params"] = Json::arrayValue;
    query["cmd"]["params"].append(token.to_string());
    return query;
}

Json::Value flash(Token auth_token, Token token, Token device_token){
    Json::Value query;
    query["auth"]["token"] = auth_token.to_string();
    query["cmd"]["method"] = "FLASH";
    query["cmd"]["params"] = Json::arrayValue;
    query["cmd"]["params"].append(token.to_string());

    // TODO: why check device_token?
    //if(device_token != ""){
        query["cmd"]["params"].append(device_token.to_string());
    //}
    return query;
}

Json::Value flashed(Token auth_token, Token token){
    Json::Value query;
    query["auth"]["token"] = auth_token.to_string();
    query["cmd"]["method"] = "FLASHED";
    query["cmd"]["params"] = Json::arrayValue;
    query["cmd"]["params"].append(token.to_string());
    return query;
}

Json::Value ping(Token auth_token){
    Json::Value query;
    query["auth"]["token"] = auth_token.to_string();
    query["cmd"]["method"] = "PING";
    return query;
}

Json::Value pong(Token auth_token){
    Json::Value query;
    query["auth"]["token"] = auth_token.to_string();
    query["cmd"]["method"] = "PONG";
    return query;
}

}
