// Boost Logging
#include <boost/log/trivial.hpp>

#include "json.h"
#include "NfcTokenReader.hpp"
#include "opener.h"
#include "token.h"

int main() {

  BOOST_LOG_TRIVIAL(info) << "Starting NFC-Reader";

  config_struct config;
  config.use_logger = false;
  config.cache_token_timeout = 600;

  LockedQueue<Token> token_read;
  LockedQueue<Json::Value> server_in;
  LockedQueue<Json::Value> server_out;

  Opener o(config, &token_read, &server_in, &server_out);
  NfcTokenReader reader(&token_read);

  o.start();
  reader.start();

  std::this_thread::sleep_for(std::chrono::seconds(10));

  BOOST_LOG_TRIVIAL(trace) << "Trying to stop thread...";

  reader.stop();
  o.stop();

  BOOST_LOG_TRIVIAL(info) << "NFC-Reader exiting normally";

  return 0;
}
