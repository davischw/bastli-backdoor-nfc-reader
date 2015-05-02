#include <freefare.h>
// Boost Logging
#include <boost/log/trivial.hpp>

#include "NfcContext.hpp"
#include "MifareDesfireKey.hpp"

#include "NfcTokenReader.hpp"

uint8_t null_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t bastli_key[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

const uint32_t bastli_backdoor_aid = 0x5;
const uint8_t bastli_key_version = 1;

void personalize_card(MifareTag tag, MifareDesfireKey &new_key);
void list_applications(MifareTag tag);
void read_token(MifareTag tag);

int main() {

  NfcTokenReader reader;

  reader.start();

  std::this_thread::sleep_for(std::chrono::seconds(10));

  BOOST_LOG_TRIVIAL(trace) << "Trying to stop thread...";

  reader.stop();

  return 0;

  const size_t max_devices = 8;

  std::shared_ptr<NfcContext> context;

  try {
    // Setup nfc context
    context = NfcContext::init();

  } catch (NfcException &e) {
    BOOST_LOG_TRIVIAL(fatal) << "Error during NFC initialization: " << e.what();
    return 1;
  }

  // Look for NFC readers
  auto devices = context->list_devices(max_devices);

  if (devices.empty()) {
    BOOST_LOG_TRIVIAL(error) << "No NFC devices found";
    return 1;
  } else {
    BOOST_LOG_TRIVIAL(info) << "Found " << devices.size() << " devices";
    for (size_t i = 0; i < devices.size(); i++) {
      BOOST_LOG_TRIVIAL(debug) << "Device " << i + 1 << ": " << devices[i];
    }

    // Just connect to the first device
    auto device = context->open(devices[0]);

    // do stuff with the device
    MifareTag *tags = nullptr;
    tags = freefare_get_tags(device.getRawPointer());
    if (tags == nullptr) {
      BOOST_LOG_TRIVIAL(warning) << "Failed to listen tags";
    } else {
      for (int i = 0; tags[i]; i++) {
        if (DESFIRE == freefare_get_tag_type(tags[i])) {
          // We found a desfire, check if it is personalized to Bastli
          BOOST_LOG_TRIVIAL(info)
              << "Found tag: " << freefare_get_tag_friendly_name(tags[i]);

          // Connect
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
            // for some reason old software doesn't work...
            BOOST_LOG_TRIVIAL(info) << "Software is too old, not using card";
            break;
          }

          uint8_t version = 0;
          res = mifare_desfire_get_key_version(tags[i], 0, &version);
          if (res < 0) {
            BOOST_LOG_TRIVIAL(warning) << "Failed to get key version";
          } else {
            // for some reason Boost Log does not print a zero interger
            // BOOST_LOG_TRIVIAL(info) << "Master key version: " << version;
            printf("Master key version: %d\n", version);
          }

          // Try to list all applications
          list_applications(tags[i]);

          auto new_key = MifareDesfireKey::create_aes_key_with_version(
              bastli_key, bastli_key_version);
          // WARNING: setting the key version with the function below does not
          // work!
          // mifare_desfire_key_set_version(new_key, bastli_key_version);

          if (version == 0) {
            // key version 0 is used for default key
            // try to personalize card

            personalize_card(tags[i], new_key);
          } else {
            if (version == bastli_key_version) {

              res = mifare_desfire_authenticate(tags[i], 0, new_key.get_raw());
              if (res < 0) {
                freefare_perror(tags[i], "mifare_desfire_authenticate");
                BOOST_LOG_TRIVIAL(warning) << "Card is neither using default "
                                              "key nor bastli key, ignoring "
                                              "card";
              } else {
                BOOST_LOG_TRIVIAL(info) << "Successful authentication, card is "
                                           "personalized with Bastli key";

                read_token(tags[i]);

                // Test code...

                BOOST_LOG_TRIVIAL(info)
                    << "Trying to delete Bastli application";
                MifareDESFireAID bastli_aid =
                    mifare_desfire_aid_new(bastli_backdoor_aid);

                if (bastli_aid == nullptr) {
                  BOOST_LOG_TRIVIAL(error) << "Failed to create Bastli AID";
                  freefare_perror(tags[i], "mifare_desfire_aid_new");
                } else {
                  res = mifare_desfire_delete_application(tags[i], bastli_aid);
                  if (res < 0) {
                    freefare_perror(tags[i],
                                    "mifare_desfire_delete_application");
                  }
                  free(bastli_aid);
                }

                res =
                    mifare_desfire_authenticate(tags[i], 0, new_key.get_raw());
                if (res < 0) {
                  freefare_perror(tags[i], "mifare_desfire_authenticate");
                  BOOST_LOG_TRIVIAL(error) << "Failed to authenticate with "
                                              "bastli key, unable to reset key";
                } else {

                  auto default_key = MifareDesfireKey::create_des_key(null_key);
                  // authenticate with default_key

                  BOOST_LOG_TRIVIAL(info)
                      << "Trying to reset card to default key";
                  res = mifare_desfire_change_key(
                      tags[i], 0, default_key.get_raw(), new_key.get_raw());

                  if (res < 0) {
                    freefare_perror(tags[i], "mifare_desfire_change_key");
                  } else {
                    BOOST_LOG_TRIVIAL(info)
                        << "Successfully reset card to default key";
                  }
                }
                // End test code...
              }
            }
          }

          res = mifare_desfire_disconnect(tags[i]);
          if (res < 0) {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to free tag";
            return 1;
          }
        }
      }

      freefare_free_tags(tags);
    }
  }

  BOOST_LOG_TRIVIAL(info) << "Bastli NFC-Reader shutting down, goodbye";

  return 0;
};

