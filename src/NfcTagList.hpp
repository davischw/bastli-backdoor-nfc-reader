#ifndef NFC_TAG_LIST_HPP
#define NFC_TAG_LIST_HPP

#include <freefare.h>
#include "NfcContext.hpp"
#include "NfcDevice.hpp"

// Class to wrap a list of NFC tags
class NfcTagList {
public:
  ~NfcTagList() {
    if (_tags != nullptr) {
      freefare_free_tags(_tags);
    }
  }

  MifareTag *get_raw() const { return _tags; }

private:
  NfcTagList(MifareTag *tags, std::shared_ptr<NfcDevice> device)
      : _tags(tags), _device(device) {}

  MifareTag *_tags;
  std::shared_ptr<NfcDevice> _device;

  // allow creation by NfcDevice
  friend NfcTagList NfcDevice::list_tags();
};

#endif // NFC_TAG_LIST_HPP
