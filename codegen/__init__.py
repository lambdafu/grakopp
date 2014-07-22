# This code is based on Grako's code generators.

# FIX:
# named items map stuff
# cpp_repr  "...\'..."..."  bah!

# -*- coding: utf-8 -*-
"""
C++ code generation for models defined with grako.model
"""
from __future__ import absolute_import, division, print_function, unicode_literals

import keyword

from grako.util import indent, trim, timestamp, ustr, urepr, compress_seq
from grako.exceptions import CodegenError
from grako.model import Node
from grako.codegen.cgbase import ModelRenderer, CodeGenerator

def cpp_repr(str):
    return 'R"(' + urepr(str)[1:-1] + ')"'

class CppCodeGenerator(CodeGenerator):
    def _find_renderer_class(self, item):
        if not isinstance(item, Node):
            return None

        name = item.__class__.__name__
        renderer = globals().get(name, None)
        if not renderer or not issubclass(renderer, Base):
            raise CodegenError('Renderer for %s not found' % name)
        return renderer


def safe_name(name):
    if keyword.iskeyword(name):
        return name + '_'
    return name


def codegen(model):
    return CppCodeGenerator().render(model)


class Base(ModelRenderer):
    def defines(self):
        return []


class Void(Base):
    template = ';'


class Fail(Base):
    template = 'return _fail();'


class Comment(Base):
    template = '''
        /* {comment} */

        '''


class EOF(Base):
    template = 'ast << _check_eof(); RETURN_IF_EXC(ast);'


class _Decorator(Base):
    def defines(self):
        return self.get_renderer(self.node.exp).defines()

    template = '{exp}'


class Group(_Decorator):
    template = '''\
                ast << _group([this] () {{
                    AstPtr ast = std::make_shared<Ast>();
                {exp:1::}
                    return ast;
                }}); RETURN_IF_EXC(ast);\
               '''


class Token(Base):
    def render_fields(self, fields):
        fields.update(token=cpp_repr(self.node.token))

    template = "ast << _token({token}); RETURN_IF_EXC(ast);"


class Pattern(Base):
    def render_fields(self, fields):
        raw_repr = cpp_repr(self.node.pattern).replace("\\\\", '\\')
        fields.update(pattern=raw_repr)

    template = "ast << _pattern({pattern}); RETURN_IF_EXC(ast);"


class Lookahead(_Decorator):
    template = '''\
                ast << _if([this] () {{
                    AstPtr ast = std::make_shared<Ast>();
                {exp:1::}
                    return ast;
                }}); RETURN_IF_EXC(ast);\
                '''


class NegativeLookahead(_Decorator):
    template = '''\
                ast << _ifnot([this] () {{
                    AstPtr ast = std::make_shared<Ast>();
                {exp:1::}
                    return ast;
                }}); RETURN_IF_EXC(ast);\
                '''


class Sequence(Base):
    def defines(self):
        return [d for s in self.node.sequence for d in s.defines()]

    def render_fields(self, fields):
        fields.update(seq='\n'.join(self.rend(s) for s in self.node.sequence))

    template = '''
                {seq}\
                '''


class Choice(Base):
    def defines(self):
        return [d for o in self.node.options for d in o.defines()]

    def render_fields(self, fields):
        template = trim(self.option_template)
        options = [
            template.format(
                option=indent(self.rend(o))) for o in self.node.options
        ]
        options = '\n'.join(o for o in options)
        firstset = ' '.join(f[0] for f in sorted(self.node.firstset) if f)
        if firstset:
            error = 'expecting one of: ' + firstset
        else:
            error = 'no available options'
        fields.update(n=self.counter(),
                      options=indent(options),
                      error=cpp_repr(error)
                      )

    def render(self, **fields):
        if len(self.node.options) == 1:
            return self.rend(self.options[0], **fields)
        else:
            return super(Choice, self).render(**fields)

    option_template = '''\
                       ast << _option(success, [this] () {{
                           AstPtr ast = std::make_shared<Ast>();
                       {option}
                           return ast;
                       }}); if (success) return ast;\
                      '''

    template = '''\
                ast << _choice([this] () {{
                    AstPtr ast = std::make_shared<Ast>();
                    bool success = false;
                {options}
                    return _error<FailedParse>({error});
                }}); RETURN_IF_EXC(ast);\
               '''


