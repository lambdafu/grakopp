/* grakopp/exceptions.hpp - Grako++ exceptions header file
   Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
   Written by Marcus Brinkmann <m.brinkmann@semantics.de>

   This file is part of Grako++.  Grako++ is free software; you can
   redistribute it and/or modify it under the terms of the 2-clause
   BSD license, see file LICENSE.TXT.
*/

#ifndef _GRAKOPP_EXCEPTIONS_HPP
#define _GRAKOPP_EXCEPTIONS_HPP 1

#include <string>
#include <iostream>
#include <boost/algorithm/string/replace.hpp>


class GrakoException : public std::exception
{
};


class ParseError : public GrakoException
{
};


class FailedParseBase : public ParseError
{
public:
  FailedParseBase() {}
  virtual const char* type() const = 0;
  virtual const char* initializer() const = 0;
  virtual void _throw() = 0;

  bool operator== (const FailedParseBase& exc) const
  {
    return !strcmp(type(), exc.type()) && !strcmp(what(), exc.what());
  }
};


/* Serialization format.  */
inline std::ostream& operator<< (std::ostream& cout, const FailedParseBase& exc)
{
  return cout << exc.what();
}


class FailedParse : public FailedParseBase
{
  const std::string _msg;

public:
  FailedParse(const std::string& msg) : FailedParseBase(), _msg(msg) {}

  const char* what() const throw()
  {
    return _msg.data();
  }

  const char* type() const
  {
    return "FailedParse";
  }

  const char* initializer() const
  {
    return _msg.data();
  }

  void _throw() { throw *this; }
};


class FailedToken : public FailedParseBase
{
  const std::string _token;
  const std::string _msg;

public:
  FailedToken(const std::string& token)
    : FailedParseBase(), _token(token),
      _msg("expecting \"" + _token + "\"")
  {
  }

  const char* what() const throw()
  {
    return _msg.data();
  }

  const char* type() const
  {
    return "FailedToken";
  }

  const char* initializer() const
  {
    return _token.data();
  }

  void _throw() { throw *this; }
};


class FailedPattern : public FailedParseBase
{
  const std::string _pattern;
  const std::string _msg;

public:
  FailedPattern(const std::string& pattern)
    : FailedParseBase(), _pattern(pattern),
      _msg("expecting \"" + _pattern + "\"")
  {
  }

  const char* what() const throw()
  {
    return _msg.data();
  }

  const char* type() const
  {
    return "FailedPattern";
  }

  const char* initializer() const
  {
    return _pattern.data();
  }

  void _throw() { throw *this; }
};


class FailedLookahead : public FailedParseBase
{
  const std::string& _msg;
public:
  FailedLookahead(const std::string& msg) : FailedParseBase(), _msg(msg) {}

  const char* what() const throw()
  {
    return "failed lookahead";
  }

  const char* type() const
  {
    return "FailedLookahead";
  }

  const char* initializer() const
  {
    return _msg.data();
  }

  void _throw() { throw *this; }
};

#endif /* GRAKOPP_EXCEPTIONS_HPP */
