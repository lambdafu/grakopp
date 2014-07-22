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


class GrakoException : std::exception
{
};


class ParseError : public GrakoException
{
};


class FailedParseBase : public ParseError
{
public:
  FailedParseBase() {}
  virtual std::ostream& output(std::ostream& cout) const = 0;
  virtual void _throw() = 0;
};

std::ostream& operator<< (std::ostream& cout, const FailedParseBase& exc)
{
  return exc.output(cout);
}


class FailedParse : public FailedParseBase
{
public:
  FailedParse(const std::string& msg) : FailedParseBase(), _msg(msg) {}
  const std::string _msg;
  std::ostream& output(std::ostream& cout) const
  {
    return cout << _msg;
  }
  void _throw() { throw *this; }
};


class FailedToken : public FailedParseBase
{
public:
  FailedToken(const std::string& token) : FailedParseBase(), _token(token) {}
  const std::string _token;
  std::ostream& output(std::ostream& cout) const
  {
    return cout << "expecting \"" << _token << "\"";
  }
  void _throw() { throw *this; }
};


class FailedPattern : public FailedParseBase
{
public:
  FailedPattern(const std::string& pattern) : FailedParseBase(), _pattern(pattern) {}
  const std::string _pattern;
  std::ostream& output(std::ostream& cout) const
  {
    return cout << "expecting \"" << _pattern << "\"";
  }
  void _throw() { throw *this; }
};


class FailedLookahead : public FailedParseBase
{
public:
  FailedLookahead(const std::string& msg) : FailedParseBase() {}
  std::ostream& output(std::ostream& cout) const
  {
    return cout << "failed lookahead";
  }
  void _throw() { throw *this; }
};

#endif /* GRAKOPP_EXCEPTIONS_HPP */
