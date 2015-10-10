# Module to find libnfc

include(LibFindMacros)

libfind_pkg_detect(LIBRABBITMQ librabbitmq FIND_PATH amqp.h FIND_LIBRARY rabbitmq)
