// Boost Logging
#include <boost/log/trivial.hpp>

// Boost Program Options
#include <boost/program_options.hpp>

#include "json.h"
#include "NfcTokenReader.hpp"
#include "opener.h"
#include "token.h"
#include "bd_client.hpp"

namespace po = boost::program_options;

int main(int argc, char** argv) {
  po::options_description generic("Generic options");
  generic.add_options()(
      "help", "show help message");

  po::options_description config_options("Configuration");
  config_options.add_options()(
      "host", po::value<std::string>(), "server host")(
      "port", po::value<std::string>(), "server port")(
      "server_token", po::value<std::string>(), "server authentication token")(
      "reader_token", po::value<std::string>(), "reader authentication token");

  po::options_description cmd("Command line options");
  cmd.add(generic).add(config_options);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, cmd), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << cmd << std::endl;
    return 1;
  }

  if (!(vm.count("host") && vm.count("port") && vm.count("server_token") && vm.count("reader_token"))) {
    std::cout << cmd << std::endl;
    return 1;
  }

  BOOST_LOG_TRIVIAL(info) << "Starting NFC-Reader";

  ConfigStruct config;
  config.use_logger = false;
  config.cache_token_timeout = 600;

  config.hostname = vm["host"].as<std::string>();
  config.port = vm["port"].as<std::string>();
  config.server_token = Token(vm["server_token"].as<std::string>());
  config.client_token = Token(vm["reader_token"].as<std::string>());

  LockedQueue<Token> token_read;
  LockedQueue<Json::Value> reader_in;
  LockedQueue<Json::Value> reader_out;

  BdClient client(config, &reader_in, &reader_out);
  Opener o(config, &token_read, &reader_in, client);
  NfcTokenReader reader(&token_read);

  o.start();
  reader.start();
  client.start();

  std::this_thread::sleep_for(std::chrono::seconds(60));

  BOOST_LOG_TRIVIAL(trace) << "Trying to stop thread...";

  reader.stop();
  o.stop();
  client.stop();

  BOOST_LOG_TRIVIAL(info) << "NFC-Reader exiting normally";

  return 0;
}
