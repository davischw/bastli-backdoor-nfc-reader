// Boost Logging
#include <boost/log/trivial.hpp>

#include "NfcTokenReader.hpp"

int main() {

  BOOST_LOG_TRIVIAL(info) << "Starting NFC-Reader";
  NfcTokenReader reader;

  reader.start();

  std::this_thread::sleep_for(std::chrono::seconds(10));

  BOOST_LOG_TRIVIAL(trace) << "Trying to stop thread...";

  reader.stop();

  BOOST_LOG_TRIVIAL(info) << "NFC-Reader exiting normally";

  return 0;
}
