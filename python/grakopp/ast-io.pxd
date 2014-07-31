# python/grakopp/cpp/ast-io.pyx - Grako++ Python bindings -*- coding: utf-8 -*-
# Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
# Written by Marcus Brinkmann <m.brinkmann@semantics.de>
#
# This file is part of Grako++.  Grako++ is free software; you can
# redistribute it and/or modify it under the terms of the 2-clause
# BSD license, see file LICENSE.TXT.

cdef extern from "grakopp/ast-io.hpp":
    pass

cdef extern from "<iostream>" namespace "std":
    # FIXME: This won't work if there is a conflicting declaration of
    # class ostream.
    cdef cppclass ostream:
        ostream& operator<< (const _Ast& val)
        ostream& operator<< (const string& val)
        ostream& operator<< (const void*& val)
    ostream cout
