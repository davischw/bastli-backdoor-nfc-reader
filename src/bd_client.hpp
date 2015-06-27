// TCP Client for the backdoor reader
#ifndef BD_CLIENT_H
#define BD_CLIENT_H

#include <boost/asio.hpp>
#include <thread>

#include "json.h"
#include "config.h"
#include "locked_queue.h"

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;

class BdClient {
public:
  BdClient(ConfigStruct config, LockedQueue<Json::Value>* reader_in, LockedQueue<Json::Value>* reader_out)
      : auth_token_(config.client_token),
        server_token_(config.server_token),
        hostname_(config.hostname),
        port_(config.port),
        socket_(io), 
        resolver_(io),
        signals(io, SIGINT, SIGTERM),
        reader_out(reader_out),
        reader_in(reader_in) {}
  
  ~BdClient();

  void start();
  void stop();

  void send(Json::Value cmd);


private:
  // Start connection with remote server
  void start_client(const boost::system::error_code &ec,
             tcp::resolver::iterator endpoint_iter);
  void stop_client();

  void start_resolve();

  void start_connect(tcp::resolver::iterator endpoint_iter);
  void handle_connect(const boost::system::error_code &ec,
                      tcp::resolver::iterator endpoint_iter);

  void close_socket();

  // Register client with server
  void start_register();
  void handle_register();

  void start_deregister();
  void handle_deregister(const boost::system::error_code &ec,
                         std::size_t bytes_transferred);

  // Listen to commands from server
  void start_read();
  void handle_read(const boost::system::error_code &ec,
                   std::size_t bytes_transferred);

  // Send commands to server
  void handle_write(const boost::system::error_code &ec,
                    std::size_t bytes_transferred);

  // Backdoor server state
  Token auth_token_;
  Token server_token_;
  std::string hostname_;
  std::string port_;
  bool registered_ = false;

  bool stopped_ = true;

  // Connection variables
  boost::asio::io_service io;
  tcp::socket socket_;
  tcp::resolver resolver_;
  boost::asio::streambuf input_buffer_;

  boost::asio::signal_set signals;

  // JSON interface
  Json::Reader json_reader_;
  Json::FastWriter json_writer_;

  // Thread
  std::thread thread_;
  LockedQueue<Json::Value>* reader_out;
  LockedQueue<Json::Value>* reader_in;
};
#endif //BD_CLIENT_H