class Closure(_Decorator):
    def render_fields(self, fields):
        fields.update(n=self.counter())

    def render(self, **fields):
        if {()} in self.node.exp.firstset:
            raise CodegenError('may repeat empty sequence')
        return '\n' + super(Closure, self).render(**fields)

    template = '''\
                ast << _closure([this] () {{
                    AstPtr ast = std::make_shared<Ast>();
                {exp:1::}
                    return ast;
                }}); RETURN_IF_EXC(ast);\
                '''


class PositiveClosure(Closure):
    def render_fields(self, fields):
        fields.update(n=self.counter())

    template = '''
                ast << _positive_closure([this] () {{
                    AstPtr ast = std::make_shared<Ast>();
                {exp:1::}
                    return ast;
                }}); RETURN_IF_EXC(ast);\
                '''


class Optional(_Decorator):
    template = '''\
                ast << _optional([this] () {{
                    AstPtr ast = std::make_shared<Ast>();
                {exp:1::}
                    return ast;
                }}); RETURN_IF_EXC(ast);\
               '''

class Cut(Base):
    template = 'ast << _cut();'


class Named(_Decorator):
    def defines(self):
        return [(self.node.name, False)] + super(Named, self).defines()

    def render_fields(self, fields):
        fields.update(n=self.counter(),
                      name=safe_name(self.node.name)
                      )

    template = '''
                (*ast)["{name}"] << [this] () {{
                    AstPtr ast = std::make_shared<Ast>();
                {exp:1::}
                    return ast;
                }}(); RETURN_IF_EXC(ast);\
                '''


# FIXME: Superfluous.  Same as Named.
class NamedList(Named):
    def defines(self):
        return [(self.name, True)] + super(Named, self).defines()

    template = '''
                (*ast)["{name}"] << [this] () {{
                    AstPtr ast = std::make_shared<Ast>();
                {exp:1::}
                    return ast;
                }}(); RETURN_IF_EXC(ast);\
                '''


class Override(Named):
    def defines(self):
        return []


class OverrideList(NamedList):
    def defines(self):
        return []


class Special(Base):
    pass


class RuleRef(Base):
    template = "ast << _{name}_(); RETURN_IF_EXC(ast);"


class RuleInclude(_Decorator):
    def render_fields(self, fields):
        super(RuleInclude, self).render_fields(fields)
        fields.update(exp=self.rend(self.node.rule.exp))

    template = '''
                {exp}
                '''


class Rule(_Decorator):
    def render_fields(self, fields):
        self.reset_counter()

        params = kwparams = ''
        if self.node.params:
            params = ', '.join(repr(
                ustr(self.rend(p))) for p in self.node.params
            )
        if self.node.kwparams:
            kwparams = ', '.join(
                '%s=%s'
                %
                (k, ustr(self.rend(v)))
                for k, v in self.kwparams
            )

        if params and kwparams:
            params = params + ', ' + kwparams
        elif kwparams:
            params = kwparams

        fields.update(params=params)

        defines = compress_seq(self.defines())
        sdefs = [d for d, l in defines if not l]
        ldefs = [d for d, l in defines if l]
        sdefs = set(sdefs)
        ldefs = set(ldefs) - sdefs
        if not (sdefs or ldefs):
            sdefines = 'AstPtr ast = std::make_shared<Ast>();'
        else:
            sdefines = "AstPtr ast = std::make_shared<Ast>\n    (AstMap({\n        "
            elements = ['{ "%s" , AST_DEFAULT }' % d for d in sdefs]
            elements += ['{ "%s" , AST_FORCELIST }' % d for d in ldefs]
            sdefines += ",\n        ".join(elements)
            sdefines += "\n    }));";

        fields.update(defines=sdefines)

    
    # {defines}, {params}
    template = '''
                AstPtr _{name}_()
                {{
                    AstPtr ast = std::make_shared<Ast>();
                    ast << _call("{name}", [this] () {{
                {defines:2::}
                {exp:2::}
                        return ast;
                    }}); RETURN_IF_EXC(ast);
                    return ast;
                }}
                '''


