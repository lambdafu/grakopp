**Work in progress!**

[![Build Status](https://travis-ci.org/lambdafu/grakopp.png)](https://travis-ci.org/lambdafu/grakopp)

Grako++
=======

Grako++ is a packrat parser runtime for Grako, written in C++.  The
aim is to provide a Grako-compatible parser generator, but with
improved performance over Python.


Differences to Grako
--------------------

* The output is always AST/JSON.
* The semantics class has to be implemented in C++ as well.
* The regular expression syntax is [ECMAScript](http://www.cplusplus.com/reference/regex/ECMAScript/), not Python.


Build
-----

The library is header-files only currently, so you can just copy the
files in include/ to a convenient location, or build from there.  More
formally, you can use cmake.

Build with clang by invoking: cmake -DCMAKE_CXX_COMPILER=/path/to/clang++

Building with g++ < 4.9 requires linking with the boost_regex library,
because the std::regex implementation is broken.  You can rely on the
exported cmake library file to configure the right settings.

Other useful option: -DCMAKE_INSTALL_PREFIX:PATH=/path/to/install


TODO
----

* nameguard, whitespace
* left recursion
* stateful parsing
* tests
* python integration (a la pyximport)
* regex syntax details
* XML output


Authors
-------

Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
Written by Marcus Brinkmann <m.brinkmann@semantics.de>
See LICENSE.txt for details.
