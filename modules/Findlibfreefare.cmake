# Module to find libfreefare

include(LibFindMacros)

libfind_package(LIBFREEFARE libnfc)

libfind_pkg_detect(LIBFREEFARE libfreefare-dev FIND_PATH freefare.h FIND_LIBRARY freefare)
