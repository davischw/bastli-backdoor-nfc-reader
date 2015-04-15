#include<freefare.h>
#include<iostream> 
//Boost Logging
#include<boost/log/trivial.hpp>


uint8_t null_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t new_key[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

int main() {

  const size_t max_devices = 8;
  nfc_connstring devices[max_devices];

  //Setup nfc context
  nfc_context *context = nullptr;
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
    for (size_t i = 0; i < device_count; i++) {
      BOOST_LOG_TRIVIAL(debug) << "Device " << i + 1 << ": " << devices[i];
    }

    nfc_device *device = nullptr;
    //Just connect to the first device
    device = nfc_open(context, devices[0]);

    if (device == nullptr) {
      BOOST_LOG_TRIVIAL(fatal) << "Failed to open device " << devices[0];

      return 1;
    }

    // do stuff with the device
    MifareTag *tags = nullptr;
    tags = freefare_get_tags(device);
    if (tags == nullptr) {
      BOOST_LOG_TRIVIAL(warning) << "Failed to listen tags";
    } else {
      for (int i = 0; tags[i]; i++) {
        if (DESFIRE == freefare_get_tag_type(tags[i])) {
          //We found a desfire, check if it is personalized to Bastli
          BOOST_LOG_TRIVIAL(info) << "Found tag: " << freefare_get_tag_friendly_name(tags[i]);

          //Connect
          int res;
          res = mifare_desfire_connect(tags[i]);
          if (res < 0) {
            BOOST_LOG_TRIVIAL(warning) << "Failed to connect to Mifare DESFire";
            continue;
	  }

          struct mifare_desfire_version_info info;
          res = mifare_desfire_get_version(tags[i], &info);
          if (res < 0) {
            freefare_perror(tags[i], "mifare_desfire_get_version");
            break;
          }

          if (info.software.version_major < 1) {
             //for some reason old software doesn't work...
             BOOST_LOG_TRIVIAL(info) << "Software is too old, not using card";
             break;
          }


          uint8_t version = 0;
          res = mifare_desfire_get_key_version(tags[i], 0, &version);
          if (res < 0) {
            BOOST_LOG_TRIVIAL(warning) << "Failed to get key version";
          } else {
             // for some reason Boost Log does not print an zero interger
             //BOOST_LOG_TRIVIAL(info) << "Master key version: " << version;
             printf("Master key version: %d\n", version);
          }

	  //Try to authenticate with the default key
          MifareDESFireKey default_key = mifare_desfire_des_key_new_with_version(null_key);
          res = mifare_desfire_authenticate(tags[i], 0, default_key);
          if (res < 0) {
            freefare_perror(tags[i], "mifare_desfire_authenticate");
          } else {
            BOOST_LOG_TRIVIAL(info) << "Successful authentication, card is using the default key";
          }

          mifare_desfire_key_free(default_key);

          res = mifare_desfire_disconnect(tags[i]);
          if (res < 0) {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to free tag";
            return 1;
          }
        }
      }

      freefare_free_tags(tags);
    }

    nfc_close(device);
  }

  nfc_exit(context);

  return 0;
}
