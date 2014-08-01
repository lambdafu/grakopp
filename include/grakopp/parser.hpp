/* grakopp/parser.hpp - Grako++ parser header file
   Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
   Written by Marcus Brinkmann <m.brinkmann@semantics.de>

   This file is part of Grako++.  Grako++ is free software; you can
   redistribute it and/or modify it under the terms of the 2-clause
   BSD license, see file LICENSE.TXT.
*/

#ifndef _GRAKOPP_PARSER_HPP
#define _GRAKOPP_PARSER_HPP 1

#include <functional>
#include <string>
#include <map>
#include <cctype>

#include "exceptions.hpp"
#include "buffer.hpp"
#include "ast.hpp"


/* OPTIMIZATION: Somehow use templating to make grammar rules with
   lambda better inline-optimizable (sim. to C++ map).  */
/* OPTIMIZATION: Specialise for void State.  */

class NoSemantics
{
};

template <typename _Semantics=NoSemantics, typename _State=int>
class Parser
{
public:
  using State = _State;
  using Semantics = _Semantics;
  using semantics_func_t = AstPtr (Semantics::*) (AstPtr&);

  Parser(Semantics* semantics=nullptr)
    : _buffer(std::make_shared<Buffer>()),
      _whitespace(" \t\r\n\x0b\x0c"),
      _nameguard_set(false), _nameguard(true),
      _state(), _semantics(semantics)
      { }

  BufferPtr _buffer;
  std::string _whitespace;
  bool _nameguard_set;
  bool _nameguard;
  State _state;
  Semantics *_semantics;

  /* We sort by position first, because we can then optimize the cut
     operator.  However, if you use an unordered map here, remember to
     iterate over all items in the cut operator.  */
  using memo_key_t = std::tuple<size_t, std::string, State>;
  using memo_value_t = std::tuple<AstPtr, size_t, State>;
  std::map<memo_key_t, memo_value_t> _memoization_cache;

  void _update_buffer()
  {
    if (!_buffer)
      return;
    _buffer->_whitespace = _whitespace;
    _buffer->_nameguard = _nameguard;
    _buffer->_pos = 0;
  }

  void set_buffer(const BufferPtr& buffer)
  {
    _buffer = buffer;
    _update_buffer();
  }

  void set_whitespace(const std::string& whitespace)
  {
    _whitespace = whitespace;
    /* Reset the default for _nameguard, if not set explicitely.  */
    if (! _nameguard_set)
      _nameguard = _whitespace.length() > 0;
    _update_buffer();
  }

  void set_nameguard(bool nameguard)
  {
    _nameguard_set = true;
    _nameguard = nameguard;
    _update_buffer();
  }

  void reset()
  {
    _memoization_cache.clear();
    _update_buffer();
  }

  template<typename T>
  AstPtr _error(std::string msg)
  {
    AstException exc(std::make_shared<T>(msg));
    AstPtr ast = std::make_shared<Ast>(exc);
    return ast;
  }

  AstPtr _call(std::string name, semantics_func_t sem_func, std::function<AstPtr ()> func)
  {
    size_t pos = _buffer->_pos;
    State& state = _state;
    memo_key_t key(pos, name, state);

    {
      /* Check memoization cache.  */
      auto cache = _memoization_cache.find(key);
      if (cache != _memoization_cache.end())
	{
	  /* TODO: Trace.  */
	  memo_value_t& value = cache->second;
	  _buffer->_pos = std::get<1>(value);
	  _state = std::get<2>(value);
	  return std::get<0>(value);
	}
    }

    if (std::islower(name[0]))
      _buffer->next_token();

    /* Call rule.  */
    AstPtr ast = func();

    //if self.parseinfo:
    //  node._add('_parseinfo', ParseInfo(self._buffer, name, pos, self._pos))

    /* Maybe override the AST.  */
    AstMap *map = ast->as_map();
    if (map)
      {
	auto el = map->find("@");
	if (el != map->end())
	  ast = el->second;
      }

    /* Apply semantics.  */
    if (_semantics)
      if (!ast->as_exception())
	ast = (_semantics->*sem_func)(ast);
    size_t next_pos = _buffer->_pos;
    State& next_state = _state;

    /* Fill memoization cache.  FIXME: Check "don't memo lookaheads" flag. */
    memo_value_t value (ast, next_pos, next_state);
    _memoization_cache[key] = value;

    if (ast->as_exception())
      {
	_buffer->_pos = pos;
	_state = state;
      }
    return ast;
  }


  AstPtr _fail()
  {
    return _error<FailedParse>("fail");
  }

  AstPtr _check_eof()
  {
    _buffer->next_token();
    if (! _buffer->atend())
      return _error<FailedParse>("Expecting end of text.");
    return std::make_shared<Ast>();
  }

