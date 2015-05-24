// Boost Logging
#include <boost/log/trivial.hpp>

#include "NfcTokenReader.hpp"
#include "opener.h"

int main() {

  BOOST_LOG_TRIVIAL(info) << "Starting NFC-Reader";

  config_struct config;
  config.use_logger = false;

  LockedQueue<std::string> token_read;

  Opener o(config, 600, &token_read);
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
