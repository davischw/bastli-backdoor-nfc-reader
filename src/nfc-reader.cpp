/*****************************************************************
 * NFC-Reader                                                    *
 *                                                               *
 * Version 0.0.1                                                 *
 *                                                               *
 *****************************************************************/

#include<iostream>

//Boost format (sprintf for C++)
#include<boost/format.hpp>

//Boost Asio (Sockets)
#include<boost/asio.hpp>
#include<boost/array.hpp>

//Boost Logging
#include<boost/log/core.hpp>
#include<boost/log/trivial.hpp>
#include<boost/log/expressions.hpp>

//Boost program options
#include<boost/program_options.hpp>

//JSON library
#include "json.h"

namespace logging = boost::log;
namespace po = boost::program_options;

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "show help message")
    ("host", po::value<std::string>(), "backdoor server host")
    ("port", po::value<std::string>(), "backdoor server port")
    ("token", po::value<std::string>(), "authentication token")
  ;

  Json::Value register_cmd;

  register_cmd["auth"]["token"] = "alfonso";
  register_cmd["auth"]["time"] = "2015-04-05T17:59:00+02:00";

  register_cmd["cmd"]["method"] = "REGISTER";
  register_cmd["cmd"]["params"] = Json::arrayValue;

  Json::FastWriter  json_writer;
  Json::Reader      json_reader;


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


  if (!vm.count("token")) {
    std::cout << "Authentication token is required!" << std::endl;
    std::cout << desc << std::endl;
    return 1;
  }

  std::string host = vm["host"].as<std::string>();
  std::string port = vm["port"].as<std::string>();
  std::string token = vm["token"].as<std::string>();

  try {
    BOOST_LOG_TRIVIAL(info) << "Bastli NFC-Reader v0.0.1 starting";

    boost::asio::io_service io;




    // Resolve the hostname
    tcp::resolver resolver(io);

    BOOST_LOG_TRIVIAL(info) << "Trying to connect to: " << host << ":" << port; 

    tcp::resolver::query query(host, port);
    auto endpoint_iterator = resolver.resolve(query);


    // Connect to remote server
    tcp::socket socket(io);
    boost::asio::connect(socket, endpoint_iterator);

    BOOST_LOG_TRIVIAL(info) << "Connection successful";



    // Registering the client
    boost::system::error_code error;
    BOOST_LOG_TRIVIAL(debug) << "Sending register command:";

    std::string out_cmd = json_writer.write(register_cmd);

    BOOST_LOG_TRIVIAL(debug) << out_cmd;

    write(socket, boost::asio::buffer(out_cmd), error);
    BOOST_LOG_TRIVIAL(debug) << "Register command sent";


    if (error) {
      throw boost::system::system_error(error);
    }


    for (;;) {
      boost::asio::streambuf buf;

      read_until(socket, buf, '\n', error);

      if (error == boost::asio::error::eof) {
        BOOST_LOG_TRIVIAL(info) << "Connection closed by remote host";
        break; //Connection closed cleanly
      } else if (error)
        throw boost::system::system_error(error);

      std::istream is(&buf);

      Json::Value received_json;

      if (json_reader.parse(is, received_json, false)) {
        BOOST_LOG_TRIVIAL(debug) << "Command received:" << received_json;


        if (received_json["cmd"]["method"] == "PING") {
          BOOST_LOG_TRIVIAL(debug) << "Received PING";

          Json::Value pong;

          pong["auth"]["token"] = token;
          pong["auth"]["time"] = "2015-04-05T17:59:00+02:00";

          pong["cmd"]["method"] = "PONG";
          pong["cmd"]["params"] = Json::arrayValue;

          BOOST_LOG_TRIVIAL(debug) << "Sending PONG";

          boost::asio::write(socket, boost::asio::buffer(json_writer.write(pong)), error);

          if (error) {
            throw boost::system::system_error(error);
          }
        }
      } else {
        BOOST_LOG_TRIVIAL(debug) << "Failed to parse received JSON";
      }


    }

    //TODO: Send deregister command to server
    BOOST_LOG_TRIVIAL(info) << "Sending deregister command to server";
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
  }


  BOOST_LOG_TRIVIAL(info) << "Program finished, shutting down";

  return 0;
}
