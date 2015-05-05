#ifndef NFC_DEVICE_HPP
#define NFC_DEVICE_HPP

#include <boost/log/trivial.hpp>

#include <memory>

#include "freefare.h"
#include "NfcContext.hpp"

class NfcTagList;

class NfcDevice : public std::enable_shared_from_this<NfcDevice> {

public:
  NfcDevice(NfcDevice &&other) {
    _device = other._device;
    _context = other._context;

    other._device = nullptr;
  }

  NfcDevice &operator=(NfcDevice &&other) {
    if (this != &other) {
      nfc_close(_device);

      _device = other._device;
      _context = other._context;
      other._device = nullptr;
    }

    return *this;
  }

  ~NfcDevice() {
    if (_device != nullptr) {
	    // Freeing device
	    BOOST_LOG_TRIVIAL(trace) << "Closing NFC device";
	    nfc_close(_device);
    }
  }

  NfcTagList list_tags();

private:
  std::shared_ptr<NfcContext> _context;
  nfc_device *_device = nullptr;

  NfcDevice(std::shared_ptr<NfcContext> context, nfc_device *device)
      : _context(context) {
    BOOST_LOG_TRIVIAL(trace) << "Creating NFC device";
    _device = device;

    if (_device == nullptr) {
      throw NfcException("Failed to open NFC device");
    }
  }

  // allow NfcContext to create a NfcDevice
  friend std::shared_ptr<NfcDevice> NfcContext::open(std::string connstring);
};

#endif
