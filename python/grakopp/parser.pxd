# python/grakopp/cpp/parser.pxd - Grako++ Python bindings -*- coding: utf-8 -*-
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

from grakopp.buffer cimport BufferPtr
from grakopp.ast cimport AstPtr

cdef extern from "grakopp/parser.hpp":
    cdef cppclass Parser[semantics, state]:
        void set_buffer(const BufferPtr& buffer) nogil
        void set_whitespace(const string& whitespace) nogil
        void set_nameguard(bool nameguard) nogil
        void reset() nogil
        # AstPtr _error[T](string msg)
        # AstPtr _call(string name, semantics_func_t sem_func, function<AstPtr ()> func)
        # AstPtr _fail()
        # AstPtr _check_eof()
        # AstPtr _cut()
        # AstPtr _token(const string& token)
        # AstPtr _pattern(const string& pattern)
        # AstPtr _try(function<AstPtr ()> func)
        # AstPtr _option(bool &success, function<AstPtr ()> func)
        # AstPtr _choice(function<AstPtr ()> func)
        # AstPtr _optional(function<AstPtr ()> func)
        # AstPtr _group(function<AstPtr ()> func)
        # AstPtr _if(function<AstPtr ()> func)
        # AstPtr _ifnot(function<AstPtr ()> func)
        # AstPtr _closure(function<AstPtr ()> func)
        # AstPtr _positive_closure(function<AstPtr ()> func)
