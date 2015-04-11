#include<freefare.h>

//Boost Logging
#include<boost/log/trivial.hpp>

int main() {

  const size_t max_devices = 8;
  nfc_connstring devices[max_devices];

  //Setup nfc context
  nfc_context *context;
  nfc_init(&context);

  if (context == NULL) {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to initalize NFC context";
    return 1;
  }

  //Look for NFC readers
  size_t device_count = nfc_list_devices(context, devices, max_devices);

  if (device_count <= 0) {
    BOOST_LOG_TRIVIAL(error) << "No NFC devices found";
    return 1;
  } else {
    BOOST_LOG_TRIVIAL(info) << "Found " << device_count << " devices";
  }

  nfc_exit(context);

  return 0;
}
