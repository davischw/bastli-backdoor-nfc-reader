# Module to find libnfc

include(LibFindMacros)

libfind_pkg_detect(LIBNFC libnfc-dev FIND_PATH nfc/nfc.h FIND_LIBRARY nfc)
