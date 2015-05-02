#ifndef NFC_EXCEPTION_HPP
#define NFC_EXCEPTION_HPP

#include <stdexcept>

class NfcException : public std::runtime_error {
public:
  NfcException(const std::string &what) : std::runtime_error(what){};
  NfcException(const char *what) : std::runtime_error(what){};
};

#endif // NFC_EXCEPTION_HPP
