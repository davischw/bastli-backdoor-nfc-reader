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

#include <amqp.h>
#include <amqp_tcp_socket.h>

namespace logging = boost::log;
namespace po = boost::program_options;

const amqp_channel_t CHAN = 1;

int main(int argc, char *argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()("help", "show help message")(
      "host", po::value<std::string>(), "backdoor server host")(
      "port", po::value<int>()->default_value(5672), "backdoor server port")(
      "user", po::value<std::string>(),
      "rabbitmq user")("password", po::value<std::string>(), "rabbitmq pw");

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

  std::string host = vm["host"].as<std::string>();
  auto port = vm["port"].as<int>();
  std::string user = vm["user"].as<std::string>();
  std::string pw = vm["password"].as<std::string>();

  amqp_socket_t *socket = NULL;
  amqp_connection_state_t conn;

  conn = amqp_new_connection();
  socket = amqp_tcp_socket_new(conn);

  if (!socket) {
    BOOST_LOG_TRIVIAL(error) << "Failed to create socket";
    exit(1);
  }

  struct timeval to;
  to.tv_sec = 2;
  to.tv_usec = 0;

  BOOST_LOG_TRIVIAL(trace) << "Opening socket...";
  int status = amqp_socket_open_noblock(socket, host.c_str(), port, &to);
  if (status) {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to open socket: " << amqp_error_string2(status);
    exit(1);
  }

  BOOST_LOG_TRIVIAL(trace) << "Socket opened, logging in...";

  auto resp =
      amqp_login(conn, "/", AMQP_DEFAULT_MAX_CHANNELS, AMQP_DEFAULT_FRAME_SIZE,
                 0, AMQP_SASL_METHOD_PLAIN, user.c_str(), pw.c_str());

  if (resp.reply_type != AMQP_RESPONSE_NORMAL) {
    BOOST_LOG_TRIVIAL(error) << "Failed to login to broker";
    if (resp.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION) {
      BOOST_LOG_TRIVIAL(error)
          << "Library error: " << amqp_error_string2(resp.library_error);
    } else {
      BOOST_LOG_TRIVIAL(error) << "Server error, method id: " << resp.reply.id;
    }
    exit(1);
  }

  BOOST_LOG_TRIVIAL(info) << "Connected to broker, ready for AMQP!";

  amqp_channel_open(conn, CHAN);
  resp = amqp_get_rpc_reply(conn);
  if (resp.reply_type != AMQP_RESPONSE_NORMAL) {
    BOOST_LOG_TRIVIAL(error) << "Failed to open channel";
    exit(1);
  }

  amqp_exchange_declare(conn, CHAN, amqp_cstring_bytes("backdoor"),
                        amqp_cstring_bytes("topic"), 0, 0,
                        amqp_empty_table);
  resp = amqp_get_rpc_reply(conn);
  if (resp.reply_type != AMQP_RESPONSE_NORMAL) {
    BOOST_LOG_TRIVIAL(error) << "Failed to declare exchange";
    exit(1);
  }

  // declare new queue, with auto generated name, empty argument table, and
  // auto_delete
  amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, CHAN, amqp_empty_bytes,
                                                  0, 0, 0, 1, amqp_empty_table);
  resp = amqp_get_rpc_reply(conn);
  if (resp.reply_type != AMQP_RESPONSE_NORMAL) {
    BOOST_LOG_TRIVIAL(error) << "Failed to declare queue";
    exit(1);
  }

  amqp_bytes_t queuename;
  queuename = amqp_bytes_malloc_dup(r->queue);
  if (queuename.bytes == NULL) {
    BOOST_LOG_TRIVIAL(error) << "Out of memory while copying queue name";
    exit(1);
  }

  amqp_queue_bind(conn, CHAN, queuename, amqp_cstring_bytes("backdoor"),
                  amqp_cstring_bytes("*.*.*"), amqp_empty_table);
  resp = amqp_get_rpc_reply(conn);
  if (resp.reply_type != AMQP_RESPONSE_NORMAL) {
    BOOST_LOG_TRIVIAL(error) << "Failed to declare queue";
    exit(1);
  }

  // consume from queue
  amqp_basic_consume(conn, CHAN, queuename, amqp_cstring_bytes("test_client"),
                     0, 1, 1, amqp_empty_table);
  resp = amqp_get_rpc_reply(conn);
  if (resp.reply_type != AMQP_RESPONSE_NORMAL) {
    BOOST_LOG_TRIVIAL(error) << "Failed to start consuming";
    exit(1);
  }

  amqp_rpc_reply_t ret;
  amqp_envelope_t envelope;
  while (1) {
    ret = amqp_consume_message(conn, &envelope, NULL, 0);
    switch (ret.reply_type) {
    case AMQP_RESPONSE_NORMAL:
      BOOST_LOG_TRIVIAL(info) << "Received normal AMQP message";
      break;
    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
      BOOST_LOG_TRIVIAL(error) << "Unspecified error while consuming message";
      exit(1);
      break;
    default:
      BOOST_LOG_TRIVIAL(fatal)
          << "Unknown response while consuming message, shutting down";
      exit(1);
    }
  }

  resp = amqp_channel_close(conn, CHAN, AMQP_REPLY_SUCCESS);
  resp = amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  status = amqp_destroy_connection(conn);

  BOOST_LOG_TRIVIAL(info) << "Program finished, shutting down";

  return 0;
}