void personalize_card(MifareTag tag, MifareDesfireKey &new_key) {
  auto default_key = MifareDesfireKey::create_des_key(null_key);
  int res = mifare_desfire_authenticate(tag, 0, default_key.get_raw());

  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_authenticate");
    return;
  }

  BOOST_LOG_TRIVIAL(info)
      << "Successful authentication, card is using the default key";
  BOOST_LOG_TRIVIAL(info) << "Setting TOP SECRET bastli key...";

  res = mifare_desfire_change_key(tag, 0, new_key.get_raw(),
                                  default_key.get_raw());

  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_change_key");
    return;
  }

  BOOST_LOG_TRIVIAL(info) << "Verifying that key has been changed...";
  uint8_t version = 0;

  res = mifare_desfire_get_key_version(tag, 0, &version);
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_get_key_version");
    return;
  }

  if (version != bastli_key_version) {
    BOOST_LOG_TRIVIAL(error) << "Key verification failed";
    return;
  }

  BOOST_LOG_TRIVIAL(info) << "Key verified";

  BOOST_LOG_TRIVIAL(trace) << "Authenticating with new key...";
  res = mifare_desfire_authenticate(tag, 0, new_key.get_raw());
  if (res < 0) {
    BOOST_LOG_TRIVIAL(error) << "Failed to authenticate with new key";
    freefare_perror(tag, "mifare_desfire_set_default_key");
  }

  res = mifare_desfire_set_default_key(tag, default_key.get_raw());
  if (res < 0) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to set default key!";
    freefare_perror(tag, "mifare_desfire_set_default_key");
    return;
  }

  BOOST_LOG_TRIVIAL(info) << "Creating application";
  MifareDESFireAID bastli_aid = mifare_desfire_aid_new(bastli_backdoor_aid);

  if (bastli_aid == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "Failed to create Bastli AID";
    freefare_perror(tag, "mifare_desfire_aid_new");
    return;
  }

  // create new application with the default key
  //
  // key settings:
  // aaaa b c d e
  //            ^------ allow change master key
  //          ^-------- free directory list without master key
  //        ^---------- free create /delete without master key
  //      ^------------ configuration changeable
  // ^^^^-------------- ChangeKey access rights:
  //                     -       0x0: Application master key is needed to change
  //                     any key
  //                     - 0x1 - 0xD: Authentication with the specified key is
  //                     necessary to change any key
  //                     -       0xE: Authentication with the key to be changed
  //                     (same KeyNo)
  //                                  is necessary to change any key
  //                     -       0xF: All keys (except application master key,
  //                     see bit e)
  //                                  within this application are frozen
  res = mifare_desfire_create_application(tag, bastli_aid, 0b00001001, 1);
  if (res < 0) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to create application!";
    freefare_perror(tag, "mifare_desfire_create_application");
    free(bastli_aid);
    return;
  }

  // list_applications(tag);

  res = mifare_desfire_select_application(tag, bastli_aid);
  free(bastli_aid);
  if (res < 0) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to select Bastli application";
    freefare_perror(tag, "mifare_desfire_select_application");
    return;
  }

  // authenticate with default key
  res = mifare_desfire_authenticate(tag, 0, default_key.get_raw());
  if (res < 0) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to authenticate (application)";
    freefare_perror(tag, "mifare_desfire_authenticate");
    return;
  }

  // create a file
  //
  // communication settings: 11 -> 3 for fully enciphered
  // access rights:           0 -> key 0 (application key) for everything..
  // size:                   32 bytes
  res = mifare_desfire_create_std_data_file(tag, 0, 3, 0, 32);
  if (res < 0) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to create data file";
    freefare_perror(tag, "mifare_desfire_create_std_data_file");
    return;
  }

  uint32_t token = 0xdeadbeef;

  res = mifare_desfire_write_data(tag, 0, 0, sizeof(token), &token);
  if (res != sizeof(token)) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to write all data!";
    return;
  }

  BOOST_LOG_TRIVIAL(info) << "Data file created";

  // write data into the file

  BOOST_LOG_TRIVIAL(info) << "Card successfully personalized";

  return;
}

