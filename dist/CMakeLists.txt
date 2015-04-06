# Define compilation of json library
add_library(jsoncpp STATIC jsoncpp.cpp)
set(jsoncpp_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/json/ PARENT_SCOPE)
