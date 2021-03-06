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

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include <NfcTokenReader.hpp>

namespace logging = boost::log;
namespace po = boost::program_options;

using std::chrono::system_clock;

const std::string exchange_name = "backdoor";

int main(int argc, char *argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()("help", "show help message")(
      "host", po::value<std::string>(), "backdoor server host")(
      "port", po::value<int>()->default_value(5672), "backdoor server port")(
      "user", po::value<std::string>(),
      "rabbitmq user")("password", po::value<std::string>(), "rabbitmq pw")(
      "device_id", po::value<int>(), "device identification");

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

  if (!vm.count("device_id")) {
    std::cout << "Device id has to be set" << std::endl;
    return 1;
  }

  std::string host = vm["host"].as<std::string>();
  auto port = vm["port"].as<int>();
  std::string user = vm["user"].as<std::string>();
  std::string pw = vm["password"].as<std::string>();
  int device_id = vm["device_id"].as<int>();

  amqp_socket_t *socket = NULL;
  amqp_connection_state_t conn;

  conn = amqp_new_connection();
  socket = amqp_tcp_socket_new(conn);

  if (!socket) {
    BOOST_LOG_TRIVIAL(error) << "Failed to create socket";
    exit(1);
  }

  int status = amqp_socket_open(socket, host.c_str(), port);
  if (status) {
    BOOST_LOG_TRIVIAL(error) << "Failed to open socket";
    exit(1);
  }

  auto resp =
      amqp_login(conn, "/", AMQP_DEFAULT_MAX_CHANNELS, AMQP_DEFAULT_FRAME_SIZE,
                 0, AMQP_SASL_METHOD_PLAIN, user.c_str(), pw.c_str());

  if (resp.reply_type != AMQP_RESPONSE_NORMAL) {
    BOOST_LOG_TRIVIAL(error) << "Failed to login to broker";
    exit(1);
  }

  BOOST_LOG_TRIVIAL(info) << "Connected to broker, ready for AMQP!";

  amqp_channel_open(conn, 1);
  resp = amqp_get_rpc_reply(conn);
  if (resp.reply_type != AMQP_RESPONSE_NORMAL) {
    BOOST_LOG_TRIVIAL(error) << "Failed to open channel";
    exit(1);
  }

  /*
  amqp_exchange_declare(conn, 1, amqp_cstring_bytes(exchange_name.c_str()),
  amqp_cstring_bytes("topic"), 0, 0, 0, 0, amqp_empty_table);
  resp = amqp_get_rpc_reply(conn);
  if (resp.reply_type != AMQP_RESPONSE_NORMAL) {
        BOOST_LOG_TRIVIAL(error) << "Failed to declare exchange";
        exit(1);
  } */

  LockedQueue<Token> token_read;
  NfcTokenReader reader(&token_read);
  reader.start();

  auto routing_key = (boost::format("basics.access.%d") % device_id).str();

  bool running = true;

  while (running) {

    if (token_read.size() == 0) {
      // wait for condition variable
      std::unique_lock<std::mutex> mlock(token_read.lock);
      BOOST_LOG_TRIVIAL(trace) << "Waiting for token...";
      token_read.object_added.wait_for(mlock, std::chrono::seconds(5));
    }

    if (token_read.size() > 0) {
      BOOST_LOG_TRIVIAL(info) << "Read token!";
      auto token = token_read.pop();

      std::string cmd = (boost::format("{ \"cmd\": { \"token\": \"%s\"} }") %
                         token.to_string()).str();

      BOOST_LOG_TRIVIAL(debug) << "Routing: " << routing_key
                               << ", json: " << cmd;

      amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange_name.c_str()),
                         amqp_cstring_bytes(routing_key.c_str()), 0, 0, NULL,
                         amqp_cstring_bytes(cmd.c_str()));
      resp = amqp_get_rpc_reply(conn);
      if (resp.reply_type != AMQP_RESPONSE_NORMAL) {
        BOOST_LOG_TRIVIAL(error) << "Failed to publish message";
        exit(1);
      }
    }

    auto last_reader_update = system_clock::now() - reader.last_update;

    if (last_reader_update > std::chrono::seconds(5)) {
      BOOST_LOG_TRIVIAL(warning) << "Reader br0kn, shutting down..";
      running = false;
      std::exit(1);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  resp = amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
  resp = amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  status = amqp_destroy_connection(conn);

  BOOST_LOG_TRIVIAL(info) << "Program finished, shutting down";

  return 0;
}
