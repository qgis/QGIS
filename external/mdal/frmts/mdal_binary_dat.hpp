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
#include "mdal_driver.hpp"

namespace MDAL
{

  class DriverBinaryDat: public Driver
  {
    public:
      DriverBinaryDat();
      ~DriverBinaryDat( ) override;
      DriverBinaryDat *create() override;

      bool canRead( const std::string &uri ) override;
      void load( const std::string &datFile, Mesh *mesh, MDAL_Status *status ) override;
      bool persist( DatasetGroup *group ) override;

    private:
      bool readVertexTimestep( const Mesh *mesh,
                               std::shared_ptr<DatasetGroup> group,
                               std::shared_ptr<DatasetGroup> groupMax,
                               double time,
                               bool hasStatus,
                               int sflg,
                               std::ifstream &in );

      double convertTimeDataToHours( double time, int originalTimeDataUnit );
      std::string mDatFile;
  };

} // namespace MDAL
#endif //MDAL_BINARY_DAT_HPP
