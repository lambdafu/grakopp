#include <string>
#include <cctype>
#include <functional>
#include <stack>
#include <map>
#include <list>
#include <memory>
#include <iostream>
#include <assert.h>

#include <boost/variant.hpp>
#include <boost/algorithm/string/replace.hpp>

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

/* FIXME: Maybe use unicode instead char.  The following is not
   unicode safe (for example, skip_to used with a non-7bit value).  */

typedef char CHAR_T;
#define CHAR_NULL ((CHAR_T) 0) /* Sort of.  */

class Buffer
{
public:
  Buffer(const std::string &text)
    : _text(text), _pos(0)
  {
  }

  const std::string _text;
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
  
};
  
typedef int state_t;

/* Simple container for parser exceptions, to avoid ambiguities in the
   boost::variant.  */
class AstException
{
public:
  AstException(std::shared_ptr<FailedParseBase> exc)
    : _exc(exc)
  {}
  std::shared_ptr<FailedParseBase> _exc;
  const FailedParseBase& operator*() const
  {
    return *_exc;
  }
};

class Ast;
using AstPtr = std::shared_ptr<Ast>;
AstPtr& operator<<(AstPtr& augend, const AstPtr& addend);

using AstNone = std::nullptr_t;
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
	    AstList& list = boost::get<AstList>(_augend._content);
	    AstList& rlist = boost::get<AstList>(_addend->_content);
	    list.insert(list.end(), rlist.begin(), rlist.end());
	  }
	else
	  _augend._content = AstList({ augend, _addend });
      }

      void operator() (AstList& list)
      {
	if (_mergeable)
	  {
	    AstList& rlist = boost::get<AstList>(_addend->_content);
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
    const AstPtr& operator=(const AstPtr& value)
    {
      AstException *exc = boost::get<AstException>(&value->_content);
      if (exc)
	_ast._content = *exc;
      else
	{
	  AstMap& map = boost::get<AstMap>(_ast._content);
	  /* Extend the existing value.  */
	  map.at(_key) << value;
	}
      return value;
    }
  };

  mapped_type operator[](const char *key)
  {
    return mapped_type(*this, key);
  }

};

