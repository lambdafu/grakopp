# python/grakopp/cpp/ast.pyx - Grako++ Python bindings -*- coding: utf-8 -*-
# Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
# Written by Marcus Brinkmann <m.brinkmann@semantics.de>
#
# This file is part of Grako++.  Grako++ is free software; you can
# redistribute it and/or modify it under the terms of the 2-clause
# BSD license, see file LICENSE.TXT.

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.list cimport list
from libcpp.map cimport map
from libcpp cimport bool


# C++ types

cdef extern from "<memory>" namespace "std":
    cdef cppclass shared_ptr[T]:
        T& operator*() nogil

    # Oh well.  Seems that returning templatized types doesn't work.
    cdef cppclass AstPtr
    cdef AstPtr make_shared[Ast](Ast& ast) nogil

cdef extern from "grakopp/exceptions.hpp":
    cdef cppclass FailedParseBase:
        const char* type() nogil
        const char* what() nogil
        const char* initializer() nogil

cdef extern from "grakopp/ast.hpp":
    ctypedef shared_ptr[Ast] AstPtr

    cdef cppclass AstNone:
        pass

    ctypedef string AstString

    cdef cppclass AstList (list[AstPtr]):
        bool _mergeable
    
    cdef cppclass AstMap (map[string, AstPtr]):
        vector[string] _order

    cdef cppclass AstException:
        shared_ptr[FailedParseBase] _exc

    cdef cppclass Ast:
        void set(const AstNone&) nogil
        void set(const AstString&) nogil
        void set(const AstList&) nogil
        void set(const AstMap&) nogil
        void set(const AstException&) nogil

        AstNone* as_none() nogil
        AstString* as_string() nogil
        AstList* as_list() nogil
        AstMap* as_map() nogil
        AstException* as_exception() nogil


# Extension types

cdef class PyAst:
    cdef AstPtr ast
