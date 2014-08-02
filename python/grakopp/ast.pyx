# python/grakopp/cpp/ast.pyx - Grako++ Python bindings -*- coding: utf-8 -*-
# Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
# Written by Marcus Brinkmann <m.brinkmann@semantics.de>
#
# This file is part of Grako++.  Grako++ is free software; you can
# redistribute it and/or modify it under the terms of the 2-clause
# BSD license, see file LICENSE.TXT.

from cython.operator cimport dereference as deref, preincrement as inc

from collections import OrderedDict

from grakopp.exceptions import FailedParse, FailedToken, FailedPattern, FailedLookahead


cdef ast_to_python(Ast& ast):
    if ast.as_none() != NULL:
        return None

    cdef AstString* ast_string = ast.as_string()
    if ast_string != NULL:
        return deref(ast_string)

    cdef AstList* ast_list = ast.as_list()
    cdef list[AstPtr].iterator it
    cdef list[AstPtr].iterator it_end
    if ast_list != NULL:
        val = []
        it = ast_list.begin()
        it_end = ast_list.end()
        while it != it_end:
            val.append(ast_to_python(deref(deref(it))))
            inc(it)
        return val

    cdef AstMap* ast_map = ast.as_map()
    cdef vector[string].iterator keys_it
    cdef vector[string].iterator keys_it_end
    cdef string key
    if ast_map != NULL:
        val = OrderedDict()
        keys_it = ast_map._order.begin()
        keys_it_end = ast_map._order.end()
        while keys_it != keys_it_end:            
            key = deref(keys_it)
            val[key] = ast_to_python(deref(deref(ast_map)[key]))
            inc(keys_it)
        # FIXME: include keys that are not in _order?
        return val

    cdef AstException *ast_exc = ast.as_exception()
    cdef const char *type
    cdef const char *initializer
    if ast_exc != NULL:
        type = deref(ast_exc._exc).type()
        initializer = deref(ast_exc._exc).initializer()
        if type == b"FailedParse":
            return FailedParse(initializer)
        elif type == b"FailedToken":
            return FailedToken(initializer)
        elif type == b"FailedPattern":
            return FailedPattern(initializer)
        elif type == b"FailedLookahead":
            return FailedLookahead(initializer)
        else:
            return FailedParse("unknown exception %s(%s)" % (type, repr(initializer)))

cdef class PyAst:
    """AST for grakopp parser."""

    def __cinit__(self):
        self.ast = make_shared[Ast]()

    def __dealloc__(self):
        pass

    def to_python(self):
        return ast_to_python(deref(self.ast))