class BasedRule(Rule):
    def defines(self):
        return self.rhs.defines()

    def render_fields(self, fields):
        super(BasedRule, self).render_fields(fields)
        fields.update(exp=self.rhs)


class Grammar(Base):
    def render_fields(self, fields):
        abstract_template = trim(self.abstract_rule_template)
        abstract_rules = [
            abstract_template.format(name=safe_name(rule.name))
            for rule in self.node.rules
        ]
        abstract_rules = indent('\n'.join(abstract_rules))

        if self.node.whitespace is None:
            whitespace = '""'
        else:
            whitespace = cpp_repr(self.node.whitespace)

        if self.node.nameguard is None:
            nameguard = '""'
        else:
            nameguard = cpp_repr(self.node.nameguard)

        rules = '\n'.join([
            self.get_renderer(rule).render() for rule in self.node.rules
        ])

        findruleitems = '\n'.join([
            '{ "%s", &%sParser::_%s_ },' % (rule.name, fields['name'], rule.name)
            for rule in self.node.rules
        ])

        version = str(tuple(int(n) for n in str(timestamp()).split('.')))

        fields.update(rules=indent(rules),
                      findruleitems=indent(findruleitems),
                      abstract_rules=abstract_rules,
                      version=version,
                      whitespace=whitespace,
                      nameguard=nameguard
                      )

    # FIXME.  Clarify interface (avoid copies). 
    abstract_rule_template = '''
            std::pair<AstPtr, state_t> _{name}_ (AstPtr& ast, const state_t & state)
            {{
                return std::pair<AstPtr, state_t>(ast, state);
            }}
            '''

    template = '''\
                /* -*- coding: utf-8 -*-
                   CAVEAT UTILITOR

                   This file was automatically generated by Grako.
                   https://pypi.python.org/pypi/grako/

                   Any changes you make to it will be overwritten the next time
                   the file is generated.
                */

                #include <grakopp/grakopp.hpp>

                // version__ = {version}

                class {name}Parser : public Parser
                {{
                public:
                    {name}Parser(Buffer& buffer, std::string whitespace={whitespace}, std::string nameguard={nameguard})
                        : Parser(buffer, whitespace, nameguard) {{ }}

                    typedef AstPtr ({name}Parser::*rule_method_t) ();
                    rule_method_t find_rule(const std::string& name)
                    {{
                        std::map<std::string, rule_method_t> map({{
                {findruleitems:3::}
                        }});
                        auto el = map.find(name);
                        if (el != map.end())
                            return el->second;
                        return 0;
                    }}

                {rules}
                }};

                class {name}Semantics
                {{
                public:
                {abstract_rules}
                }};

                int
                main(int argc, char *argv[])
                {{
                    Buffer buf;
                    std::string startrule(argv[2]);

                    buf.from_file(argv[1]);
                    {name}Parser parser(buf);

                    try
                    {{
                        {name}Parser::rule_method_t rule = parser.find_rule(startrule);
                        AstPtr ast = (parser.*rule)();
                        std::cout << *ast << "\\n";
                        AstException *exc = boost::get<AstException>(&ast->_content);
                        if (exc)
                            exc->_exc->_throw();
                    }}
                    catch(FailedParseBase& exc)
                    {{
                        std::cout << "Error: " << exc << "\\n";
                    }}

                    return 0;
                }}
               '''
