/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_UGRID_HPP
#define MDAL_UGRID_HPP

#include <map>
#include <string>
#include <stddef.h>

#include "mdal_cf.hpp"
#include "mdal_driver.hpp"

namespace MDAL
{
  /**
   * Driver of UGRID file format.
   *
   * The result UGRID NetCDF file is strictly based on CF-conventions 1.6
   */
  class DriverUgrid: public DriverCF
  {
    public:
      DriverUgrid();
      ~DriverUgrid() override = default;
      DriverUgrid *create() override;

    private:
      CFDimensions populateDimensions( ) override;
      void populateFacesAndVertices( Vertices &vertices, Faces &faces ) override;
      void populateVertices( Vertices &vertices );
      void populateFaces( Faces &faces );
      void addBedElevation( MemoryMesh *mesh ) override;
      std::string getCoordinateSystemVariableName() override;
      std::set<std::string> ignoreNetCDFVariables() override;
      void parseNetCDFVariableMetadata( int varid, const std::string &variableName,
                                        std::string &name, bool *is_vector, bool *is_x ) override;

      void parse2VariablesFromAttribute( const std::string &name, const std::string &attr_name,
                                         std::string &var1, std::string &var2,
                                         bool optional ) const;
      std::string findMeshName( int dimension, bool optional ) const;
      std::string mMesh2dName;
      std::string mMesh1dName;
      std::string nodeZVariableName() const;
  };

} // namespace MDAL

#endif // MDAL_UGRID_HPP
