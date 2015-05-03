#ifndef NFC_DEVICE_HPP
#define NFC_DEVICE_HPP

#include <memory>

#include "freefare.h"
#include "NfcContext.hpp"

class NfcTagList;

class NfcDevice : public std::enable_shared_from_this<NfcDevice> {

public:
  ~NfcDevice() {
    // Freeing device
    nfc_close(_device);
  }

  NfcTagList list_tags();

private:
  std::shared_ptr<NfcContext> _context;
  nfc_device *_device = nullptr;

  NfcDevice(std::shared_ptr<NfcContext> context, nfc_device *device)
      : _context(context) {
    _device = device;

    if (_device == nullptr) {
      throw NfcException("Failed to open NFC device");
    }
  }

  // allow NfcContext to create a NfcDevice
  friend std::shared_ptr<NfcDevice> NfcContext::open(std::string connstring);
};

#endif
