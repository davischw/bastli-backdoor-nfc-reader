/*****************************************************************
 * NFC-Reader                                                    *
 *                                                               *
 * Version 0.0.1                                                 *
 *                                                               *
 *****************************************************************/

#include<iostream>

//Boost format (sprintf for C++)
#include<boost/format.hpp>

#include<boost/bind.hpp>

//Boost Asio (Sockets)
#include<boost/asio.hpp>

//Boost Logging
#include<boost/log/trivial.hpp>

//Boost program options
#include<boost/program_options.hpp>

//JSON library
#include "json.h"


#include "bd_client.hpp"

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

  BdClient client(token);

  try {
    BOOST_LOG_TRIVIAL(info) << "Bastli NFC-Reader v0.0.1 starting";

    client.run(host, port);




    // Start asynchronous processing

  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
  }


  BOOST_LOG_TRIVIAL(info) << "Program finished, shutting down";

  return 0;
}
