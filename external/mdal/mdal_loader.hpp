/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_LOADER_HPP
#define MDAL_LOADER_HPP

#include <string>

#include "mdal.h"
#include "mdal_defines.hpp"

namespace MDAL
{

  class Loader
  {
    public:
      static Mesh *load( const std::string &meshFile, Status *status );
  };

} // namespace MDAL
#endif //MDAL_LOADER_HPP
