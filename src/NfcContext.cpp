#include <NfcContext.hpp>


std::unique_ptr<std::vector<std::string>> NfcContext::list_devices(size_t max_devices) {
  nfc_connstring *devices = new nfc_connstring[max_devices];

  size_t found_devices = nfc_list_devices(_context, devices, max_devices);

  std::unique_ptr<std::vector<std::string>> result(new std::vector<std::string>(found_devices));

  for (size_t i = 0; i < found_devices; i++) {
    (*result)[i] = std::string(devices[i]);
  }

  delete[] devices;

  return result;
}
