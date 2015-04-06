// TCP Client for the backdoor reader

#include <boost/asio.hpp>
#include "json.h"

using boost::asio::ip::tcp;
using boost::asio::deadline_timer;

class BdClient {
  public:
    BdClient(std::string auth_token)
      : auth_token_(auth_token),
        socket_(io),
        signals(io, SIGINT, SIGTERM)
    {
    }

    void run(std::string host, std::string port);

  private:

    // Start connection with remote server
    void start(tcp::resolver::iterator endpoint_iter);

    // Stop the client
    void stop();

    void start_connect(tcp::resolver::iterator endpoint_iter);
    void handle_connect(const boost::system::error_code& ec,
        tcp::resolver::iterator endpoint_iter);

    void close_socket();


    // Signal handling
    void handle_signals(const boost::system::error_code& error, int signal_number);


    // Register client with server
    void start_register();
    void handle_register();

    void start_deregister();
    void handle_deregister(const boost::system::error_code& ec, std::size_t bytes_transferred);

    // Listen to commands from server
    void start_read();
    void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred);


    // Send commands to server
    void handle_write(const boost::system::error_code& ec, std::size_t bytes_transferred);


    //Backdoor server state
    std::string auth_token_;
    bool registered_ = false;

    bool stopped_ = false;


    //Connection variables
    boost::asio::io_service io;
    tcp::socket socket_;
    boost::asio::streambuf input_buffer_;

    boost::asio::signal_set signals;

    //JSON interface
    Json::Reader      json_reader_;
    Json::FastWriter  json_writer_;
};
