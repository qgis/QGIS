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

#include "mdal_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"

namespace MDAL
{

  /**
   * ASCII Dat format is used by various solvers and the output
   * from various solvers can have slightly different header.
   * The format is used by TUFLOW, BASEMENT and HYDRO_AS-2D solvers.
   *
   * The most frequent form is based on official SMS documentation
   * https://www.xmswiki.com/wiki/SMS:ASCII_Dataset_Files_*.dat
   * The official format only supports data defined on vertices,
   * both scalar vector. The new format is recognized by keyword
   * "DATASET" on the first line of the file.
   *
   * BASEMENT solver also stores data defined on faces. To recognize
   * such dataset, the dataset name contains "_els_" substring
   * (e.g. depth_els_1.dat)
   *
   * In one file, there is always one dataset group stored.
   *
   * Sometime the "older" datasets may have some part of the
   * header missing, e.g. the file starts with SCALAR or VECTOR or TS
   * keyword. The older format does not have "active" flags for faces
   * and does not recognize most of the keywords. Old format data
   * are always defined on vertices
   */
  class DriverAsciiDat: public Driver
  {
    public:
      DriverAsciiDat();
      ~DriverAsciiDat( ) override;
      DriverAsciiDat *create() override;

      bool canRead( const std::string &uri ) override;
      void load( const std::string &datFile, Mesh *mesh, MDAL_Status *status ) override;

    private:
      bool canReadOldFormat( const std::string &line ) const;
      bool canReadNewFormat( const std::string &line ) const;

      void loadOldFormat( std::ifstream &in, Mesh *mesh, MDAL_Status *status ) const;
      void loadNewFormat( std::ifstream &in, Mesh *mesh, MDAL_Status *status ) const;

      void readVertexTimestep(
        const Mesh *mesh,
        std::shared_ptr<DatasetGroup> group,
        double t,
        bool isVector,
        bool hasStatus,
        std::ifstream &stream ) const;

      void readFaceTimestep(
        const Mesh *mesh,
        std::shared_ptr<DatasetGroup> group,
        double t,
        bool isVector,
        std::ifstream &stream ) const;

      std::string mDatFile;
  };

} // namespace MDAL
#endif //MDAL_ASCII_DAT_HPP
