#include <boost/log/trivial.hpp>

#include "NfcContext.hpp"
#include "NfcDevice.hpp"

int main() {

  std::shared_ptr<NfcContext> context;

  try {
    context = NfcContext::init();
  } catch (NfcException &e) {
    BOOST_LOG_TRIVIAL(error) << "Failed to initialize NFC-Context";
    return 1;
  }

  auto connstrings = context->list_devices(5);

  if (connstrings.empty()) {
    BOOST_LOG_TRIVIAL(warning) << "No NFC device found";
    return 1;
  }

  if (connstrings.size() != 1) {
    BOOST_LOG_TRIVIAL(warning) << "Found more than one device, using first one";
  }

  BOOST_LOG_TRIVIAL(info) << "Using device " << connstrings[0];
  auto device = context->open(connstrings[0]);

  // TODO: List all tags, and then reset them
  return 0;
}
