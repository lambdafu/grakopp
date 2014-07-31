/* grakopp/ast-io.hpp - Grako++ AST I/O header file
   Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
   Written by Marcus Brinkmann <m.brinkmann@semantics.de>

   This file is part of Grako++.  Grako++ is free software; you can
   redistribute it and/or modify it under the terms of the 2-clause
   BSD license, see file LICENSE.TXT.
*/

#ifndef _GRAKOPP_AST_IO_HPP
#define _GRAKOPP_AST_IO_HPP 1

#include <ostream>
#include <streambuf>
#include <boost/algorithm/string/replace.hpp>

#include "ast.hpp"


class AstIndentGuard : public std::streambuf
{
  std::streambuf* _out;
  bool _bol;
  std::string _indent;
  std::ostream* _owner;

protected:
  virtual int overflow(int ch)
  {
    if (_bol && ch != '\n')
      {
	_out->sputn(_indent.data(), _indent.size());
	_bol = false;
      }
    else if (ch == '\n')
      _bol = true;

    return _out->sputc(ch);
  }

public:
  explicit AstIndentGuard(std::streambuf* out, int indent=4)
    : _out(out), _bol(true), _indent(indent, ' '), _owner(0)
  {
  }

  explicit AstIndentGuard(std::ostream& out, int indent=4)
    : _out(out.rdbuf()), _bol(true), _indent(indent, ' '), _owner(&out)
  {
    _owner->rdbuf(this);
  }

  ~AstIndentGuard()
  {
    if (_out)
      _owner->rdbuf(_out);
  }
};


std::ostream& operator<< (std::ostream& cout, const Ast& ast)
{
  class AstDumper : public boost::static_visitor<void>
  {
    void json_escape(std::string* data) const
    {
      using boost::algorithm::replace_all;
      replace_all(*data, "\\", "\\\\");
      replace_all(*data, "\"", "\\\"");
      replace_all(*data, "\b", "\\b");
      replace_all(*data, "\f", "\\f");
      replace_all(*data, "\n", "\\n");
      replace_all(*data, "\t", "\\t");
      /* FIXME: Replace all < 32 */
    }
    
  public:
    AstDumper(std::ostream& cout) : _cout(cout) {}
    std::ostream& _cout;
    
    void operator() (const AstNone& none) const
    {
      _cout << "null";
    }

    void operator() (const AstString& leaf) const
    {
      std::string _leaf = leaf;
      json_escape(&_leaf);
      _cout << "\"" << _leaf << "\"";
    }

    void operator() (const AstList& list) const
    {
      bool first = true;
      _cout << "[";
      {
	AstIndentGuard ind_cout(_cout);
	
	for (auto& child: list)
	  {
	    if (first)
	      {
		first = false;
		_cout << "\n";
	      }
	    else
	      _cout << ",\n";
	    _cout << *child;
	  }
      }
      if (!first)
	_cout << "\n";
      _cout << "]";
    }

    void operator() (const AstMap& map) const
    {
      bool first = true;
      _cout << "{";
      {
	AstIndentGuard ind_out(_cout);

	for (auto& key: map._order)
	  {
	    if (first)
	      {
		first = false;
		_cout << "\n";
	      }
	    else
	      _cout << ",\n";
	    _cout << "\"" << key << "\" : " << *map.at(key);
	  }
	/* FIXME: Output those keys which are in the map but not in
	   _order?  */
      }
      if (!first)
	_cout << "\n";
      _cout << "}";
    }

    void operator() (const AstException& exc) const
    {
      std::string msg = (*exc).what();
      json_escape(&msg);
      _cout << (*exc).type() << "(\"" << msg << "\")";
    }

  };

  AstDumper dumper(cout);
  boost::apply_visitor(dumper, ast._content);
  return cout;
}


/* Forward declaration.  */
std::istream& operator>> (std::istream& is, AstPtr& val);

std::istream& operator>> (std::istream& is, AstNone& val)
{
  char ch;
  is >> ch;
  if (ch != 'n')
    throw std::invalid_argument("null expected");

  is >> ch;
  if (ch != 'u')
    throw std::invalid_argument("null expected");

  is >> ch;
  if (ch != 'l')
    throw std::invalid_argument("null expected");

  is >> ch;
  if (ch != 'l')
    throw std::invalid_argument("null expected");

  return is;
}

