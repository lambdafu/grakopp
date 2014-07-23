# CMakeLists.txt - Grako++ CMake build file
# Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
# Written by Marcus Brinkmann <m.brinkmann@semantics.de>
#
# This file is part of Grako++.  Grako++ is free software; you can
# redistribute it and/or modify it under the terms of the 2-clause
# BSD license, see file LICENSE.TXT.

cmake_minimum_required(VERSION 3.0)
project(Grako++)

if(CMAKE_COMPILER_IS_GNUCXX)
    # Replace this with constructs like (see target_compile_options).
    # target_compile_features(libgrakopp INTERFACE cxx_generic_lambda)
    set(GCC_STD_OPTION -std=c++11)

    # See grakopp/buffer.hpp.
    if(CMAKE_C_COMPILER_VERSION VERSION_LESS "4.9")
      find_package(Boost REQUIRED COMPONENTS regex)
    endif()
endif()

if (CMAKE_CXX_COMPILER MATCHES ".*clang")
  set(CMAKE_COMPILER_IS_CLANGXX 1)
endif ()
if(CMAKE_COMPILER_IS_CLANGXX)
    set(GCC_STD_OPTION -std=c++11)
    set(CLANG_STDLIB_OPTION -stdlib=libc++)
endif()


# Header-only interface library.
add_library(libgrakopp INTERFACE)
target_include_directories(libgrakopp INTERFACE ${Boost_INCLUDE_DIRS}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_options(libgrakopp INTERFACE ${GCC_STD_OPTION} ${CLANG_STDLIB_OPTION})
target_link_libraries(libgrakopp INTERFACE ${Boost_REGEX_LIBRARY} ${CLANG_STDLIB_OPTION})

install(TARGETS libgrakopp EXPORT libgrakoppExport)
install(EXPORT libgrakoppExport NAMESPACE Upstream::
  DESTINATION lib/cmake/libgrakopp)
install(FILES
  include/grakopp/ast.hpp
  include/grakopp/buffer.hpp
  include/grakopp/exceptions.hpp
  include/grakopp/grakopp.hpp
  include/grakopp/parser.hpp
  DESTINATION include/grakopp)

# Small test program.
add_executable(grakopp-demo grakopp-demo.cpp)
target_include_directories(grakopp-demo PRIVATE libgrakopp)
target_link_libraries(grakopp-demo libgrakopp)

enable_testing()
add_subdirectory(tests)