# python/grakopp/cpp/ast.pyx - Grako++ Python bindings -*- coding: utf-8 -*-
# Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
# Written by Marcus Brinkmann <m.brinkmann@semantics.de>
#
# This file is part of Grako++.  Grako++ is free software; you can
# redistribute it and/or modify it under the terms of the 2-clause
# BSD license, see file LICENSE.TXT.

from cython.operator cimport dereference as deref, preincrement as inc
from cpython.ref cimport PyObject, Py_XDECREF, Py_XINCREF
from libcpp.string cimport string

from collections import OrderedDict

from grakopp.exceptions import FailedParse, FailedToken, FailedPattern, FailedLookahead
from grakopp.exceptions import FailedSemantics as GrakoppFailedSemantics
from grako.exceptions import FailedSemantics as GrakoFailedSemantics

# Define an AST extension to store native python objects.

cdef cppclass AstPyObject "AstPyObject" (AstExtensionType):
    PyObject* _object

    __init__(PyObject* obj) with gil:
        Py_XINCREF(obj)
        this._object = obj

    __dealloc__() with gil:
        Py_XDECREF(this._object)

    string output() nogil const:
        with gil:
            return repr(<object> this._object)

ctypedef shared_ptr[AstPyObject] AstPyObjectPtr

cdef extern from "<memory>" namespace "std":
    cdef cppclass AstPyObject
    cdef AstPyObjectPtr make_shared_AstPyObject "std::make_shared<AstPyObject>" (PyObject*) nogil

cdef extern from "<memory>" namespace "std":
    AstExtension dynamic_pointer_cast_ast_extension "std::dynamic_pointer_cast<AstExtensionType>" (AstPyObjectPtr)
    AstPyObjectPtr dynamic_pointer_cast_ast_py_object "std::dynamic_pointer_cast<AstPyObject>" (AstExtension)
    shared_ptr[FailedParseBase] dynamic_pointer_cast_FailedParseBase "std::dynamic_pointer_cast<FailedParseBase>" (shared_ptr[FailedSemantics])

class PyAstExtension:
    # For unknown extension objects, just capture the output.

    def __init__(self, str):
        self._repr = str

    def __repr__(self):
        return "%s(%s)" % (self.__class__.__name__, self._repr)


cdef AstPtr python_to_ast(PyObject* obj):
    """Store native Python object into an AST object and return a
    pointer."""
    cdef AstExtension ast_ext
    ast_ext = dynamic_pointer_cast_ast_extension(make_shared_AstPyObject(obj))
    cdef AstPtr new_ast
    new_ast = make_shared[Ast]()
    deref(new_ast).set(ast_ext)
    return new_ast

cdef AstPtr exc_to_ast(PyObject* _exc):
    cdef object exc = <object> _exc
    """Store Python exception into an AST object and return a pointer."""
    cdef AstException ast_exc

    if isinstance(exc, GrakoFailedSemantics):
        ast_exc._exc = dynamic_pointer_cast_FailedParseBase(make_shared_FailedSemantics(exc.args[0]))
    else:
        ast_exc._exc = dynamic_pointer_cast_FailedParseBase(make_shared_FailedSemantics("Unknown runtime exception"))

    cdef AstPtr new_ast
    new_ast = make_shared[Ast]()
    deref(new_ast).set(ast_exc)
    return new_ast

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
        elif type == b"FailedSemantics":
            return GrakoppFailedSemantics(initializer)
        else:
            return FailedParse("unknown exception %s(%s)" % (type, repr(initializer)))

    cdef AstExtension *ast_ext = ast.as_extension()
    cdef AstExtension ext_obj
    cdef AstPyObjectPtr py_obj
    cdef string ext_repr
    if ast_ext != NULL:
        ext_obj = deref(ast_ext)
        py_obj = dynamic_pointer_cast_ast_py_object(ext_obj)
        if <bool> py_obj:
            return <object> deref(py_obj)._object
        else:
            # For unknown extension objects, just capture the output.
            ext_repr = deref(ext_obj).output()
            return PyAstExtension(<bytes> ext_repr)
    return None

cdef class PyAst:
    """AST for grakopp parser."""

    def __cinit__(self):
        self.ast = make_shared[Ast]()

    def __dealloc__(self):
        pass

    def to_python(self):
        return ast_to_python(deref(self.ast))
