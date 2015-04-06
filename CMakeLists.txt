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
add_executable(nfc-reader src/nfc-reader.cpp)

include_directories(${jsoncpp_INCLUDE_DIR})
target_link_libraries(nfc-reader jsoncpp)

# Look for Boost libraries
find_package(Boost REQUIRED COMPONENTS system filesystem date_time thread log program_options)

if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(nfc-reader ${Boost_LIBRARIES})
endif (Boost_FOUND)

