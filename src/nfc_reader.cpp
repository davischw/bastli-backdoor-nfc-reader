#include <iostream>
#include <fstream>

// Boost Logging
#include <boost/log/core.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// Boost Program Options
#include <boost/program_options.hpp>

#include "signal.h"

#include "json.h"
#include "NfcTokenReader.hpp"
#include "opener.h"
#include "token.h"
#include "bd_client.hpp"


namespace po = boost::program_options;



int main(int argc, char** argv) {
  po::options_description generic("Generic options");
  generic.add_options()(
      "help,h", "show help message")(
      "debug,d", "show debug output")(
      "config,c", po::value<std::string>()->default_value("/etc/backdoor.conf"), "use config file")(
      "use_display", "use external display");

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

  std::string config_filename = vm["config"].as<std::string>();

  std::ifstream config_file(config_filename, std::ifstream::in);
  
  po::store(po::parse_config_file(config_file, config_options), vm);
  po::notify(vm);

  config_file.close();

  if (vm.count("help")) {
    std::cout << cmd << std::endl;
    return 1;
  }

  if (!(vm.count("host") && vm.count("port") && vm.count("server_token") && vm.count("reader_token"))) {
    std::cout << cmd << std::endl;
    return 1;
  }

  boost::log::add_console_log(std::cout, boost::log::keywords::auto_flush = true );

 if (!vm.count("debug")) {
   boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
 }

  std::cout << "Ohai, starting reader" << std::endl;
  std::cout.flush();


  BOOST_LOG_TRIVIAL(info) << "Starting NFC-Reader";

  ConfigStruct config;
  config.use_logger = vm.count("use_display");
  
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


  // Setup signal handling
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);

  //wait for signal
  int signal;
  int res = sigwait(&mask, &signal);
  
  if (res > 0) {
    //error in signal handler
    BOOST_LOG_TRIVIAL(fatal) << "Failure in signal handling";
    return 1;
  }

  switch (signal) {
  case SIGCHLD:
    //One of the threads stopped...
    BOOST_LOG_TRIVIAL(fatal) << "Thread shut down, trying to stop other threads and shutting down";
    break;
  case SIGINT:
  case SIGTERM:
    BOOST_LOG_TRIVIAL(info) << "Received shutdown signal";
  }

  BOOST_LOG_TRIVIAL(trace) << "Trying to stop thread...";

  reader.stop();
  o.stop();
  client.stop();

  BOOST_LOG_TRIVIAL(info) << "NFC-Reader exiting normally";

  return 0;
}
