#ifndef MIFARE_DESFIRE_KEY_HPP
#define MIFARE_DESFIRE_KEY_HPP

#include <freefare.h>

#include <boost/log/trivial.hpp>

// Wrapper class for MifareDesfireKey from
// libfreefare
class MifareDesfireKey {

public:
  MifareDesfireKey(MifareDesfireKey &&other) {
    _key = other._key;

    other._key = nullptr;
  }

  MifareDesfireKey &operator=(MifareDesfireKey &&other) {

    if (this != &other) {
      mifare_desfire_key_free(_key);

      _key = other._key;
      other._key = nullptr;
    }

    return *this;
  }

  ~MifareDesfireKey() {
    // Check if the key has been moved away
    if (_key != nullptr) {
      mifare_desfire_key_free(_key);
    }
  }

  MifareDESFireKey get_raw() const { return _key; }

private:
  MifareDESFireKey _key;

  MifareDesfireKey(MifareDESFireKey k) : _key(k) {
    BOOST_LOG_TRIVIAL(trace) << "Creating key...";

    if (k == nullptr) {
      throw std::runtime_error("Failed to create key");
    }
  }

public:
  static MifareDesfireKey create_des_key(uint8_t value[8]) {
    return MifareDesfireKey(mifare_desfire_des_key_new(value));
  };

  static MifareDesfireKey create_des_key_with_version(uint8_t value[8]) {
    return MifareDesfireKey(mifare_desfire_des_key_new_with_version(value));
  };

  static MifareDesfireKey create_3des_key(uint8_t value[16]) {
    return MifareDesfireKey(mifare_desfire_3des_key_new(value));
  };

  static MifareDesfireKey create_3des_key_with_version(uint8_t value[16]) {
    return MifareDesfireKey(mifare_desfire_3des_key_new_with_version(value));
  };

  static MifareDesfireKey create_3k3des_key(uint8_t value[24]) {
    return MifareDesfireKey(mifare_desfire_3k3des_key_new_with_version(value));
  };
  static MifareDesfireKey create_3k3des_key_with_version(uint8_t value[24]) {
    return MifareDesfireKey(mifare_desfire_3k3des_key_new_with_version(value));
  };

  static MifareDesfireKey create_aes_key(uint8_t value[16]) {
    return MifareDesfireKey(mifare_desfire_aes_key_new(value));
  };
  static MifareDesfireKey create_aes_key_with_version(uint8_t value[16],
                                                      uint8_t version) {
    return MifareDesfireKey(
        mifare_desfire_aes_key_new_with_version(value, version));
  };
};

#endif
