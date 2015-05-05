#ifndef NFC_CONTEXT_HPP
#define NFC_CONTEXT_HPP

#include <nfc/nfc.h>
#include <memory>
#include <vector>

#include "NfcException.hpp"

class NfcDevice;

class NfcContext : public std::enable_shared_from_this<NfcContext> {

public:
  NfcContext(const NfcContext& that) = delete;
  NfcContext& operator=(const NfcContext& that) = delete;

  static std::shared_ptr<NfcContext> init();

  ~NfcContext() {
    // Freeing nfc_context
    nfc_exit(_context);
  };

  std::vector<std::string> list_devices(size_t max_devices);
  std::shared_ptr<NfcDevice> open(std::string connstring);

private:
  NfcContext() {
    nfc_init(&_context);

    if (_context == nullptr) {
      throw NfcException("Failed to initialize NFC Context");
    }
  };

  nfc_context *_context = nullptr;
};

#endif // NFC_CONTEXT_HPP
