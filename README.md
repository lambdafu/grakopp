**Work in progress!**

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

TODO
----

* nameguard, whitespace
* left recursion
* stateful parsing
* tests
* cython integration
* regex syntax details
* XML output

Authors
-------

Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
Written by Marcus Brinkmann <m.brinkmann@semantics.de>
See LICENSE.txt for details.
