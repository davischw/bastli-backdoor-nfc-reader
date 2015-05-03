#include "NfcDevice.hpp"
#include "NfcTagList.hpp"

NfcTagList NfcDevice::list_tags() {
  MifareTag *tags = nullptr;
  tags = freefare_get_tags(_device);

  if (tags == nullptr) {
    throw NfcException("Failed to list tags");
  }

  return NfcTagList(tags, this->shared_from_this());
}
