# Module to find SimpleAmqpClient (rabbitmq c++ libarary)

include(LibFindMacros)

libfind_pkg_detect(SIMPLEAMQPCLIENT simpleamqpclient FIND_PATH SimpleAmqpClient/SimpleAmqpClient.h FIND_LIBRARY SimpleAmqpClient)
