# Specify minimum CMake version
cmake_minimum_required(VERSION 2.8)

# Project name
project(nfc-reader)

# Export compile commands for use with autocompletion
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Use fancy new C++11
add_definitions(-std=c++11)

set(CMAKE_CXX_FLAGS "-Wall -Werror")

# Use bin/ for binaries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add dist folder (json library)
add_subdirectory(dist/)

# Use dynamically linked boost
add_definitions(-DBOOST_ALL_DYN_LINK)

# Create server test client
add_executable(nfc-test-client src/nfc_test_client.cpp src/bd_client.cpp src/token.cpp src/command.cpp)

# Create test executable for nfc personalisation
add_executable(nfc-personalize src/nfc_personalize.cpp src/NfcContext.cpp src/NfcDevice.cpp src/token.cpp)

add_executable(nfc-reader src/nfc_reader.cpp src/NfcTokenReader.cpp src/NfcContext.cpp src/NfcDevice.cpp src/opener.cpp src/token.cpp src/command.cpp src/bd_client.cpp)

add_executable(nfc-reset src/nfc_reset.cpp src/NfcContext.cpp)


# Setup installation
install(TARGETS nfc-reader
        RUNTIME DESTINATION bin)

install(FILES config/backdoor.service
        DESTINATION /usr/lib/systemd/system)

install(FILES config/backdoor.conf DESTINATION /etc)

include_directories(${jsoncpp_INCLUDE_DIR})
target_link_libraries(nfc-test-client jsoncpp)
target_link_libraries(nfc-reader jsoncpp)


option(USE_GPERF "link gperf (Google performance measurement tools" ON)

if (USE_GPERF)
  set (CMAKE_SHARED_LINKER_FLAGS"-lprofiler")
endif (USE_GPERF)


# Look for Boost libraries
find_package(Boost REQUIRED COMPONENTS system filesystem date_time thread log program_options regex)

if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(nfc-test-client ${Boost_LIBRARIES})
  target_link_libraries(nfc-personalize ${Boost_LIBRARIES})
  target_link_libraries(nfc-reader ${Boost_LIBRARIES})
  target_link_libraries(nfc-reset ${Boost_LIBRARIES})
endif (Boost_FOUND)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/modules)

# For CMake Versions > 3.0.0, the pthread library has to be added as a dependency
# Reason for this is currently unknown

if (CMAKE_VERSION GREATER 3)
  set(CMAKE_THREAD_PREFER_PTHREAD 1)
  set(THREADS_PREFER_PTHREAD_FLAG 1)

  find_package(Threads REQUIRED)
  if (CMAKE_USE_PTHREADS_INIT)
    target_link_libraries(nfc-test-client Threads::Threads)
    target_link_libraries(nfc-personalize Threads::Threads)
    target_link_libraries(nfc-reader Threads::Threads)
    target_link_libraries(nfc-reset Threads::Threads)
  else()
	  message(FATAL_ERROR "PThread not found, exiting")
  endif()
endif()

# Look for wiringPi
find_package(wiringPi REQUIRED)

include_directories(${WIRINGPI_INCLUDE_DIR})
target_link_libraries(nfc-reader ${WIRINGPI_LIBRARY})

# Look for libnfc
find_package(libnfc REQUIRED)

include_directories(${LIBNFC_INCLUDE_DIR})
target_link_libraries(nfc-personalize ${LIBNFC_LIBRARY})
target_link_libraries(nfc-reader ${LIBNFC_LIBRARY})
target_link_libraries(nfc-reset ${LIBNFC_LIBRARY})

# Look for libfreefare
find_package(libfreefare REQUIRED)

include_directories(${LIBFREEFARE_INCLUDE_DIR})
target_link_libraries(nfc-personalize ${LIBFREEFARE_LIBRARY})
target_link_libraries(nfc-reader ${LIBFREEFARE_LIBRARY})
target_link_libraries(nfc-reset ${LIBFREEFARE_LIBRARY})

include_directories(src/)
