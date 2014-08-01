# python/grakopp/cpp/buffer.pxd - Grako++ Python bindings -*- coding: utf-8 -*-
# Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
# Written by Marcus Brinkmann <m.brinkmann@semantics.de>
#
# This file is part of Grako++.  Grako++ is free software; you can
# redistribute it and/or modify it under the terms of the 2-clause
# BSD license, see file LICENSE.TXT.

from libcpp.string cimport string
from libcpp cimport bool


# C++ types

cdef extern from "<memory>" namespace "std":
    cdef cppclass shared_ptr[T]:
        T& operator*() nogil

    # Oh well.  Seems that returning templatized types doesn't work.
    cdef cppclass BufferPtr
    cdef BufferPtr make_shared[Buffer]() nogil


cdef extern from "grakopp/buffer.hpp":
    ctypedef shared_ptr[Buffer] BufferPtr

    cdef cppclass Buffer:
        size_t _pos
        void from_string(const string& text) nogil
        void from_file(const string& filename) nogil
        size_t len() nogil
        bool atend() nogil
        bool ateol() nogil
        char current() nogil
        char at(size_t pos) nogil
        char peek(size_t off) nogil
        char next() nogil
        void go_to(size_t pos) nogil
        void move(size_t off) nogil
        void next_token() nogil
        size_t skip_to(char ch) nogil
        size_t skip_past(char ch) nogil
        size_t skip_to_eol() nogil
        bool is_space() nogil
        bool is_name_char(size_t pos) nogil
        bool match(string token) nogil
        # boost::optional<std::string> matchre(std::string pattern) nogil


# Extension types

cdef class PyBuffer(object):
    cdef BufferPtr buffer
