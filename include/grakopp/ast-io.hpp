/* grakopp/ast.hpp - Grako++ AST header file
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
    void xml_escape(std::string* data) const
    {
      using boost::algorithm::replace_all;
      replace_all(*data, "&",  "&amp;");
      replace_all(*data, "\"", "&quot;");
      replace_all(*data, "\'", "&apos;");
      replace_all(*data, "<",  "&lt;");
      replace_all(*data, ">",  "&gt;");
    }
    
    void json_escape(std::string* data) const
    {
      using boost::algorithm::replace_all;
      replace_all(*data, "\\", "\\\\");
      replace_all(*data, "\"",  "\\\"");
      replace_all(*data, "\b",  "\\\b");
      replace_all(*data, "\f",  "\\\f");
      replace_all(*data, "\n",  "\\\n");
      replace_all(*data, "\t",  "\\\t");
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
      _cout << "[\n";
      {
	AstIndentGuard ind_cout(_cout);
	
	for (auto& child: list)
	  {
	    if (first)
	      first = false;
	    else
	      _cout << ", \n";
	    _cout << *child;
	  }
      }
      _cout << "\n]";
    }

    void operator() (const AstMap& map) const
    {
      bool first = true;
      _cout << "{\n";
      {
	AstIndentGuard ind_out(_cout);

	for (auto& key: map._order)
	  {
	    if (first)
	      first = false;
	    else
	      _cout << ", \n";
	    _cout << "\"" << key << "\" : " << *map.at(key);
	  }
      }
      _cout << "\n}";
    }

    void operator() (const AstException& exc) const
    {
      _cout << *exc;
    }

  };

  AstDumper dumper(cout);
  boost::apply_visitor(dumper, ast._content);
  return cout;
}


/* Forward declaration.  */
std::istream& operator>> (std::istream& is, AstPtr& val);

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
  else
    throw std::invalid_argument("AST expected");

  return is;
}
    

#endif /* _GRAKOPP_AST_IO_HPP */
