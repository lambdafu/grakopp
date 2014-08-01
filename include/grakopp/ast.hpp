/* grakopp/ast.hpp - Grako++ AST header file
   Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
   Written by Marcus Brinkmann <m.brinkmann@semantics.de>

   This file is part of Grako++.  Grako++ is free software; you can
   redistribute it and/or modify it under the terms of the 2-clause
   BSD license, see file LICENSE.TXT.
*/

#ifndef _GRAKOPP_AST_HPP
#define _GRAKOPP_AST_HPP 1

#include <string>
#include <list>
#include <map>
#include <iostream>
#include <memory>

#include <boost/variant.hpp>

#include <assert.h>

#include "exceptions.hpp"

/* Simple container for parser exceptions, to avoid ambiguities in the
   boost::variant.  */
class AstException
{
public:
  std::shared_ptr<FailedParseBase> _exc;

  AstException() {}
  AstException(std::shared_ptr<FailedParseBase> exc)
    : _exc(exc)
  {}
  const FailedParseBase& operator*() const
  {
    return *_exc;
  }

  bool operator==(const AstException& exc) const
  {
    return *_exc == *exc._exc;
  }
};

class Ast;

using AstPtr = std::shared_ptr<Ast>;
AstPtr& operator<<(AstPtr& augend, const AstPtr& addend);


/* Just a dummy type.  */
class AstNone {};

using AstString = std::string;

class AstList : public std::list<AstPtr>
{
public:
  AstList() : _mergeable(false) {}
  AstList(std::list<AstPtr> list) : std::list<AstPtr>(list), _mergeable(false) {}
  /* A mergeable list can extend a preceding list on the concrete
     stack.  Example: Grammar "a" ("b" "c") would create a mergeable
     list containing the group tokens.  Mergeability is a property of
     the right-hand side only.  Closures are not mergeable.  */
  bool _mergeable;
};


class AstMap : public std::map<std::string, AstPtr>
{
#define AST_DEFAULT 0
#define AST_FORCELIST 1

public:
  std::vector<std::string> _order;

  AstMap() {}

  AstMap(std::vector<std::pair<std::string, int>> keys)
  {
    for (auto pair: keys)
      {
	const std::string& key = pair.first;
	bool force_list = !!(pair.second & AST_FORCELIST);

	_order.push_back(key);
	if (force_list)
	  (*this)[key] = std::make_shared<Ast>(AstList());
	else
	  (*this)[key] = std::make_shared<Ast>();
      }
  }
};


class Ast
{
public:
  Ast() : _content(AstNone()), _cut(false) {}
  Ast(const AstString& str) : _content(str), _cut(false) {}
  Ast(const AstList& list) : _content(list), _cut(false) {}
  Ast(const AstMap& map) : _content(map), _cut(false) {}
  Ast(const AstException& exc) : _content(exc), _cut(false) {}

  /* Payload variants.  */
  boost::variant<AstNone, AstString, AstList, AstMap, AstException> _content;

  /* This is set if a cut was encountered during parsing.  */
  bool _cut;

  void set(const AstNone& none)
  {
    _content = none;
  }

  void set(const AstString& string)
  {
    _content = string;
  }

  void set(const AstList& list)
  {
    _content = list;
  }

  void set(const AstMap& map)
  {
    _content = map;
  }

  void set(const AstException& exc)
  {
    _content = exc;
  }


  const AstNone& the_none() const
  {
    return boost::get<AstNone>(_content);
  }

  AstNone& the_none()
  {
    return boost::get<AstNone>(_content);
  }

  const AstNone* as_none() const
  {
    return boost::get<AstNone>(&_content);
  }

  AstNone* as_none()
  {
    return boost::get<AstNone>(&_content);
  }

  const AstString& the_string() const
  {
    return boost::get<AstString>(_content);
  }

  AstString& the_string()
  {
    return boost::get<AstString>(_content);
  }

  const AstString* as_string() const
  {
    return boost::get<AstString>(&_content);
  }

  AstString* as_string()
  {
    return boost::get<AstString>(&_content);
  }

  const AstList& the_list() const
  {
    return boost::get<AstList>(_content);
  }

  AstList& the_list()
  {
    return boost::get<AstList>(_content);
  }

  const AstList* as_list() const
  {
    return boost::get<AstList>(&_content);
  }

  AstList* as_list()
  {
    return boost::get<AstList>(&_content);
  }

  const AstMap& the_map() const
  {
    return boost::get<AstMap>(_content);
  }

  AstMap the_map()
  {
    return boost::get<AstMap>(_content);
  }

  const AstMap* as_map() const
  {
    return boost::get<AstMap>(&_content);
  }

  AstMap* as_map()
  {
    return boost::get<AstMap>(&_content);
  }

  const AstException& the_exception() const
  {
    return boost::get<AstException>(_content);
  }

  AstException& the_exception()
  {
    return boost::get<AstException>(_content);
  }

  const AstException* as_exception() const
  {
    return boost::get<AstException>(&_content);
  }

  AstException* as_exception()
  {
    return boost::get<AstException>(&_content);
  }


  /* Concrete nodes use AstNone, AstString and AstList.  Abstract nodes
     use AstMap.  */
  class AstAdder : public boost::static_visitor<void>
  {
  public:
    AstAdder(Ast& augend, const AstPtr& addend)
      : _augend(augend), _addend(addend)
    {
    }

