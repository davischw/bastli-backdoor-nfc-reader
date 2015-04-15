#include<freefare.h>

//Boost Logging
#include<boost/log/trivial.hpp>

#include<NfcContext.hpp>

int main() {

  const size_t max_devices = 8;

  //Setup nfc context
  NfcContext context;

  //Look for NFC readers
  
  auto devices = context.list_devices(max_devices);

  if (devices->empty()) {
    BOOST_LOG_TRIVIAL(error) << "No NFC devices found";
    return 1;
  } else {
    BOOST_LOG_TRIVIAL(info) << "Found " << devices->size() << " devices";
  }

  return 0;
};
