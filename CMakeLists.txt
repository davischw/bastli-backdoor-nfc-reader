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

# Use dynamically linked boost on Linux
if(UNIX)
  add_definitions(-DBOOST_ALL_DYN_LINK)
endif()


add_library(bd-token src/token.cpp)
add_library(nfc-cpp-wrapper src/NfcTokenReader.cpp src/NfcDevice.cpp src/NfcContext.cpp)
add_library(bd-commands src/command.cpp)

target_link_libraries(bd-commands bd-token)
target_link_libraries(nfc-cpp-wrapper bd-commands)


# Create executable for nfc personalisation
if (WIN32)
  add_executable(nfc-personalize src/nfc_personalize.cpp)
  target_link_libraries(nfc-personalize nfc-cpp-wrapper)
endif()



# Create server test clients
add_executable(nfc-test-client src/nfc_test_client.cpp)
add_executable(nfc-test-producer src/nfc_test_producer.cpp)



# only build opener on linux
if(UNIX)
  add_executable(nfc-opener src/nfc_opener.cpp)
  target_link_libraries(nfc-opener bd-commands)
endif()

add_executable(nfc-reader-amqp src/nfc_reader_amqp.cpp)
target_link_libraries(nfc-reader-amqp nfc-cpp-wrapper)

add_executable(nfc-reset src/nfc_reset.cpp)
target_link_libraries(nfc-reset nfc-cpp-wrapper)

# Setup installation
if (UNIX)
install(TARGETS nfc-reader-amqp nfc-opener
        RUNTIME DESTINATION bin)

install(FILES config/backdoor-reader.service config/backdoor-opener.service
        DESTINATION /usr/lib/systemd/system)

install(FILES config/backdoor.conf DESTINATION /etc)
endif()

include_directories(${jsoncpp_INCLUDE_DIR})
target_link_libraries(bd-commands jsoncpp)


# On Windows, add a BOOST_ROOT variable, which can be used to enter the
# location of Boot
if(WIN32)
  set(BOOST_ROOT_DIR "${BOOST_ROOT}" CACHE PATH "Boost root directory")
  set(BOOST_ROOT "${BOOST_ROOT_DIR}")
  set(Boost_USE_STATIC_LIBS ON)

  # We need to include winsocks..
  target_link_libraries(nfc-test-client wsock32 ws2_32)
  target_link_libraries(nfc-reader-amqp wsock32 ws2_32)
endif()

# Look for Boost libraries
# make sure to require log before thread, otherwise linking will fail
find_package(Boost REQUIRED COMPONENTS log thread system filesystem date_time program_options regex)

if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(nfc-test-client ${Boost_LIBRARIES})

  if (WIN32)
    target_link_libraries(nfc-personalize ${Boost_LIBRARIES})
  endif()

  if (UNIX)
    #target_link_libraries(nfc-reader ${Boost_LIBRARIES})
    target_link_libraries(nfc-opener ${Boost_LIBRARIES} )
  endif()

  target_link_libraries(nfc-reader-amqp ${Boost_LIBRARIES})
  target_link_libraries(nfc-reset ${Boost_LIBRARIES})
  target_link_libraries(nfc-test-producer ${Boost_LIBRARIES} )
endif (Boost_FOUND)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/modules)

# For CMake Versions > 3.0.0, the pthread library has to be added as a dependency
# Reason for this is currently unknown
if (UNIX)
  if (CMAKE_VERSION GREATER 3)
    set(CMAKE_THREAD_PREFER_PTHREAD 1)
    set(THREADS_PREFER_PTHREAD_FLAG 1)

    find_package(Threads REQUIRED)
    if (CMAKE_USE_PTHREADS_INIT)
      target_link_libraries(nfc-test-client Threads::Threads)
      #target_link_libraries(nfc-reader Threads::Threads)
      target_link_libraries(nfc-reset Threads::Threads)
    target_link_libraries(nfc-test-producer Threads::Threads)
    target_link_libraries(nfc-reader-amqp Threads::Threads)
    target_link_libraries(nfc-opener Threads::Threads)
    else()
  	  message(FATAL_ERROR "PThread not found, exiting")
    endif()
  endif()
endif()

if (UNIX)
  # Look for wiringPi
  find_package(wiringPi REQUIRED)

include_directories(${WIRINGPI_INCLUDE_DIR})

target_link_libraries(nfc-opener ${WIRINGPI_LIBRARY})
endif()

# Look for libnfc
find_package(libnfc REQUIRED)

include_directories(${LIBNFC_INCLUDE_DIR})
target_link_libraries(nfc-cpp-wrapper ${LIBNFC_LIBRARY})


# Look for libfreefare
find_package(libfreefare REQUIRED)

include_directories(${LIBFREEFARE_INCLUDE_DIR})
target_link_libraries(nfc-cpp-wrapper ${LIBFREEFARE_LIBRARY})

find_package(librabbitmq REQUIRED)
include_directories(${LIBRABBITMQ_INCLUDE_DIR})
target_link_libraries(nfc-test-client ${LIBRABBITMQ_LIBRARY})
target_link_libraries(nfc-test-producer ${LIBRABBITMQ_LIBRARY} )
target_link_libraries(nfc-reader-amqp ${LIBRABBITMQ_LIBRARY})

include_directories(src/)

find_package(simpleamqpclient REQUIRED)
include_directories(${SIMPLEAMQPCLIENT_INCLUDE_DIR})

if (UNIX)
  target_link_libraries(nfc-opener ${SIMPLEAMQPCLIENT_LIBRARY})
endif()

if (WIN32)
  target_link_libraries(nfc-personalize ${SIMPLEAMQPCLIENT_LIBRARY})
endif()
