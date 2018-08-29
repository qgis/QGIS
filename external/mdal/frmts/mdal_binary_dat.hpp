/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_BINARY_DAT_HPP
#define MDAL_BINARY_DAT_HPP

#include <string>
#include <vector>
#include <memory>
#include <iosfwd>
#include <iostream>
#include <fstream>

#include "mdal_data_model.hpp"
#include "mdal.h"

namespace MDAL
{

  class LoaderBinaryDat
  {
    public:
      LoaderBinaryDat( const std::string &datFile );
      void load( Mesh *mesh, MDAL_Status *status );

    private:
      bool readVertexTimestep( const Mesh *mesh,
                               std::shared_ptr<DatasetGroup> group,
                               std::shared_ptr<DatasetGroup> groupMax,
                               double time,
                               bool hasStatus,
                               int sflg,
                               std::ifstream &in );

      std::string mDatFile;
  };

} // namespace MDAL
#endif //MDAL_BINARY_DAT_HPP