void read_token(MifareTag tag) {

  auto default_key = MifareDesfireKey::create_des_key(null_key);


  MifareDESFireAID bastli_aid = mifare_desfire_aid_new(bastli_backdoor_aid);
  if (bastli_aid == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "Failed to create Bastli AID";
    freefare_perror(tag, "mifare_desfire_aid_new");
    return;
  }

  int res = mifare_desfire_select_application(tag, bastli_aid);
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_select_application");

    free(bastli_aid);
    return;
  }

  // authenticate with default_key
  res = mifare_desfire_authenticate(tag, 0, default_key.get_raw());
  if (res < 0) {
    freefare_perror(tag, "mifare_desfire_authenticate");

    free(bastli_aid);
    return;
  }

  uint32_t data[3] = { 0 };
  BOOST_LOG_TRIVIAL(trace) << "sizeof data: " << sizeof(data);
  res = mifare_desfire_read_data(tag, 0, 0, 4, data);
  if (res < 0) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to read data...";
    freefare_perror(tag, "mifare_desfire_read_data");

    free(bastli_aid);
    return;
  }

  for (int i = 0; i < 3; i++) {
    printf("%d\n", data[i]);
  }

  BOOST_LOG_TRIVIAL(info) << "Successfully read token from card: " << data[0];

  free(bastli_aid);
}

void list_applications(MifareTag tag) {
  size_t app_count;
  MifareDESFireAID *aids = nullptr;

  int res = mifare_desfire_get_application_ids(tag, &aids, &app_count);
  if (res < 0) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to get application ids";
    freefare_perror(tag, "mifare_desfire_get_application_ids");
  } else {
    BOOST_LOG_TRIVIAL(info) << "Found " << app_count << " applications: ";
    for (size_t j = 0; j < app_count; j++) {
      BOOST_LOG_TRIVIAL(info)
          << "Application id: " << mifare_desfire_aid_get_aid(aids[j]);

      // select the application
      res = mifare_desfire_select_application(tag, aids[j]);
      if (res < 0) {
        BOOST_LOG_TRIVIAL(trace) << "Failed to select application";
        continue;
      }

      // list all files
      size_t num_files;
      uint8_t *files = nullptr;
      res = mifare_desfire_get_file_ids(tag, &files, &num_files);
      if (res < 0) {
        BOOST_LOG_TRIVIAL(trace) << "Failed to get list of file ids..";
        freefare_perror(tag, "mifare_desfire_get_file_ids");
      } else {
        BOOST_LOG_TRIVIAL(trace) << "Application has " << num_files << " files";

        for (size_t i = 0; i < num_files; i++) {
          BOOST_LOG_TRIVIAL(trace) << "File with id " << files[i];
        }
      }

      if (files != nullptr) {
        free(files);
      }
    }
  }

  // select default / root application
  res = mifare_desfire_select_application(tag, nullptr);
  if (res < 0) {
    BOOST_LOG_TRIVIAL(warning) << "Failed to select default application!";
  }

  mifare_desfire_free_application_ids(aids);
}
