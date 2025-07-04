/*
===============================================================================

  FILE:  excepts.hpp
  
  CONTENTS:
    Exception types

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.
  
  COPYRIGHT:
  
    (c) 2007-2014, martin isenburg, rapidlasso - tools to catch reality
    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
===============================================================================
*/

#ifndef __excepts_hpp__
#define __excepts_hpp__

#include <stdexcept>

namespace lazperf
{

struct error : public std::runtime_error
{
    error(const std::string& what) : std::runtime_error(what)
    {}
};

} // namespace lazperf

#endif // __excepts_hpp__
