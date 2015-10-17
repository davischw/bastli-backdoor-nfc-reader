/******************************************************************
 * NFC-Test Client                                               *
 *                                                               *
 * Version 0.0.1                                                 *
 *                                                               *
 *****************************************************************/

#include <iostream>

// Boost Logging
#include <boost/log/trivial.hpp>

// Boost program options
#include <boost/program_options.hpp>

#include <boost/format.hpp>

#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include <wiringPi.h>

namespace logging = boost::log;
namespace po = boost::program_options;

void setupGPIO();
void openDoor();

const int DOOR_PIN = 21; // BCM Pin number

int main(int argc, char *argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()("help", "show help message")(
      "host", po::value<std::string>(), "backdoor server host")(
      "port", po::value<int>()->default_value(5672), "backdoor server port")(
      "user", po::value<std::string>(),
      "rabbitmq user")("password", po::value<std::string>(),
                       "rabbitmq pw")("device_id", po::value<int>());

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  if (!(vm.count("host") && vm.count("port"))) {
    std::cout << "host and port are required!" << std::endl;
    std::cout << desc << std::endl;
    return 1;
  }

  if (!(vm.count("user") && vm.count("password"))) {
    std::cout << "User and password are required" << std::endl;
    std::cout << desc << std::endl;
    return 1;
  }

  if (!(vm.count("device_id"))) {
    std::cout << "Device id is required" << std::endl;
    return 1;
  }

  std::string host = vm["host"].as<std::string>();
  auto port = vm["port"].as<int>();
  std::string user = vm["user"].as<std::string>();
  std::string pw = vm["password"].as<std::string>();
  int device_id = vm["device_id"].as<int>();

  setupGPIO();

  std::string routing_key = (boost::format("lock.open.%d") % device_id).str();

  auto chann = AmqpClient::Channel::Create(host, port, user, pw, "/");

  // create anonymous queue
  auto queueName = chann->DeclareQueue("");
  chann->BindQueue(queueName, "backdoor", routing_key, AmqpClient::Table());

  BOOST_LOG_TRIVIAL(debug) << "Connected to broker, created queue";

  // start listening for messages
  auto tag = chann->BasicConsume(queueName, "opener_device");

  while (1) {
    chann->BasicConsumeMessage(tag);
    BOOST_LOG_TRIVIAL(info) << "Received open message, opening door";
    openDoor();
  }

  BOOST_LOG_TRIVIAL(info) << "Program finished, shutting down";

  return 0;
}

void setupGPIO() {
  BOOST_LOG_TRIVIAL(info) << "Setting pin " << DOOR_PIN << "as output";
  BOOST_LOG_TRIVIAL(debug) << (boost::format("gpio export %i out\n") % DOOR_PIN)
                                  .str();
  system((boost::format("gpio export %i out") % DOOR_PIN).str().c_str());

  if (wiringPiSetupSys() == -1) {
    BOOST_LOG_TRIVIAL(fatal) << "Configuration of wiringPi failed.";
    exit(1);
  }

  pinMode(DOOR_PIN, OUTPUT);
}

void openDoor() {
  BOOST_LOG_TRIVIAL(debug) << "Opening door...";
  digitalWrite(DOOR_PIN, HIGH);
  sleep(1);
  digitalWrite(DOOR_PIN, LOW);
  BOOST_LOG_TRIVIAL(debug) << "Opened";
}
