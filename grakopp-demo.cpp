#include <grakopp/grakopp.hpp>

/* This stuff is generated.  */
class MyParser : public Parser
{
public:

  MyParser()
    : Parser()
  {
  }

  typedef AstPtr (MyParser::*rule_method_t) ();
  rule_method_t find_rule(const std::string& name)
  {
    std::map<std::string, rule_method_t> map({
	{ "rule_one", &MyParser::rule_one },
	{ "rule_h1", &MyParser::rule_h1 },
	{ "startrule", &MyParser::startrule }
      });
    auto el = map.find(name);
    if (el != map.end())
      return el->second;
    return 0;
  }

  AstPtr rule_h1()
  {
    AstPtr ast = std::make_shared<Ast>();
    ast << _call("h1", [this] () {
      AstPtr ast = std::make_shared<Ast>();
      ast << _token(R"(=)"); RETURN_IF_EXC(ast);
      (*ast)["@"] << [this] () {
	AstPtr ast = std::make_shared<Ast>();
	ast << _pattern(R"([a-z]*)"); RETURN_IF_EXC(ast);
	return ast;
      }(); RETURN_IF_EXC(ast);
      ast << _token(R"(=)"); RETURN_IF_EXC(ast);
      return ast;
    }); RETURN_IF_EXC(ast);
    return ast;
  }

  AstPtr rule_one()
  {
    AstPtr ast = std::make_shared<Ast>
      (AstMap({
	  { "foo", AST_DEFAULT },
	    { "@", AST_FORCELIST }
	}));

    (*ast)["foo"] << _token("foo"); RETURN_IF_EXC(ast);
    (*ast)["@"] << _token("bar"); RETURN_IF_EXC(ast);
    ast << _token("baz"); RETURN_IF_EXC(ast);
    ast << _check_eof(); RETURN_IF_EXC(ast);
    return ast;
  };

  AstPtr startrule()
  {
    // AstPtr ast = std::make_shared<Ast>();
    // ast << _call("rule_one", [this] () { return this->rule_one(); }); RETURN_IF_EXC(ast);
    // return ast;

    /* This is generated for concrete rules.  */
    AstPtr ast = std::make_shared<Ast>();

    ast << _pattern("[a-z]"); RETURN_IF_EXC(ast);
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

    /* This is generated for abstract rules.  */
    {
    AstPtr ast = std::make_shared<Ast>
      (AstMap({
	  { "foo", AST_DEFAULT },
	    { "bar", AST_FORCELIST }
	}));

    (*ast)["foo"] << [this] () {
      AstPtr ast = std::make_shared<Ast>();
      ast << _token("foo"); RETURN_IF_EXC(ast);
      return ast;
    }();
    (*ast)["bar"] << _token("bar"); RETURN_IF_EXC(ast);
    ast << _token("baz"); RETURN_IF_EXC(ast);
    ast << _check_eof(); RETURN_IF_EXC(ast);
    }
    //    this->_try([] () {
    // do stuff
    //      });

    return ast;

  }
};


int
main(int argc, char *argv[])
{
  BufferPtr buf = std::make_shared<Buffer>();
  MyParser parser;

  buf->from_file(argv[1]);
  parser.set_buffer(buf);

  try
    {
      MyParser::rule_method_t rule = parser.find_rule(argv[2]);
      AstPtr ast = (parser.*rule)();
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
