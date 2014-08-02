#!/usr/bin/env python

try:
    import setuptools
except ImportError:
    from distutils.core import setup
else:
    from setuptools import setup

from distutils.core import Extension
from Cython.Build import cythonize

import os
topdir = os.path.dirname(os.path.abspath(__file__))

buffer_ext = Extension("grakopp.buffer", ["python/grakopp/buffer.pyx"], include_dirs=[topdir + "/include"],
                    language = "c++",
                    extra_compile_args = ["-std=c++11"])
ast_ext = Extension("grakopp.ast", ["python/grakopp/ast.pyx"], include_dirs=[topdir + "/include"],
                    language = "c++",
                    extra_compile_args = ["-std=c++11"])

setup(name="grakopp",
      version="0.1.0",
      description="Grako++ is a packrat parser runtime for Grako, written in C++.",

      author="Marcus Brinkmann",
      author_email="m.brinkmann@semantics.de",
      url="https://github.com/lambdafu/grakopp",
      keywords="grako parser peg",
      long_description=open("README.rst").read(),
      license="BSD",

      zip_safe=False,
      requires=["grako", "cython"],
      entry_points={
          "console_scripts": [
              "grakopp = grakopp.tool:main",
          ]
      },

      package_dir={"": "python"},
      packages=["grakopp", "grakopp.codegen"],
      package_data = { "grakopp" : ["*.pxd", "*.pyx"] },
      ext_modules = cythonize([ast_ext, buffer_ext]),

      classifiers=[
          "Development Status :: 3 - Alpha",
          "Environment :: Console",
          "Intended Audience :: Developers",
          "Intended Audience :: Science/Research",
          "License :: OSI Approved :: BSD License",
          "Operating System :: OS Independent",
          "Natural Language :: English",
          "Operating System :: OS Independent",
          "Programming Language :: C++",
          "Programming Language :: Python :: 2.7",
          "Programming Language :: Python :: 3.4",
          "Topic :: Software Development :: Code Generators",
          "Topic :: Software Development :: Compilers",
          "Topic :: Software Development :: Interpreters",
          "Topic :: Text Processing :: General"
          ]
      )
