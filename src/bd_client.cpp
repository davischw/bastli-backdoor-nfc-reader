#include "bd_client.hpp"
#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>
#include "command.h"

BdClient::~BdClient() {
  if(thread_.joinable())
	thread_.join();
}

void BdClient::start() {

  // Resolve the hostname
  tcp::resolver resolver(io);

  BOOST_LOG_TRIVIAL(info) << "Trying to connect to: " << hostname_ << ":" << port_;

  tcp::resolver::query query(hostname_, port_);

  resolver.async_resolve(query,
                         boost::bind(&BdClient::start_client, this,
                                     boost::asio::placeholders::error,
                                     boost::asio::placeholders::iterator));

  // Setup signal handling
  //signals.async_wait(boost::bind(&BdClient::handle_signals, this,
                                 //boost::asio::placeholders::error,
                                 //boost::asio::placeholders::signal_number));
  thread_ = std::thread([this](){ io.run(); });
}

void BdClient::send(Json::Value cmd) {
  //Add stuff to the io_service, hope this works with multiple sets...
  async_write(socket_, boost::asio::buffer(json_writer_.write(cmd)),
              boost::bind(&BdClient::handle_write, this, _1, _2));
  
}

void BdClient::start_client(const boost::system::error_code &ec,
                     tcp::resolver::iterator endpoint_iter) {
  if (ec) {
    BOOST_LOG_TRIVIAL(error) << "Failed to resolve address of backdoor server";
    throw boost::system::system_error(ec);
  }

  stopped_ = false;

  start_connect(endpoint_iter);
}

void BdClient::stop() {
  // Stop boost io service (threadsafe)
  io.stop();
}

void BdClient::stop_client() {
  if (stopped_) {
    BOOST_LOG_TRIVIAL(debug)
        << "Trying to stop already stopped client, ignoring";
    return;
  }

  if (registered_) {
    // we have to deregister from the server first
    start_deregister();
  } else {
    // we can just close the socket
    close_socket();
  }
}

void BdClient::start_connect(tcp::resolver::iterator endpoint_iter) {
  // Start deadline timer for connection

  BOOST_LOG_TRIVIAL(info) << "Connecting to remote server...";
  async_connect(socket_, endpoint_iter,
                boost::bind(&BdClient::handle_connect, this, _1, _2));
}

void BdClient::handle_connect(const boost::system::error_code &ec,
                              tcp::resolver::iterator endpoint_iter) {
  BOOST_LOG_TRIVIAL(trace) << "BdClient::handle_connect";

  if (stopped_)
    return;

  if (ec == boost::asio::error::not_found) {
    // Unable to connect to server, stopping client
    BOOST_LOG_TRIVIAL(error) << "Failed to connect to server";
    stop_client();
    return;
  }

  if (ec) {
    // Some random error happened..
    throw boost::system::system_error(ec);
  }

  BOOST_LOG_TRIVIAL(info) << "Connection established";

  start_register();
}

void BdClient::start_register() {
  BOOST_LOG_TRIVIAL(trace) << "BdClient::start_register";

  if (stopped_)
    return;

  // Send the client register command

  BOOST_LOG_TRIVIAL(info) << "Sending register command";

  async_write(socket_, boost::asio::buffer(json_writer_.write(Command::registr(auth_token_))),
              boost::bind(&BdClient::handle_register, this));
}

void BdClient::handle_register() {
  registered_ = true;

  BOOST_LOG_TRIVIAL(debug) << "Register command sucessfuly sent";

  if (stopped_) {
    return;
  }

  // Now are registered with the server, do useful stuff...

  // Wait for commands from server
  start_read();
}

void BdClient::start_read() {
  BOOST_LOG_TRIVIAL(debug) << "Starting to listen for server commands";

  async_read_until(socket_, input_buffer_, '\n',
                   boost::bind(&BdClient::handle_read, this, _1, _2));
  BOOST_LOG_TRIVIAL(trace) << "start_read() finished";
}

void BdClient::handle_read(const boost::system::error_code &ec,
                           std::size_t bytes_transferred) {
  BOOST_LOG_TRIVIAL(trace) << "BdClient::handle_read";

  if (stopped_)
    return;

  if (ec == boost::asio::error::eof) {
    // Connection closed by remote host
    BOOST_LOG_TRIVIAL(info) << "Connection closed by remote host";

    // since we don't have a connection anymore, just close the socket
    // immediatly
    close_socket();
    return;
  }

  if (ec) {
    throw boost::system::system_error(ec);
  }

  BOOST_LOG_TRIVIAL(debug) << "Received message from server";

  std::istream is(&input_buffer_);

  Json::Value received_json;

  if (json_reader_.parse(is, received_json, false)) {
    // We received a valid command!
    BOOST_LOG_TRIVIAL(debug) << "Received a valid JSON command";
    BOOST_LOG_TRIVIAL(debug) << received_json;

    if (received_json["cmd"]["method"] == "PING") {

      BOOST_LOG_TRIVIAL(debug) << "Received PING";


      BOOST_LOG_TRIVIAL(debug) << "Sending PONG";

      async_write(socket_, boost::asio::buffer(json_writer_.write(Command::pong(auth_token_))),
                  boost::bind(&BdClient::handle_write, this, _1, _2));
    } else {
      BOOST_LOG_TRIVIAL(trace) << "Forwarding command to opener";
      reader_in->push(received_json);
      BOOST_LOG_TRIVIAL(trace) << "reader_in.size(): " << reader_in->size();
    }
  } else {
    // Log wrong command
    BOOST_LOG_TRIVIAL(warning) << "Unable to parse received JSON";
  }

  start_read();
}

void BdClient::handle_write(const boost::system::error_code &ec,
                            std::size_t bytes_transferred) {
  BOOST_LOG_TRIVIAL(trace) << "BdClient::handle_write";

  if (ec) {
    throw boost::system::system_error(ec);
  }
}

void BdClient::start_deregister() {
  BOOST_LOG_TRIVIAL(trace) << "BdClient::start_deregister";

  BOOST_LOG_TRIVIAL(info) << "Unregistering from server";

  async_write(socket_, boost::asio::buffer(json_writer_.write(Command::unregister(auth_token_))),
              boost::bind(&BdClient::handle_deregister, this,
                          boost::asio::placeholders::error,
                          boost::asio::placeholders::bytes_transferred));
}

void BdClient::handle_deregister(const boost::system::error_code &ec,
                                 std::size_t bytes_transferred) {
  BOOST_LOG_TRIVIAL(trace) << "BdClient::handle_deregister";

  if (ec) {
    BOOST_LOG_TRIVIAL(error) << "Failed to deregister!";
    throw boost::system::system_error(ec);
  }

  close_socket();
}

void BdClient::close_socket() {
  BOOST_LOG_TRIVIAL(trace) << "BdClient::close_socket";

  stopped_ = true;

  boost::system::error_code ec;

  // we can now shutdown, as we have succesfully closed the socket
  socket_.shutdown(tcp::socket::shutdown_both, ec);

  if (ec) {
    BOOST_LOG_TRIVIAL(error) << "Failed to shutdown properly";
    throw boost::system::system_error(ec);
  }

  socket_.close();
}
