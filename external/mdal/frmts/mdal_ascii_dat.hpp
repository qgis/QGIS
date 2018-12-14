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

  class DriverAsciiDat: public Driver
  {
    public:
      DriverAsciiDat();
      ~DriverAsciiDat( ) override;
      DriverAsciiDat *create() override;

      bool canRead( const std::string &uri ) override;
      void load( const std::string &datFile, Mesh *mesh, MDAL_Status *status ) override;


    private:
      void readVertexTimestep(
        const Mesh *mesh,
        std::shared_ptr<DatasetGroup> group,
        double t,
        bool isVector,
        bool hasStatus,
        std::ifstream &stream );

      void readFaceTimestep(
        const Mesh *mesh,
        std::shared_ptr<DatasetGroup> group,
        double t,
        bool isVector,
        std::ifstream &stream );

      std::string mDatFile;
  };

} // namespace MDAL
#endif //MDAL_ASCII_DAT_HPP