AstPtr& operator<<(AstPtr& augend, const AstPtr& addend)
{
  augend->add(addend);
  return augend;
}



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
      for (auto child: list)
	{
	  if (first)
	    first = false;
	  else
	    _cout << ", \n";
	  _cout << "  " << *child;
	}
      _cout << "]\n";
    }

    void operator() (const AstMap& map) const
    {
      bool first = true;
      _cout << "{\n";
      for (auto key: map._order)
	{
	  if (first)
	    first = false;
	  else
	      _cout << ", \n";
	  _cout << "  \"" << key << "\" : " << *map.at(key);
	}
      _cout << "}\n";
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


class Parser
{
public:
  Parser(Buffer& buffer)
    : _buffer(buffer)
  {
  }

  Buffer& _buffer;

  using memo_key_t = std::tuple<size_t, std::string, state_t>;
  using memo_value_t = std::tuple<AstPtr, size_t, state_t>;
  std::map<memo_key_t, memo_value_t> _memoization_cache;
  state_t _state;

  template<typename T>
  AstPtr _error(std::string msg)
  {
    AstException exc(std::make_shared<T>(msg));
    AstPtr ast = std::make_shared<Ast>(exc);
    return ast;
  }

  AstPtr _fail()
  {
    return _error<FailedParse>("fail");
  }

  AstPtr _check_eof()
  {
    _buffer.next_token();
    if (! _buffer.atend())
      return _error<FailedParse>("Expecting end of text.");
    return std::make_shared<Ast>();
  }

  AstPtr _cut()
  {
    /* This AST object will be merged into the actual AST and set the
       cut flag there.  */
    AstPtr ast = std::make_shared<Ast>();
    ast->_cut = true;

    /* TODO: Kill memoization cache.  */

    return ast;
  }

  AstPtr _token(std::string token)
  {
    _buffer.next_token();
    if (! _buffer.match(token))
      {
	return _error<FailedToken>(token);
      }
    // _trace_match(token);
    AstPtr node = std::make_shared<Ast>(token);
    return node;
  }

  /* In case of an exception, the parser state is unmodified (useful
     to implement choices etc.)  The exception is passed through anyway!  */
  AstPtr _try(std::function<AstPtr ()> func)
  {
    size_t pos = _buffer._pos;
    state_t state = _state;
    AstPtr ast = func();
    if (boost::get<AstException>(&ast->_content))
      {
	_state = state;
	_buffer._pos = pos;
      }
    return ast;
  }

  AstPtr _option(bool &success, std::function<AstPtr ()> func)
  {
    /* Sets success to true if succeeds (otherwise does not touch it).  */
    AstPtr ast = _try(func);
    if (boost::get<AstException>(&ast->_content) && !ast->_cut)
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
    // 	AstPtr ast = std::make_shared<Ast>();
    // 	bool success = false;
    // 	ast << _option(success, func);
    // 	return ast;
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
    AstList *list = boost::get<AstList>(&ast->_content);
    if (list)
      list->_mergeable = true;
    return ast;
  }

  AstPtr _if(std::function<AstPtr ()> func)
  {
    size_t pos = _buffer._pos;
    state_t state = _state;
    // _enter_lookahead

    AstPtr ast = func();

    // _leave_lookahead
    _state = state;
    _buffer._pos = pos;

    /* Only pass through failures.  */
    if (boost::get<AstException>(&ast->_content))
      return ast;
    else
      return std::make_shared<Ast>();
  }

  AstPtr _ifnot(std::function<AstPtr ()> func)
  {
    AstPtr ast = _if(func);
    /* Invert result.  */
    if (boost::get<AstException>(&ast->_content))
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
	size_t pos = _buffer._pos;
	AstPtr ast = _try(func);

	/* Only if no exception! */
	if (!boost::get<AstException>(&ast->_content) && (pos == _buffer._pos))
	  return _error<FailedParse>("empty closure");

	if (boost::get<AstException>(&ast->_content))
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
    if (boost::get<AstException>(&ast->_content))
      return ast;

    /* We need to merge the closure.  */
    AstPtr opt_ast = _closure(func);
    AstList *list = boost::get<AstList>(&opt_ast->_content);
    if (list)
      list->_mergeable = true;

    return ast << opt_ast;
  }

};

#define RETURN_IF_EXC(ast) if (boost::get<AstException>(&ast->_content)) return ast

/* This stuff is generated.  */
class MyParser : public Parser
{
public:
  MyParser(Buffer& buffer)
    : Parser(buffer)
  {
  }

  AstPtr startrule()
  {
    /* This is generated for concrete rules.  */
    AstPtr ast = std::make_shared<Ast>();

    ast << _ifnot([this] () {
	AstPtr ast = std::make_shared<Ast>();
	ast << _token("g"); RETURN_IF_EXC(ast);
	return ast;
      }); RETURN_IF_EXC(ast);
    ast << _token("foo"); RETURN_IF_EXC(ast);
    ast << _group([this] () {
	AstPtr ast = std::make_shared<Ast>();
	ast << _token("bar"); RETURN_IF_EXC(ast);
	ast << _token("baz"); RETURN_IF_EXC(ast);
	return ast;
      }); RETURN_IF_EXC(ast);
    ast << _token("bar"); RETURN_IF_EXC(ast);

    ast << _choice([this] () {
	AstPtr ast = std::make_shared<Ast>();
	bool success = false;
	ast << _option(success, [this] () {
	    AstPtr ast = std::make_shared<Ast>();
	    ast << _token("foo"); RETURN_IF_EXC(ast);
	    return ast;
	  }); if (success) return ast;
	ast << _option(success, [this] () {
	    AstPtr ast = std::make_shared<Ast>();
	    ast << _token("ba"); RETURN_IF_EXC(ast);
	    ast << _cut();
	    ast << _token("r"); RETURN_IF_EXC(ast);
	    return ast;
	  }); if (success) return ast;
	ast << _option(success, [this] () {
	    AstPtr ast = std::make_shared<Ast>();
	    ast << _token("baz"); RETURN_IF_EXC(ast);
	    return ast;
	  }); if (success) return ast;
	ast << _error<FailedParse>("expecting one of: foo bar baz");
	return ast;
      }); RETURN_IF_EXC(ast);

    ast << _positive_closure([this] () {
	AstPtr ast = std::make_shared<Ast>();
	ast << _token("tro"); RETURN_IF_EXC(ast);
	return ast;
      }); RETURN_IF_EXC(ast);
    ast << _closure([this] () {
	AstPtr ast = std::make_shared<Ast>();
	ast << _token("lo"); RETURN_IF_EXC(ast);
	return ast;
      }); RETURN_IF_EXC(ast);


    // ast << _fail();

    return ast;

    /* This is generated for abstract rules.  */
    // AstPtr ast = std::make_shared<Ast>
    //   (AstMap({
    // 	  { "foo", AST_DEFAULT },
    //       { "bar", AST_FORCELIST }
    // 	}));

    // (*ast)["foo"] = _token("foo"); RETURN_IF_EXC(ast);
    // (*ast)["bar"] = _token("bar"); RETURN_IF_EXC(ast);
    // ast << _token("baz"); RETURN_IF_EXC(ast);
    // ast << _check_eof(); RETURN_IF_EXC(ast);
    // return ast;

    //    this->_try([] () {
    // do stuff
    //      });
  }
};


int
main(int argc, char *argv[])
{
  Buffer buf(argv[1]);
  MyParser parser(buf);

  try
    {
      AstPtr ast = parser.startrule();
      std::cout << *ast << "\n";
      AstException *exc = boost::get<AstException>(&ast->_content);
      if (exc)
	exc->_exc->_throw();
    }
  catch(FailedParseBase& exc)
    {
      std::cout << "Error: " << exc << "\n";
    }

  return 0;
}
