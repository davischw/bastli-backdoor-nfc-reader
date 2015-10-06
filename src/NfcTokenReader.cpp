#include "NfcTokenReader.hpp"
#include "NfcTagList.hpp"
#include "MifareDesfireKey.hpp"

#include <chrono>

NfcTokenReader::NfcTokenReader(LockedQueue<Token>* queue) : _queue(queue) {};

NfcTokenReader::~NfcTokenReader() {
    if (_thread.joinable()) {
      _thread.join();
    }
}

void NfcTokenReader::start() {
  if (!_running) {
    _running = true;
    _thread = std::thread(&NfcTokenReader::run, this);
  }
}

void NfcTokenReader::stop() { _running = false; }

std::shared_ptr<NfcDevice> NfcTokenReader::initialize_device() {

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
    BOOST_LOG_TRIVIAL(info) << "Connection to device established";

    while (_running) {
      auto start = std::chrono::system_clock::now();
      auto tokens = poll(device);
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

      BOOST_LOG_TRIVIAL(debug) << "Got token in " << duration.count() << "ms";

      for (auto token : tokens) {
        // add print support for token...
        BOOST_LOG_TRIVIAL(trace) << "Read token xxxx with size " << token.size();
        _queue->push(token);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_ms));
    }

  } catch (NfcException &e) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to initialize NFC Device";
    return;
  }
}

std::vector<Token> NfcTokenReader::poll(std::shared_ptr<NfcDevice> device) {
  auto tags = device->list_tags();

  std::vector<Token> tokens;

  if (tags.get_raw() == nullptr) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to poll tags";
  } else {

    auto i = 0;
    for (; tags.get_raw()[i]; i++) {
      BOOST_LOG_TRIVIAL(trace) << "Adding token to vector";

      if (auto t = read_tag(tags.get_raw()[i])) {
        tokens.push_back(*t);
      }
    }

    BOOST_LOG_TRIVIAL(debug) << "Processed " << i << " tokens";
  }

  return tokens;
}

boost::optional<Token> NfcTokenReader::read_tag(MifareTag tag) {
  BOOST_LOG_TRIVIAL(trace) << "Reading tag";

  auto start = std::chrono::system_clock::now();

  // verify Tag type
  if (DESFIRE != freefare_get_tag_type(tag)) {
    BOOST_LOG_TRIVIAL(debug) << "Tag is not a Desfire card";
    return boost::none;
  }
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  BOOST_LOG_TRIVIAL(debug) << "Verified tag type in " << duration.count() << "ms";

  // Connect
  int res;
  res = mifare_desfire_connect(tag);
  if (res < 0) {
    BOOST_LOG_TRIVIAL(debug) << "Failed to connect to Mifare DESFire";
    return boost::none;
  }
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  BOOST_LOG_TRIVIAL(debug) << "Connect to tag in " << duration.count() << "ms";

  struct mifare_desfire_version_info info;
  res = mifare_desfire_get_version(tag, &info);
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_get_version");
    return boost::none;
  }
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  BOOST_LOG_TRIVIAL(debug) << "Got version after " << duration.count() << "ms";

  if (info.software.version_major < 1) {
    // for some reason old software doesn't work...
    BOOST_LOG_TRIVIAL(debug) << "Software is too old, not using card";
    return boost::none;
  }

  const uint8_t bastli_key_version = 1;
  uint8_t bastli_key[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
  auto new_key = MifareDesfireKey::create_aes_key_with_version(
      bastli_key, bastli_key_version);

  duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  BOOST_LOG_TRIVIAL(debug) << "Start auth after " << duration.count() << "ms";

  res = mifare_desfire_authenticate(tag, 0, new_key.get_raw());
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_authenticate");
    BOOST_LOG_TRIVIAL(warning) << "Card is not using "
                                  "bastli key, ignoring "
                                  "card";
    return boost::none;
  }

  duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  BOOST_LOG_TRIVIAL(debug) << "Authenticated to tag after after " << duration.count() << "ms";

  const uint32_t bastli_backdoor_aid = 0x5;

  uint8_t null_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  auto default_key = MifareDesfireKey::create_des_key(null_key);

  MifareDESFireAID bastli_aid = mifare_desfire_aid_new(bastli_backdoor_aid);
  if (bastli_aid == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "Failed to create Bastli AID";
    freefare_perror(tag, "mifare_desfire_aid_new");
    return boost::none;
  }

  res = mifare_desfire_select_application(tag, bastli_aid);
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_select_application");

    free(bastli_aid);
    return boost::none;
  }
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  BOOST_LOG_TRIVIAL(debug) << "Selected application in " << duration.count() << "ms";

  // authenticate with default_key
  res = mifare_desfire_authenticate(tag, 0, default_key.get_raw());
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_authenticate");

    free(bastli_aid);
    return boost::none;
  }
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  BOOST_LOG_TRIVIAL(debug) << "Authenticated to application in" << duration.count() << "ms";

  // The mifare_desfire_read_data function is broken
  // (see
  // https://github.com/nfc-tools/libfreefare/commit/a0ba196b498979921b9a9247771b816bbfec014f)
  // so we have to prepare a bigger buffer than actually needed...
  Token token;

  std::array<char, sizeof(token) + 10> buffer;
  BOOST_LOG_TRIVIAL(trace) << "Buffer size: " << sizeof(buffer);

  // we want to read one token, but place it in the buffer...
  res = mifare_desfire_read_data(tag, 0, 0, sizeof(token), buffer.data());

  int tokensize = sizeof(token);

  if (res < tokensize) {
    BOOST_LOG_TRIVIAL(warning) << "Did not read enough data!";
  } else if (res > tokensize) {
    BOOST_LOG_TRIVIAL(error) << "Did read too much data!";
  }

  if (res < 0) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to read data...";
    freefare_perror(tag, "mifare_desfire_read_data");

    free(bastli_aid);
    return boost::none;
  }


  std::copy(buffer.begin(), buffer.begin() + token.size(), token.begin());

  BOOST_LOG_TRIVIAL(trace) << "Successfully read token from card: " << token.to_string();

  free(bastli_aid);

  BOOST_LOG_TRIVIAL(trace) << "Freed bastli_aid";
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  BOOST_LOG_TRIVIAL(debug) << "read_tag in " << duration.count() << "ms";

  return token;
}
