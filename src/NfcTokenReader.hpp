#ifndef NFC_TOKEN_READER_HPP
#define NFC_TOKEN_READER_HPP

#include <vector>
#include <thread>

#include "freefare.h"

#include "NfcContext.hpp"

class NfcTokenReader {

public:
  NfcTokenReader(){};

  ~NfcTokenReader() {
    if (_thread.joinable()) {
      _thread.join();
    }
  }

  void start();
  void stop();

private:
  std::thread _thread;
  bool _running = false;

  NfcDevice initialize_device();
  void run();

  std::vector<uint32_t> poll(NfcDevice &device);
  uint32_t read_tag(MifareTag tag);
};

#endif // NFC_TOKEN_READER_HPP
