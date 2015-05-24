#ifndef NFC_TOKEN_READER_HPP
#define NFC_TOKEN_READER_HPP

#include <vector>
#include <thread>

#include "NfcContext.hpp"
#include "NfcDevice.hpp"
#include "locked_queue.h"

class NfcTokenReader {

public:
  NfcTokenReader(LockedQueue<std::string>* queue);
  ~NfcTokenReader();

  void start();
  void stop();

private:
  std::thread _thread;
  bool _running = false;
  LockedQueue<std::string>* _queue;

  std::shared_ptr<NfcDevice> initialize_device();
  void run();

  std::vector<uint32_t> poll(std::shared_ptr<NfcDevice> device);
  uint32_t read_tag(MifareTag tag);
};

#endif // NFC_TOKEN_READER_HPP
