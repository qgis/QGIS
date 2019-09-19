/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_SWW_HPP
#define MDAL_SWW_HPP

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <utility>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"
#include "mdal_netcdf.hpp"

namespace MDAL
{
  /**
   * AnuGA format with extension .SWW
   *
   * The format is based on NetCDF storage
   *
   * Bed Elevation can be static (== one bed elevation for all timesteps, stored
   * in "z" variable or "elevation" variable)
   * or dynamic (each timestep has its own elevation data, stored in "elevation"
   * variable with multiple dimensions)
   *
   * Vector data are recognized by prefix "x" and "y" in the name
   * Maximums data are recognized by suffix "_range" in the name
   */
  class DriverSWW: public Driver
  {
    public:
      DriverSWW();
      ~DriverSWW( ) override = default;
      DriverSWW *create() override;

      std::unique_ptr< Mesh > load( const std::string &resultsFile, MDAL_Status *status ) override;
      bool canRead( const std::string &uri ) override;

    private:
      size_t getVertexCount( const NetCDFFile &ncFile ) const;
      std::vector<double> readZCoords( const NetCDFFile &ncFile ) const;
      MDAL::Vertices readVertices( const NetCDFFile &ncFile ) const;
      MDAL::Faces readFaces( const NetCDFFile &ncFile ) const;
      std::vector<double> readTimes( const NetCDFFile &ncFile ) const;
      /**
       * Finds all variables (arrays) in netcdf file and base on the name add it as
       * vector or scalar dataset group
       */
      void readDatasetGroups( const NetCDFFile &ncFile, MDAL::MemoryMesh *mesh, const std::vector<double> &times ) const;
      bool parseGroupName( std::string &groupName, std::string &xName, std::string &yName ) const;

      std::shared_ptr<MDAL::DatasetGroup> readScalarGroup(
        const NetCDFFile &ncFile,
        MDAL::MemoryMesh *mesh,
        const std::vector<double> &times,
        const std::string variableBaseName,
        const std::string arrName
      ) const;

      std::shared_ptr<MDAL::DatasetGroup> readVectorGroup(
        const NetCDFFile &ncFile,
        MDAL::MemoryMesh *mesh,
        const std::vector<double> &times,
        const std::string variableBaseName,
        const std::string arrXName,
        const std::string arrYName
      ) const;

      void addBedElevation(
        const NetCDFFile &ncFile,
        MDAL::MemoryMesh *mesh,
        const std::vector<double> &times
      ) const;

      std::string mFileName;
  };
} // namespace MDAL
#endif //MDAL_SWW_HPP
