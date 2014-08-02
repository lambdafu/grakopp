# python/grakopp/cpp/exceptions.py - Grako++ Python bindings -*- coding: utf-8 -*-
# Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
# Written by Marcus Brinkmann <m.brinkmann@semantics.de>
#
# This file is part of Grako++.  Grako++ is free software; you can
# redistribute it and/or modify it under the terms of the 2-clause
# BSD license, see file LICENSE.TXT.

class GrakoException(Exception):
    def __init__(self, msg):
        super(GrakoException, self).__init__(msg)


class ParseError(GrakoException):
    def __init__(self, msg):
        super(ParseError, self).__init__(msg)


class FailedParseBase(ParseError):
    def __init__(self, msg):
        super(FailedParseBase, self).__init__(msg)
        self.msg = msg

    def __repr__(self):
        return "%s(%s)" % (self.__class__.__name__, repr(self.msg))

    @property
    def message(self):
        return self.msg


class FailedParse(FailedParseBase):
    def __init__(self, msg):
        super(FailedParse, self).__init__(msg)


class FailedToken(FailedParseBase):
    def __init__(self, msg):
        super(FailedToken, self).__init__(msg)

    @property
    def message(self):
        return "expecting %s" % repr(self.msg).lstrip('u')


class FailedPattern(FailedParseBase):
    def __init__(self, msg):
        super(FailedPattern, self).__init__(msg)

    @property
    def message(self):
        return "expecting %s" % repr(self.msg).lstrip('u')


class FailedLookahead(FailedParseBase):
    def __init__(self, msg):
        super(FailedLookahead, self).__init__(msg)

    @property
    def message(self):
        return 'failed lookahead'
