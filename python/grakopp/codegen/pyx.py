# python/grakopp/codegen.py - Grako++ code generator backend for grako -*- coding: utf-8 -*-
# Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
# Written by Marcus Brinkmann <m.brinkmann@semantics.de>
#
# This file is part of Grako++.  Grako++ is free software; you can
# redistribute it and/or modify it under the terms of the 2-clause
# BSD license, see file LICENSE.TXT.

# The Python parts in this file is based on Grako's code generators.

from __future__ import (absolute_import, division, print_function,
                        unicode_literals)

"""
C++ code generation for models defined with grako.model
"""

from grako.util import indent, trim, timestamp
from grako.exceptions import CodegenError
from grako.model import Node
from grako.codegen.cgbase import ModelRenderer, CodeGenerator

class PyxCodeGenerator(CodeGenerator):
    def _find_renderer_class(self, item):
        if not isinstance(item, Node):
            return None

        name = item.__class__.__name__
        renderer = globals().get(name, None)
        if not renderer or not issubclass(renderer, ModelRenderer):
            raise CodegenError('Renderer for %s not found' % name)
        return renderer


def codegen(model):
    return PyxCodeGenerator().render(model)


class Grammar(ModelRenderer):
    def render_fields(self, fields):
        abstract_template = trim(self.abstract_rule_template)
        abstract_rules = [
            abstract_template.format(parsername=fields['name'], name=rule.name)
            for rule in self.node.rules
        ]
        abstract_rules = indent('\n'.join(abstract_rules))

        abstract_py_template = trim(self.abstract_rule_py_template)
        abstract_rules_py = [
            abstract_py_template.format(parsername=fields['name'], name=rule.name)
            for rule in self.node.rules
        ]
        abstract_rules_py = indent('\n'.join(abstract_rules_py))

        rule_template = trim(self.rule_template)
        rules = [
            rule_template.format(parsername=fields['name'], name=rule.name)
            for rule in self.node.rules
        ]
        rules = '\n'.join(rules)

        if self.node.statetype is not None:
            statetype_arg = ", " + self.node.statetype
        else:
            statetype_arg = ""

        version = str(tuple(int(n) for n in str(timestamp()).split('.')))

        fields.update(rules=indent(rules),
                      abstract_rules=abstract_rules,
                      abstract_rules_py=abstract_rules_py,
                      version=version,
                      statetype_arg=statetype_arg
                      )

    abstract_rule_template = '''
            AstPtr _{name}_(AstPtr& ast) nogil:
                return wrapped_call("_{name}_", ast)
            '''

    abstract_rule_py_template = '''
            def _{name}_(self, ast):
                print "_{name}_", ast
                return ast
            '''

    rule_template = '''
        def _{name}_(self):
            ast = PyAst()
            ast.ast = deref(self.parser)._{name}_()
            return ast
        '''

    template = '''\
                # -*- coding: utf-8 -*-
                # CAVEAT UTILITOR
                #
                # This file was automatically generated by Grako++.
                # https://pypi.python.org/pypi/grakopp/
                #
                # Any changes you make to it will be overwritten the next time
                # the file is generated.
                
                # Version: {version}

                from cython.operator cimport dereference as deref

                from grakopp.buffer cimport PyBuffer
                from grakopp.parser cimport Parser
                from grakopp.ast cimport Ast, AstPtr, PyAst, python_to_ast

                from cpython.ref cimport PyObject, Py_XINCREF, Py_XDECREF

                cdef cppclass {name}WrappedSemantics({name}Semantics):
                    PyObject* _semantics

                    __init__():
                        this._semantics = NULL

                    void set_semantics(semantics) with gil:
                        if this._semantics != NULL:
                            Py_XDECREF(this._semantics)
                            if semantics is None:
                                this._semantics = NULL
                                return
                        this._semantics = <PyObject*> semantics
                        Py_XINCREF(this._semantics)

                    AstPtr wrapped_call(const char* rule, AstPtr& ast) with gil:
                        if this._semantics == NULL:
                            return ast

                        func = getattr(<object>this._semantics, rule, None)
                        if func == None:
                            return ast

                        pyast = PyAst()
                        pyast.ast = ast
                        # We could also work with PyAst objects...
                        obj = pyast.to_python()
                        obj = func(obj)

                        # Dance with Cython to return an AstPtr.
                        return python_to_ast(<PyObject*> obj)

                {abstract_rules}


                cdef class {name}PyParser(object):
                    """Parser for Grakopp grammar '{name}'."""

                    cdef {name}Parser* parser
                    # TODO: Support custom C++ semantics
                    cdef basicWrappedSemantics* semantics

                    def __cinit__(self):
                        self.parser = new {name}Parser()
                        # TODO: Support custom C++ semantics
                        self.semantics = new basicWrappedSemantics()

                    def __dealloc__(self):
                        del self.parser
                        del self.semantics

                    def set_semantics(self, semantics):
                        deref(self.semantics).set_semantics(semantics)
                        deref(self.parser)._semantics = self.semantics

                    # Because Cython does not supported templated extension types,
                    # we can't use inheritance but have to put all members here.

                    def set_buffer(self, PyBuffer buffer):
                        deref(self.parser).set_buffer(buffer.buffer)

                    def set_whitespace(self, whitespace):
                        deref(self.parser).set_whitespace(whitespace)

                    def set_nameguard(self, nameguard):
                        deref(self.parser).set_nameguard(nameguard)

                    def reset(self):
                        deref(self.parser).reset()

                    # typedef AstPtr (nameParser::*rule_method_t) ();
                    # rule_method_t find_rule(const std::string& name);

                {rules}


                class basicPySemantics(object):
                {abstract_rules_py}

               '''
