/* grakopp.hpp - Grako++ main header file
   Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
   Written by Marcus Brinkmann <m.brinkmann@semantics.de>

   This file is part of Grako++.  Grako++ is free software; you can
   redistribute it and/or modify it under the terms of the 2-clause
   BSD license, see file LICENSE.TXT.
*/

#ifndef _GRAKOPP_BUFFER_HPP
#define _GRAKOPP_BUFFER_HPP 1

#include <features.h>
#include <string>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cerrno>

/* Use std::regex, except where it is not available (then import
   boost::regex into the std namespace).  */

#if (! defined(__GNUC__)) || __GNUC_PREREQ (4,9) || __clang__
#include <regex>
#else
#include <boost/regex.hpp>
namespace std
{
  using boost::regex;
  namespace regex_constants
  {
    using boost::regex_constants::match_flag_type;
    using boost::regex_constants::match_continuous;
    using boost::regex_constants::match_prev_avail;
  }
  using boost::smatch;
  using boost::regex_search;
}
#endif


/* FIXME: Maybe use unicode instead char.  The following is not
   unicode safe (for example, skip_to used with a non-7bit value).  */

typedef char CHAR_T;
#define CHAR_NULL ((CHAR_T) 0) /* Sort of.  */


class Buffer
{
public:

  Buffer()
    : _pos(0)
  {
  }

  Buffer(const std::string &text)
    : _text(text), _pos(0)
  {
  }

  void from_file(const char *filename)
  {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
      {
	std::ostringstream contents;
	contents << in.rdbuf();
	in.close();
	_text = contents.str();
	return;
      }
    throw(errno);
  }

  std::string _text;
  size_t _pos;

  size_t len() const
  {
    return _text.length();
  }

  bool atend() const
  {
    return _pos >= len();
  }

  bool ateol() const
  {
    return atend()
      || _text[_pos] == '\r'
      || _text[_pos] == '\n';
  }

  CHAR_T current() const
  {
    if (atend())
      return CHAR_NULL;
    else
      return _text[_pos];
  }

  CHAR_T at(size_t pos) const
  {
    if (pos >= len())
      return CHAR_NULL;
    return _text[pos];
  }

  CHAR_T peek(size_t off) const
  {
    return at(_pos + off);
  }

  CHAR_T next()
  {
    if (atend())
      return CHAR_NULL;
    return _text[_pos++];
  }

  void go_to(size_t pos)
  {
    size_t length = len();

    if (pos < 0)
      _pos = 0;
    else if (pos > length)
      _pos = length;
    else
      _pos = pos;
  }

  void move(size_t off)
  {
    go_to(_pos + off);
  }

  void next_token()
  {
    size_t pos;
    do
      {
	pos = _pos;
	/* FIXME: eatwhitespace, eatcomments.  */
      }
    while (pos != _pos);
  }

  size_t skip_to(CHAR_T ch)
  {
    size_t pos = _pos;
    size_t length = len();
    while (pos < length && _text[pos] != ch)
      ++pos;
    go_to(pos);
    return pos;
  }

  size_t skip_past(CHAR_T ch)
  {
    skip_to(ch);
    next();
    return _pos;
  }

  size_t skip_to_eol()
  {
    return skip_to('\n');
  }

  bool is_space()
  {
    return false;
    /* return self.current() in self.whitespace */
  }

  bool is_name_char()
  {
    CHAR_T ch = this->current();
    return ch != CHAR_NULL && std::isalpha(ch);
  }

  bool match(std::string token)
  {
    int len = token.length();
    bool eq = (_text.compare(_pos, len, token) == 0);
    if (eq)
      {
	move(len);
	return true;
      }
    return false;
  }

  boost::optional<std::string> matchre(std::string pattern)
  {
    boost::optional<std::string> maybe_token;
    std::regex re(pattern);

    /* Multiline is the default.  */
    std::regex_constants::match_flag_type flags = std::regex_constants::match_continuous;
    if (_pos > 0)
      flags |= std::regex_constants::match_prev_avail;

    std::smatch match;
    int cnt = std::regex_search(_text.cbegin() + _pos, _text.cend(), match, re, flags);
    if (cnt > 0)
      {
	maybe_token = match[0];
	_pos += match[0].length();
      }

    return maybe_token;
  }

};

#endif /* GRAKOPP_BUFFER_H */
