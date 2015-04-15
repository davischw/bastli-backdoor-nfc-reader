#ifndef NFC_CONTEXT_HPP
#define NFC_CONTEXT_HPP

#include<nfc/nfc.h>
#include<memory>
#include<vector>
#include<stdexcept>

class NfcContext {
  public:

    NfcContext() {
      nfc_init(&_context);

      if (_context == nullptr) {
        throw std::runtime_error("Failed to initialize NFC Context");
      }
    };

    ~NfcContext() {
      nfc_exit(_context);
    };


    std::unique_ptr< std::vector<std::string> > list_devices(size_t max_devices);

  private:
    nfc_context* _context = nullptr;
};
#endif
