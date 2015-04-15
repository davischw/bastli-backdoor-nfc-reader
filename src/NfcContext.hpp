#ifndef NFC_CONTEXT_HPP
#define NFC_CONTEXT_HPP

#include<nfc/nfc.h>
#include<memory>
#include<vector>
#include<stdexcept>

#include<boost/log/trivial.hpp>

class NfcDevice;

class NfcContext {
  public:

    NfcContext() {
      nfc_init(&_context);

      if (_context == nullptr) {
        throw std::runtime_error("Failed to initialize NFC Context");
      }
    };

    ~NfcContext() {
      //Freeing nfc_context
      nfc_exit(_context);
    };


    std::unique_ptr< std::vector<std::string> > list_devices(size_t max_devices);
    std::unique_ptr<NfcDevice> open(std::string connstring);

  private:
    nfc_context* _context = nullptr;
};

class NfcDevice {

public:
    NfcDevice(std::shared_ptr<NfcContext> context, nfc_device* device)
            : _context(context)
    {
        if (device == nullptr) {
            throw std::runtime_error("device pointer not set to an object");
        }

        _device = device;
    }

    ~NfcDevice() {
        //Freeing device
        nfc_close(_device);
    }

    nfc_device* getRawPointer();

private:
    std::shared_ptr<NfcContext> _context;
    nfc_device* _device;
};
#endif //NFC_CONTEXT_HPP
