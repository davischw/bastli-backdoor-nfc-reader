# Specify minimum CMake version
cmake_minimum_required(VERSION 2.8)

# Project name
project(nfc-reader)

# Export compile commands for use with autocompletion
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Use fancy new C++11
add_definitions(-std=c++11)

set(CMAKE_CXX_FLAGS "-Wall")

# Use bin/ for binaries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add dist folder (json library)
add_subdirectory(dist/)

# Use dynamically linked boost
add_definitions(-DBOOST_ALL_DYN_LINK)

# Create main executable
add_executable(nfc-reader src/nfc-reader.cpp src/bd_client.cpp)

# Create test executable for nfc personalisation
add_executable(nfc-personalize src/nfc_personalize.cpp src/NfcContext.cpp)

include_directories(${jsoncpp_INCLUDE_DIR})
target_link_libraries(nfc-reader jsoncpp)

# Look for Boost libraries
find_package(Boost REQUIRED COMPONENTS system filesystem date_time thread log program_options regex)

if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(nfc-reader ${Boost_LIBRARIES})
  target_link_libraries(nfc-personalize ${Boost_LIBRARIES})
endif (Boost_FOUND)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/modules)

# Use PThreads
set(CMAKE_THREAD_PREFER_PTHREAD 1)
set(THREADS_PREFER_PTHREAD_FLAG 1)
find_package(Threads REQUIRED)
if (CMAKE_USE_PTHREADS_INIT)
  #target_link_libraries(nfc-reader Threads::Threads)
  #	target_link_libraries(nfc-personalize Threads::Threads)
else()
	message(FATAL_ERROR "PThread not found, exiting")
endif()

# Look for libnfc
find_package(libnfc REQUIRED)

include_directories(${LIBNFC_INCLUDE_DIR})
target_link_libraries(nfc-personalize ${LIBNFC_LIBRARY})

# Look for libfreefare
find_package(libfreefare REQUIRED)

include_directories(${LIBFREEFARE_INCLUDE_DIR})
target_link_libraries(nfc-personalize ${LIBFREEFARE_LIBRARY})

include_directories(src/)
