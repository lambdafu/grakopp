/* grakopp/grakopp.hpp - Grako++ main header file
   Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
   Written by Marcus Brinkmann <m.brinkmann@semantics.de>

   This file is part of Grako++.  Grako++ is free software; you can
   redistribute it and/or modify it under the terms of the 2-clause
   BSD license, see file LICENSE.TXT.
*/

#ifndef _GRAKOPP_GRAKOPP_HPP
#define _GRAKOPP_GRAKOPP_HPP 1

#include "buffer.hpp"
#include "exceptions.hpp"
#include "ast.hpp"
#include "parser.hpp"

#define RETURN_IF_EXC(ast) if (ast->as_exception()) return ast

#endif /* GRAKOPP_GRAKOPP_HPP */
