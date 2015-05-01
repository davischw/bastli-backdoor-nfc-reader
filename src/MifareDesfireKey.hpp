#ifndef MIFARE_DESFIRE_KEY_HPP
#define MIFARE_DESFIRE_KEY_HPP

#include <freefare.h>

// Wrapper class for MifareDesfireKey from
// libfreefare
class MifareDesfireKey {
  public:
    MifareDesfireKey(uint8_t value[8]) {
      //setup the key...
      _key = mifare_desfire_des_key_new(value);

      if (_key == nullptr) {
        throw std::runtime_error("Failed to create DES key");
      }
    }

    ~MifareDesfireKey() {
      mifare_desfire_key_free(_key);
    }

    MifareDESFireKey get_raw() { return _key; }


  private:
    MifareDESFireKey _key = nullptr;
};

#endif
