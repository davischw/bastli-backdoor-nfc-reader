#include "NfcTokenReader.hpp"

void NfcTokenReader::start() {
  if (!_running) {
    _running = true;
    _thread = std::thread(&NfcTokenReader::run, this);
  }
}

void NfcTokenReader::stop() { _running = false; }

NfcDevice NfcTokenReader::initialize_device() {

  std::shared_ptr<NfcContext> context;

  // Setup nfc context
  context = NfcContext::init();

  // Look for NFC readers
  auto devices = context->list_devices(5);

  if (devices.empty()) {
    throw NfcException("No devices found!");
  }

  BOOST_LOG_TRIVIAL(info) << "Found " << devices.size() << " devices";

  for (size_t i = 0; i < devices.size(); i++) {
    BOOST_LOG_TRIVIAL(debug) << "Device " << i + 1 << ": " << devices[i];
  }

  // Just connect to the first device
  return context->open(devices[0]);
}

void NfcTokenReader::run() {
  // get the nfc device

  try {
    auto device = initialize_device();

    // continuosly poll for an NFC Tag

    while (_running) {
      auto tokens = poll(device);

      for (auto token : tokens) {
        BOOST_LOG_TRIVIAL(trace) << "Read token " << token;
      }
    }

  } catch (NfcException &e) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to initialize NFC Device";
    return;
  }
}

std::vector<uint32_t> NfcTokenReader::poll(NfcDevice &device) {
  MifareTag *tags = nullptr;
  tags = freefare_get_tags(device.getRawPointer());

  std::vector<uint32_t> tokens;

  if (tags == nullptr) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to poll tags";
  } else {

    auto i = 0;
    for (; tags[i]; i++) {
      auto t = read_tag(tags[i]);

      if (t != 0) {
        tokens.push_back(t);
      }
    }

    BOOST_LOG_TRIVIAL(info) << "Processed " << i << " tokens";
    free(tags);
  }

  return tokens;
}

uint32_t NfcTokenReader::read_tag(MifareTag tag) {
  // TODO: read token from the tag...
  return 0;
}
