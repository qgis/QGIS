/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_ASCII_DAT_HPP
#define MDAL_ASCII_DAT_HPP

#include <string>
#include <vector>
#include <memory>
#include <iosfwd>
#include <iostream>
#include <fstream>

#include "mdal_defines.hpp"
#include "mdal.h"

namespace MDAL
{

  class LoaderAsciiDat
  {
    public:
      LoaderAsciiDat( const std::string &datFile );
      void load( Mesh *mesh, MDAL_Status *status );

    private:
      void readVertexTimestep(
        const Mesh *mesh,
        std::vector<std::shared_ptr<Dataset>> &datOutputs,
        double t,
        bool isVector,
        bool hasStatus,
        std::ifstream &stream );

      void readFaceTimestep(
        const Mesh *mesh,
        std::vector<std::shared_ptr<Dataset>> &datOutputs,
        double t,
        bool isVector,
        std::ifstream &stream );

      void addDatasets( MDAL::Mesh *mesh,
                        const std::string &name,
                        const std::vector<std::shared_ptr<Dataset>> &datOutputs ) const;

      std::string mDatFile;
  };

} // namespace MDAL
#endif //MDAL_ASCII_DAT_HPP
