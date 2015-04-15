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

std::unique_ptr<NfcDevice> NfcContext::open(std::string connection_string) {
    //Max length for a connection string is NFC_BUFSIZE_CONNSTRING
    if (connection_string.length() > NFC_BUFSIZE_CONNSTRING) {
      throw std::runtime_error("connection_string is too long, maximum length is given by NFC_BUFSIZE_CONNSTRING");
    }

    nfc_device *device = nullptr;

    device = nfc_open(_context, connection_string.c_str());

    return std::unique_ptr<NfcDevice>(new NfcDevice(std::shared_ptr<NfcContext>(this), device));
}

nfc_device *NfcDevice::getRawPointer() {
    return _device;
}
