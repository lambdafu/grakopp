# python/grakopp/cpp/buffer.pyx - Grako++ Python bindings -*- coding: utf-8 -*-
# Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
# Written by Marcus Brinkmann <m.brinkmann@semantics.de>
#
# This file is part of Grako++.  Grako++ is free software; you can
# redistribute it and/or modify it under the terms of the 2-clause
# BSD license, see file LICENSE.TXT.

from cython.operator cimport dereference as deref


cdef class PyBuffer(object):
    """Buffer for grakopp parser."""

    # See buffer.pxd for C members.
    def __cinit__(self):
        self.buffer = make_shared[Buffer]()

    def __dealloc__(self):
        pass

    property pos:
        def __get__(self):
            return deref(self.buffer)._pos

        def __set__(self, pos):
            deref(self.buffer).go_to(pos)

    def from_string(self, str):
        deref(self.buffer).from_string(str)

    def from_file(self, str):
        deref(self.buffer).from_file(str)

    def len(self):
        return deref(self.buffer).len()

    def atend(self):
        return deref(self.buffer).atend()

    def ateol(self):
        return deref(self.buffer).ateol()

    def current(self):
        return deref(self.buffer).current()

    def at(self, pos):
        return deref(self.buffer).at(pos)

    def at(self, off):
        return deref(self.buffer).peek(off)

    def next(self):
        return deref(self.buffer).next()

    def go_to(self, pos):
        deref(self.buffer).go_to(pos)

    def move(self, off):
        deref(self.buffer).move(off)

    def next_token(self):
        deref(self.buffer).next_token()

    def skip_to(self, ch):
        return deref(self.buffer).skip_to(ch)

    def skip_past(self, ch):
        return deref(self.buffer).skip_past(ch)

    def skip_to_eol(self):
        return deref(self.buffer).skip_to_eol()

    def is_space(self):
        return deref(self.buffer).is_space()

    def is_name_char(self, pos):
        return deref(self.buffer).is_name_char(pos)

    def match(self, token):
        return deref(self.buffer).match(token)

    # boost::optional<std::string> matchre(std::string pattern) nogil