  AstPtr _cut()
  {
    /* This AST object will be merged into the actual AST and set the
       cut flag there.  */
    AstPtr ast = std::make_shared<Ast>();
    ast->_cut = true;

    /* Grako:

       "Kota Mizushima et al say that we can throw away memos for
       previous positions in the buffer under certain circumstances,
       without affecting the linearity of PEG parsing.
       http://goo.gl/VaGpj

       We adopt the heuristic of always dropping the cache for
       positions less than the current cut position. It remains to be
       proven if doing it this way affects linearity. Empirically, it
       hasn't."  */

    size_t cutpos = _buffer->_pos;
    /* This is a bit cheesy, but it'll work.  We need a string larger
       than all valid strings (and state doesn't matter then).  */
    memo_key_t last_key(cutpos, "\xff", State());
    auto upper = _memoization_cache.upper_bound(last_key);
    // std::cout << "Dropping " << std::distance(_memoization_cache.begin(),upper) << "\n";
    _memoization_cache.erase(_memoization_cache.begin(), upper);
    return ast;
  }

  AstPtr _token(const std::string& token)
  {
    _buffer->next_token();
    if (! _buffer->match(token))
      {
	return _error<FailedToken>(token);
      }
    // _trace_match(token);
    AstPtr node = std::make_shared<Ast>(token);
    return node;
  }

  AstPtr _pattern(const std::string& pattern)
  {
    boost::optional<std::string> maybe_token = _buffer->matchre(pattern);
    if (! maybe_token)
      {
	return _error<FailedPattern>(pattern);
      }
    const std::string& token = *maybe_token;
    // _trace_match(token);
    AstPtr node = std::make_shared<Ast>(token);
    return node;
  }

  /* In case of an exception, the parser state is unmodified (useful
     to implement choices etc.)  The exception is passed through anyway!  */
  AstPtr _try(std::function<AstPtr ()> func)
  {
    size_t pos = _buffer->_pos;
    State state = _state;
    AstPtr ast = func();
    if (ast->as_exception())
      {
	_state = state;
	_buffer->_pos = pos;
      }
    return ast;
  }

  AstPtr _option(bool &success, std::function<AstPtr ()> func)
  {
    /* Sets success to true if succeeds (otherwise does not touch it).  */
    AstPtr ast = _try(func);
    if (ast->as_exception() && !ast->_cut)
      {
	/* Non-cut exceptions are ignored, but don't report success
	   (normal failed option).  */
	return std::make_shared<Ast>();
      }
    /* Exceptions with cut are propagated with _cut set to true.  This
       is equivalent to nested FailedCut exceptions in Grako.  Yes,
       this counts as success (== don't consider more options).  */
    success = true;
    /* Forget the cut status after a successful parse.  */
    ast->_cut = false;
    return ast;
  }

  AstPtr _choice(std::function<AstPtr ()> func)
  {
    /* There is totally nothing to do here :), we just need the
       scope.  */
    AstPtr ast = _try(func);
    return ast;
  }

  AstPtr _optional(std::function<AstPtr ()> func)
  {
    // AstPtr ast = _choice([this, &func] () {
    //	AstPtr ast = std::make_shared<Ast>();
    //	bool success = false;
    //	ast << _option(success, func);
    //	return ast;
    //   });
    // return ast;
    // identical to:
    bool success = false;
    return _option(success, func);
  }

  AstPtr _group(std::function<AstPtr ()> func)
  {
    AstPtr ast = func();
    /* If a list is returned, make it mergable.  */
    AstList *list = ast->as_list();
    if (list)
      list->_mergeable = true;
    return ast;
  }

  AstPtr _if(std::function<AstPtr ()> func)
  {
    size_t pos = _buffer->_pos;
    State state = _state;
    // _enter_lookahead

    AstPtr ast = func();

    // _leave_lookahead
    _state = state;
    _buffer->_pos = pos;

    /* Only pass through failures.  */
    if (ast->as_exception())
      return ast;
    else
      return std::make_shared<Ast>();
  }

  AstPtr _ifnot(std::function<AstPtr ()> func)
  {
    AstPtr ast = _if(func);
    /* Invert result.  */
    if (ast->as_exception())
      return std::make_shared<Ast>();
    else
      /* If we had a invert() function on every exception, this could
	 provide more diagnostics, maybe? */
      return _error<FailedLookahead>("");
  }

  AstPtr _closure(std::function<AstPtr ()> func)
  {
    AstPtr cum_ast = std::make_shared<Ast>(AstList());

    do
      {
	size_t pos = _buffer->_pos;
	AstPtr ast = _try(func);

	/* Only if no exception! */
	if (!ast->as_exception() && (pos == _buffer->_pos))
	  return _error<FailedParse>("empty closure");

	if (ast->as_exception())
	  {
	    if (ast->_cut)
	      /* Exceptions after cut are fatal.  */
	      return ast;
	    else
	      /* Non-cut exceptions are ignored (just stop).  */
	      return cum_ast;
	  }

	/* Collect result.  */
	cum_ast << ast;
      }
    while (true);
  }

  AstPtr _positive_closure(std::function<AstPtr ()> func)
  {
    AstPtr ast = std::make_shared<Ast>(AstList());
    ast << func();
    if (ast->as_exception())
      return ast;

    /* We need to merge the closure.  */
    AstPtr opt_ast = _closure(func);
    AstList *list = opt_ast->as_list();
    if (list)
      list->_mergeable = true;

    return ast << opt_ast;
  }

};

#endif /* _GRAKOPP_PARSER_HPP */
