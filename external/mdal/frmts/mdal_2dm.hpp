/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_2DM_HPP
#define MDAL_2DM_HPP

#include <string>

#include "mdal_defines.hpp"
#include "mdal.h"

namespace MDAL
{

  class Loader2dm
  {
    public:
      Loader2dm( const std::string &meshFile );
      Mesh *load( Status *status );

    private:
      std::string mMeshFile;
  };

} // namespace MDAL
#endif //MDAL_2DM_HPP