std::istream& operator>> (std::istream& is, AstString& val)
{
  char ch;

  if (is.peek() != '"')
    throw std::invalid_argument("quote expected");
  is >> ch;

  while (! is.eof())
    {
      is >> ch;

      if (ch == '"')
	return is;
      else if (ch == '\\')
	{
	  is >> ch;
	  if (ch == 'b')
	    val += '\b';
	  else if (ch == 'f')
	    val += '\f';
	  else if (ch == 'n')
	    val += '\n';
	  else if (ch == 'r')
	    val += '\r';
	  else if (ch == 't')
	    val += '\t';
	  else if (ch == 'u')
	    throw std::invalid_argument("unicode support not implemented yet");
	  else
	    val += ch;
	}
      else
	val += ch;
    }
  throw std::invalid_argument("EOF in string");
}

std::istream& operator>> (std::istream& is, AstList& val)
{
  char ch;

  if (is.peek() != '[')
    throw std::invalid_argument("list expected");
  is >> ch >> std::ws;

  while (! is.eof() )
    {
      if (is.peek() == ']')
	{
	  is >> ch;
	  return is;
	}

      AstPtr ast = std::make_shared<Ast>();
      is >> ast >> std::ws;
      val.push_back(ast);

      ch = is.peek();
      if (ch == ',')
	is >> ch >> std::ws;
      if (ch != ',' && ch != ']')
	throw std::invalid_argument("expected comma");
    }
  throw std::invalid_argument("EOF in list");
}

std::istream& operator>> (std::istream& is, AstMap& val)
{
  char ch;

  if (is.peek() != '{')
    throw std::invalid_argument("map expected");
  is >> ch >> std::ws;

  while (! is.eof())
    {
      if (is.peek() == '}')
	{
	  is >> ch;
	  return is;
	}

      AstString key;
      is >> key >> std::ws;

      if (is.peek() != ':')
	throw std::invalid_argument("expected colon");
      is >> ch >> std::ws;

      AstPtr ast = std::make_shared<Ast>();
      is >> ast >> std::ws;

      /* FIXME: Maybe override in AstMap.  */
      val._order.push_back(key);
      val[key] = ast;

      ch = is.peek();
      if (ch == ',')
	is >> ch >> std::ws;
      if (ch != ',' && ch != '}')
	throw std::invalid_argument("expected comma");
    }
  throw std::invalid_argument("EOF in list");
}

std::istream& operator>> (std::istream& is, AstException& exc)
{
#define MAX_TOKEN 40
  char token[MAX_TOKEN];
  is.getline(token, MAX_TOKEN, '(');
  /* Paranoia.  */
  token[MAX_TOKEN - 1] = '\0';
  /* FIXME: Report better error on invalid input, in particular if
     opening parenthesis is missing.  */

  AstString msg;
  is >> msg;

  if (is.peek() != ')')
    throw std::invalid_argument("closing parenthesis expected");
  is.ignore(1);

  if (!strcmp(token, "FailedParse"))
    exc._exc = std::make_shared<FailedParse>(msg);
  else if (!strcmp(token, "FailedToken"))
    exc._exc = std::make_shared<FailedToken>(msg);
  else if (!strcmp(token, "FailedPattern"))
    exc._exc = std::make_shared<FailedPattern>(msg);
  else if (!strcmp(token, "FailedLookahead"))
    exc._exc = std::make_shared<FailedLookahead>(msg);
  else
    throw std::invalid_argument("unknown exception");

  return is;
}


std::istream& operator>> (std::istream& is, AstPtr& val)
{
  char ch = is.peek();

  if (ch == '"')
    {
      AstString str;
      is >> str;
      val->_content = str;
    }
  else if (ch == '[')
    {
      AstList list;
      is >> list;
      val->_content = list;
    }
  else if (ch == '{')
    {
      AstMap map;
      is >> map;
      val->_content = map;
    }
  else if (ch == 'F')
    {
      AstException exc;
      is >> exc;
      val->_content = exc;
    }
  else if (ch == 'n')
    {
      AstNone none;
      is >> none;
      val->_content = none;
    }
  else
    throw std::invalid_argument("AST expected");

  return is;
}
    

#endif /* _GRAKOPP_AST_IO_HPP */
