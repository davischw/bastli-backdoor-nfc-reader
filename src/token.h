#ifndef TOKEN_H
#define TOKEN_H

#include <array>

class Token {


public:
  Token();
  Token(std::string data);

  std::string to_string() const;
  bool operator ==(const Token &token) const;

  size_t size() const;

  typedef std::array<char, 16> token_data_t;

  token_data_t::iterator begin();
  token_data_t::iterator end();
  char* data();


private:
  token_data_t _data;
};

#endif
