**Work in progress!**

|Build Status|

Grako++
=======

Grako++ is a packrat parser runtime for Grako, written in C++. The aim
is to provide a Grako-compatible parser generator, but with improved
performance over Python.

Differences to Grako
--------------------

* The output is always AST/JSON.
* The semantics class can be implemented in C++ or Cython (pure Python
  through Cython is planned).
* The regular expression syntax is
  `ECMAScript <http://www.cplusplus.com/reference/regex/ECMAScript/>`__,
  not Python.
* Some features of Grako are missing, see below in the TODO section.
* State types must implement operator< for std::map (for future
  flexibility, consider implementing the hash trait, too).

Build
-----

The library is header-files only currently, so you can just copy the
files in include/ to a convenient location, or build from there. More
formally, you can use cmake.

Build with clang by invoking: cmake -DCMAKE\_CXX\_COMPILER=/path/to/clang++

Building with g++ < 4.9 requires linking with the boost\_regex library,
because the std::regex implementation is broken. You can rely on the
exported cmake library file to configure the right settings.

Other useful option: -DCMAKE\_INSTALL\_PREFIX:PATH=/path/to/install

Usage
-----

The grakopp program is used like grako to compile PEG files to source
code. There are four different source code output formats that can be
specified with the -f/--format option:

+------------+-------------+-------------------------+
| --format   | --output    | Purpose                 |
+============+=============+=========================+
| hpp        | name.hpp    | C++ declaration         |
+------------+-------------+-------------------------+
| cpp        |  name.cpp   | C++ implementation      |
+------------+-------------+-------------------------+
| pxd        | name.pxd    | Cython declaration      |
+------------+-------------+-------------------------+
| pyx        | name.pyx    | Cython implementation   |
+------------+-------------+-------------------------+

For pure C++ parsers, generating the hpp and cpp files is sufficient.
For Python integration, the pxd and pyx files are also needed. For
"name" you should subsitute the actual name of the parser (either the
base name of the PEG file or the argument to the --name option). The
filenames are, for now, hard-coded into the source files (the underscore
protects the C++ implementation from Cython-generated source files).

Here is an example how to build a parser:

.. code:: sh

    $ ./grakopp -f hpp -o _basic.hpp tests/basic/basic.peg
    $ ./grakopp --whitespace="" --no-nameguard -f cpp -o _basic.cpp tests/basic/basic.peg
    $ g++ -DGRAKOPP_MAIN -std=c++11 -Iinclude -O4 -o basic _basic.cpp -lboost_regex

You can invoke the parser like a Python parser generated with grako
(currently, no options are supported, though, except for an internal
option --test that compares the AST with a text fixture file):

.. code:: sh

    $ echo -n e1e2 | ./basic /dev/stdin sequence
    [
        "e1",
        "e2"
    ]

C++ Interface
-------------

The following header files exist:

+-----------------------+---------------------------+
| Header                | Purpose                   |
+=======================+===========================+
| grakopp/exception.hpp | Parser exceptions         |
+-----------------------+---------------------------+
| grakopp/buffer.hpp    | Buffer for I/O            |
+-----------------------+---------------------------+
| grakopp/ast.hpp       | AST implementation        |
+-----------------------+---------------------------+
| grakopp/parser.hpp    | Parser base class         |
+-----------------------+---------------------------+
| grakopp/grakopp/hpp   | Include all above         |
+-----------------------+---------------------------+
| grakopp/ast-io.hpp    | Optional AST stream I/O   |
+-----------------------+---------------------------+

Python Integration
------------------

Until the Python package is prepared properly, build the Cython
extensions manually, for example like this:

.. code:: sh

    $ cd python/grakopp
    $ cython --cplus buffer.pyx
    $ cython --cplus ast.pyx
    $ g++ -std=c++11 -I../../include -shared -pthread -fPIC -fwrapv -O2 -Wall -fno-strict-aliasing -I/usr/include/python2.7 -o buffer.so buffer.cpp 
    $ g++ -std=c++11 -I../../include -shared -pthread -fPIC -fwrapv -O2 -Wall -fno-strict-aliasing -I/usr/include/python2.7 -o ast.so ast.cpp 
    $ cd ../..

To continue the above example:

.. code:: sh

    $ ./grakopp -f pxd -o basic.pxd tests/basic/basic.peg
    $ ./grakopp -f pyx -o basic.pyx tests/basic/basic.peg
    $ cython -Ipython --cplus basic.pyx
    $ g++ -std=c++11 -Iinclude -shared -pthread -fPIC -fwrapv -O2 -Wall -fno-strict-aliasing -I/usr/include/python2.7 -o basic.so basic.cpp _basic.cpp -l boost_regex

You can then use it from Python:

::

    $ PYTHONPATH=python python
    >>> from grakopp import buffer
    >>> b = buffer.PyBuffer()
    >>> b.from_string("e1e2")
    >>> import basic
    >>> p = basic.basicPyParser()
    >>> p.set_buffer(b)
    >>> a = p._sequence_()
    >>> a.to_python()
    ['e1', 'e2']
    >>> b.pos
    4
    >>> a = p._sequence_()
    >>> a.to_python()
    FailedToken('e1')

TODO
----

* dynamic Ast objects (so you can pass through Python or XML objects)
* python/distutils integration
* automatic compilation a la pyximport
* add namespace
* unicode support?
* more support and tests for stateful parsing
* regex syntax tests (make sure generated C strings are always proper)
* profile and optimize

Grako features missing:

* ignorecase (buffer match, matchre)
* comments skipping
* buffer line parsing and trace output (also in exceptions)
* ParseInfo
* rules with arguments
* left recursion
* semantic action "\_default"

Authors
-------

::

    Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
    Written by Marcus Brinkmann <m.brinkmann@semantics.de>
    See LICENSE.txt for details.

.. |Build Status| image:: https://travis-ci.org/lambdafu/grakopp.png
   :target: https://travis-ci.org/lambdafu/grakopp
