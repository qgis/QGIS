/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_XMDF_HPP
#define MDAL_XMDF_HPP

#include <string>
#include <vector>
#include <memory>
#include <iosfwd>
#include <iostream>
#include <fstream>

#include "mdal_defines.hpp"
#include "mdal.h"
#include "mdal_hdf5.hpp"

namespace MDAL
{

  class LoaderXmdf
  {
    public:
      LoaderXmdf( const std::string &datFile );
      void load( Mesh *mesh, MDAL_Status *status );

    private:
      std::string mDatFile;
      Datasets readXmdfGroupAsDataSet(
        const HdfGroup &rootGroup,
        const std::string &name,
        size_t vertexCount,
        size_t faceCount );

      void addDataSetsFromGroup(
        Datasets &datasets,
        const HdfGroup &rootGroup,
        size_t vertexCount,
        size_t faceCount );
  };

} // namespace MDAL
#endif //MDAL_ASCII_DAT_HPP
