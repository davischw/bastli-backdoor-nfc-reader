#include "token.h"

Token::Token() {
  _data.fill('\0');
}

Token::Token(std::string data) {
  if (data.size() - 1  > _data.size()) {
    throw new std::runtime_error("wrong data size for token");
  }

  _data.fill('\0');

  std::copy(data.begin(), data.end(), _data.begin());
}

size_t Token::size() const {
  return _data.size();
}

std::string Token::to_string() const {
  std::string s;

  for (auto c : _data) {
    if (c == '\0') break;

    s.push_back(c);
  }

  return s;
}

Token::token_data_t::iterator Token::begin() {
  return _data.begin();
}

Token::token_data_t::iterator Token::end() {
  return _data.end();
}

char* Token::data() {
  return _data.data();
}

bool Token::operator ==(const Token &token) const{
  return (_data == token._data);
}

