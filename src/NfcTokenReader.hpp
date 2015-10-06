#ifndef NFC_TOKEN_READER_HPP
#define NFC_TOKEN_READER_HPP

#include <vector>
#include <thread>
#include <boost/optional.hpp>

#include "NfcContext.hpp"
#include "NfcDevice.hpp"
#include "locked_queue.h"
#include "token.h"

class NfcTokenReader {

public:
  NfcTokenReader(LockedQueue<Token>* queue);
  ~NfcTokenReader();

  void start();
  void stop();


private:
  std::thread _thread;
  bool _running = false;

  // min sleep time between polling
  int _sleep_ms = 200;

  LockedQueue<Token>* _queue;

  std::shared_ptr<NfcDevice> initialize_device();
  void run();

  std::vector<Token> poll(std::shared_ptr<NfcDevice> device);
  boost::optional<Token> read_tag(MifareTag tag);
};

#endif // NFC_TOKEN_READER_HPP
