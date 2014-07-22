/* grakopp/grakopp.hpp - Grako++ main header file
   Copyright (C) 2014 semantics Kommunikationsmanagement GmbH
   Written by Marcus Brinkmann <m.brinkmann@semantics.de>

   This file is part of Grako++.  Grako++ is free software; you can
   redistribute it and/or modify it under the terms of the 2-clause
   BSD license, see file LICENSE.TXT.
*/

#ifndef _GRAKOPP_GRAKOPP_HPP
#define _GRAKOPP_GRAKOPP_HPP 1

#include <functional>
#include <stack>
#include <map>
#include <list>
#include <memory>
#include <iostream>
#include <assert.h>

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/algorithm/string/replace.hpp>


#include "buffer.hpp"
#include "exceptions.hpp"
#include "ast.hpp"
#include "parser.hpp"

#define RETURN_IF_EXC(ast) if (boost::get<AstException>(&ast->_content)) return ast

#endif /* GRAKOPP_GRAKOPP_HPP */
