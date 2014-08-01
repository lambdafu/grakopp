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
#include <unordered_map>

#include <boost/optional.hpp>

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
    using boost::regex_constants::match_not_dot_newline;
  }
  using boost::smatch;
  using boost::regex_search;
}
#define BOOST_REGEX
#endif


/* FIXME: Maybe use unicode instead char.  The following is not
   unicode safe (for example, skip_to used with a non-7bit value).  */

typedef char CHAR_T;
#define CHAR_NULL ((CHAR_T) 0) /* Sort of.  */


class Buffer;
using BufferPtr = std::shared_ptr<Buffer>;

class Buffer
{
public:
  std::string _text;
  size_t _pos;
  std::string _whitespace;
  bool _nameguard;

  Buffer()
    : _pos(0), _whitespace(),
      _nameguard(false)
  {
  }

  void from_string(const std::string& text)
  {
    _text = text;
  }

  void from_file(const std::string& filename)
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
	/* FIXME: eatcomments.  */
	if (_whitespace.length() > 0)
	  {
	    size_t new_pos = _text.find_first_not_of(_whitespace, _pos);
	    if (new_pos != std::string::npos)
	      _pos = new_pos;
	  }
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

  bool is_name_char(size_t pos)
  {
    CHAR_T ch = this->at(pos);
    return ch != CHAR_NULL && std::isalpha(ch);
  }

  bool match(const std::string& token)
  {
    int len = token.length();

    if (len == 0)
      return true;

    bool eq = (_text.compare(_pos, len, token) == 0);
    if (!eq)
      return false;

    if (_nameguard)
      {
	bool token_first_is_alpha = is_name_char(_pos);
	bool follow_is_alpha = is_name_char(_pos + len);

	if (token_first_is_alpha && follow_is_alpha)
	  {
	    /* Check if the token is alphanumeric.  */
	    auto begin = _text.cbegin() + _pos;
	    auto end = begin + len;

	    bool token_is_alnum = find_if(begin, end, 
					  [](char ch) { return !std::isalnum(ch); }) == end;
	    if (token_is_alnum)
	      return false;
	  }
      }

    move(len);
    return true;
  }

  boost::optional<std::string> matchre(std::string pattern)
  {
    boost::optional<std::string> maybe_token;

    /* FIXME: Not thread-safe.  */
    static std::unordered_map<std::string, std::regex *>_lookup;
    std::regex *rep;
    if (_lookup.count(pattern) == 0)
      {
	rep = new std::regex(pattern);
	_lookup[pattern] = rep;
      }
    else
      rep = _lookup[pattern];
    std::regex& re = *rep;

    /* Multiline is the default.  */
    std::regex_constants::match_flag_type flags = std::regex_constants::match_continuous;
    if (_pos > 0)
      flags |= std::regex_constants::match_prev_avail;
#ifdef BOOST_REGEX
    flags |= std::regex_constants::match_not_dot_newline;
#endif

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
