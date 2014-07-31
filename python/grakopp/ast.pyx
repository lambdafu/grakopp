# distutils: language = c++
# -*- coding: utf-8 -*-

# cython --cplus ast.pyx
# g++ -std=c++11 -I../../include -shared -pthread -fPIC -fwrapv -O2 -Wall -fno-strict-aliasing -I/usr/include/python2.7 -o ast.so ast.cpp 

from __future__ import absolute_import

from cython.operator cimport dereference as deref, preincrement as inc

from collections import OrderedDict

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

cdef class PyAst:
    cdef Ast* thisptr
    def __cinit__(self):
        self.thisptr = new Ast()
    def __dealloc__(self):
        del self.thisptr
    def to_python(self):
        return ast_to_python(deref(self.thisptr))