    Ast& _augend;
    const AstPtr& _addend;

    class AstAdderTo : public boost::static_visitor<void>
    {
    public:
      AstAdderTo(Ast& augend, const AstPtr& addend, bool mergeable=false)
	: _augend(augend), _addend(addend), _mergeable(mergeable)
      {
      }

      Ast& _augend;
      const AstPtr& _addend;
      bool _mergeable;

      void operator() (AstNone& none)
      {
	/* None-augend is replaced.  */
	_augend._content = _addend->_content;
      }

      void operator() (AstString& str)
      {
	AstPtr augend = std::make_shared<Ast>(str);

	if (_mergeable)
	  {
	    _augend._content = AstList({ augend });
	    AstList& list = _augend.the_list();
	    AstList& rlist = _addend->the_list();
	    list.insert(list.end(), rlist.begin(), rlist.end());
	  }
	else
	  _augend._content = AstList({ augend, _addend });
      }

      void operator() (AstList& list)
      {
	if (_mergeable)
	  {
	    AstList& rlist = _addend->the_list();
	    list.insert(list.end(), rlist.begin(), rlist.end());
	  }
	else
	  {
	    list.push_back(_addend);
	  }
      }

      void operator() (AstMap& map)
      {
	/* Adding to a map is ignored - this is used to set exceptions
	   in the named-parameter case, and otherwise ignored.  */

	/* FIXME: No.  Adding a map to a map can be entirely intentional,
	   for example within an option:
	   foo = "a" (key: "b" "c") "d";
	   Maybe add mergeable flag to maps?  */
      }

      void operator() (AstException& exc)
      {
	/* Can this happen?  */
	assert(!"AstAdder can't add to an exception.");
      }
    };

    void operator() (AstNone& none)
    {
      /* None addend is ignored (except for cut flag), can happen with
	 lookaheads, for example.  */
    }

    void operator() (AstString& str)
    {
      AstAdderTo adder_to(_augend, _addend);
      boost::apply_visitor(adder_to, _augend._content);
    }

    void operator() (AstList& list)
    {
      AstAdderTo adder_to(_augend, _addend, list._mergeable);
      boost::apply_visitor(adder_to, _augend._content);
    }

    void operator() (AstMap& map)
    {
      AstAdderTo adder_to(_augend, _addend);
      boost::apply_visitor(adder_to, _augend._content);
    }

    void operator() (AstException& exc)
    {
      /* Exceptions overwrite everything.  */
      _augend._content = exc;
    }
  };

  void add (const AstPtr& addend)
  {
    AstAdder adder(*this, addend);
    /* If the cut flag was encountered in the addend, make sure it is
       set for the augend, too.  Note that this can leave dangling
       _cut flags being true in child nodes of the AST (for example,
       if a map is added to a list).  This is not a problem, as they
       are never checked.  */
    if (addend->_cut)
      _cut = true;
    boost::apply_visitor(adder, addend->_content);
  }

  class mapped_type
  {
  public:
    mapped_type(Ast& ast, const char *key) : _ast(ast), _key(key) {}
    Ast& _ast;
    std::string _key;
    mapped_type& operator<<(const AstPtr& value)
    {
      AstException *exc = value->as_exception();
      if (exc)
	_ast._content = *exc;
      else
	{
	  AstMap* map = _ast.as_map();

	  if (!map)
	    {
	      /* Coerce left side to a map.  This covers nested named items, such as
		 "rule = ( name: value );".  */
	      _ast._content = AstMap();
	      map = _ast.as_map();
	    }

	  /* Also in the nested name case, the key may not already exist.  */
	  if (map->count(_key) == 0)
	    (*map)[_key] = std::make_shared<Ast>();

	  /* Extend the existing value.  */
	  map->at(_key) << value;
	}
      return *this;
    }

  };

  mapped_type operator[](const char *key)
  {
    return mapped_type(*this, key);
  }

  
  class AstComparator : public boost::static_visitor<bool>
  {
  public:
    const Ast& _other;
    AstComparator(const Ast& other) : _other(other) {}
    
    bool operator() (const AstNone& none) const
    {
      return boost::get<AstNone>(&_other._content);
    }
    
    bool operator() (const AstString& leaf) const
    {
      const AstString *ast_leaf = _other.as_string();
      return ast_leaf && (leaf == *ast_leaf);
    }
    
    bool operator() (const AstList& list) const
    {
      const AstList *ast_list = _other.as_list();
      return ast_list && (list == *ast_list);
    }

    bool operator() (const AstMap& map) const
    {
      const AstMap *ast_map = _other.as_map();
      return ast_map && (map == *ast_map);
    }

    bool operator() (const AstException& exception) const
    {
      const AstException *ast_exception = _other.as_exception();
      return ast_exception && (exception == *ast_exception);
    }
  };
    
  inline bool operator== (const Ast &ast) const
  {
    AstComparator comparator(ast);
    return boost::apply_visitor(comparator, _content);
  }
};


inline AstPtr& operator<<(AstPtr& augend, const AstPtr& addend)
{
  augend->add(addend);
  return augend;
}

inline bool operator== (const AstPtr& ast1, const AstPtr& ast2)
{
  return *ast1 == *ast2;
}

inline bool operator!= (const AstPtr& ast1, const AstPtr& ast2)
{
  return !(*ast1 == *ast2);
}


#endif /* GRAKOPP_AST_HPP */
