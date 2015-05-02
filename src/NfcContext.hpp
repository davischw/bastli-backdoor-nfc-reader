#ifndef NFC_CONTEXT_HPP
#define NFC_CONTEXT_HPP

#include <nfc/nfc.h>
#include <memory>
#include <vector>

#include <boost/log/trivial.hpp>

#include "NfcException.hpp"

class NfcDevice;

class NfcContext : public std::enable_shared_from_this<NfcContext> {

public:
  static std::shared_ptr<NfcContext> init();

  ~NfcContext() {
    // Freeing nfc_context
    nfc_exit(_context);
  };

  std::vector<std::string> list_devices(size_t max_devices);
  std::unique_ptr<NfcDevice> open(std::string connstring);

private:
  NfcContext() {
    nfc_init(&_context);

    if (_context == nullptr) {
      throw NfcException("Failed to initialize NFC Context");
    }
  };

  nfc_context *_context = nullptr;
};

class NfcDevice {

public:
  ~NfcDevice() {
    // Freeing device
    nfc_close(_device);
  }

  nfc_device *getRawPointer();

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

  friend std::unique_ptr<NfcDevice> NfcContext::open(std::string connstring);
};
#endif // NFC_CONTEXT_HPP
