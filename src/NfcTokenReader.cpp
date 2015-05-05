#include "NfcTokenReader.hpp"
#include "NfcTagList.hpp"
#include "MifareDesfireKey.hpp"

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
      auto tokens = poll(device);

      for (uint32_t token : tokens) {
        BOOST_LOG_TRIVIAL(trace) << "Read token " << token;
      }
    }

  } catch (NfcException &e) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to initialize NFC Device";
    return;
  }
}

std::vector<uint32_t> NfcTokenReader::poll(std::shared_ptr<NfcDevice> device) {
  auto tags = device->list_tags();

  std::vector<uint32_t> tokens;

  if (tags.get_raw() == nullptr) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to poll tags";
  } else {

    auto i = 0;
    for (; tags.get_raw()[i]; i++) {
      auto t = read_tag(tags.get_raw()[i]);

      BOOST_LOG_TRIVIAL(trace) << "Adding token to vector";

      if (t != 0) {
        tokens.push_back(t);
      }
    }

    BOOST_LOG_TRIVIAL(info) << "Processed " << i << " tokens";
  }

  return tokens;
}

uint32_t NfcTokenReader::read_tag(MifareTag tag) {
  BOOST_LOG_TRIVIAL(trace) << "Reading tag";

  // verify Tag type
  if (DESFIRE != freefare_get_tag_type(tag)) {
    BOOST_LOG_TRIVIAL(debug) << "Tag is not a Desfire card";
    return 0;
  }

  // Connect
  int res;
  res = mifare_desfire_connect(tag);
  if (res < 0) {
    BOOST_LOG_TRIVIAL(debug) << "Failed to connect to Mifare DESFire";
    return 0;
  }

  struct mifare_desfire_version_info info;
  res = mifare_desfire_get_version(tag, &info);
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_get_version");
    return 0;
  }

  if (info.software.version_major < 1) {
    // for some reason old software doesn't work...
    BOOST_LOG_TRIVIAL(debug) << "Software is too old, not using card";
    return 0;
  }

  const uint8_t bastli_key_version = 1;
  uint8_t bastli_key[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
  auto new_key = MifareDesfireKey::create_aes_key_with_version(
      bastli_key, bastli_key_version);

  res = mifare_desfire_authenticate(tag, 0, new_key.get_raw());
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_authenticate");
    BOOST_LOG_TRIVIAL(warning) << "Card is not using "
                                  "bastli key, ignoring "
                                  "card";
    return 0;
  }

  const uint32_t bastli_backdoor_aid = 0x5;

  uint8_t null_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  auto default_key = MifareDesfireKey::create_des_key(null_key);

  MifareDESFireAID bastli_aid = mifare_desfire_aid_new(bastli_backdoor_aid);
  if (bastli_aid == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "Failed to create Bastli AID";
    freefare_perror(tag, "mifare_desfire_aid_new");
    return 0;
  }

  res = mifare_desfire_select_application(tag, bastli_aid);
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_select_application");

    free(bastli_aid);
    return 0;
  }

  // authenticate with default_key
  res = mifare_desfire_authenticate(tag, 0, default_key.get_raw());
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_authenticate");

    free(bastli_aid);
    return 0;
  }

  // The mifare_desfire_read_data function is broken
  // (see
  // https://github.com/nfc-tools/libfreefare/commit/a0ba196b498979921b9a9247771b816bbfec014f)
  // so we have to prepare a bigger buffer than actually needed...
  uint32_t data[3] = {0};
  res = mifare_desfire_read_data(tag, 0, 0, sizeof(uint32_t), data);

  if (res < 4) {
    BOOST_LOG_TRIVIAL(warning) << "Did not read enough data!";
  } else if (res > 4) {
    BOOST_LOG_TRIVIAL(error) << "Did read too much data!";
  }

  if (res < 0) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to read data...";
    freefare_perror(tag, "mifare_desfire_read_data");

    free(bastli_aid);
    return 0;
  }

  BOOST_LOG_TRIVIAL(trace) << "Successfully read token from card: " << data[0];

  free(bastli_aid);

  BOOST_LOG_TRIVIAL(trace) << "Freed bastli_aid";

  return data[0];
}
