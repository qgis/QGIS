/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_3DI_HPP
#define MDAL_3DI_HPP

#include <map>
#include <string>
#include <stddef.h>

#include "mdal_config.hpp"
#include "mdal_cf.hpp"
#include "mdal_driver.hpp"

namespace MDAL
{

  /**
   * Driver of 3Di file format.
   *
   * The result 3Di NetCDF file is based on CF-conventions with some additions.
   * It is unstructured grid with data stored in NetCDF/HDF5 file format.
   * A division is made between  a 1D and 2D which can be distinguished through
   * the prefixes “MESH2D” and “MESH1D”. For both meshes the information is present
   * in coordinate, id, and type variables.
   *
   * A version of the data scheme is not present yet.
   *
   * The 2D Mesh consists of calculation Nodes that represents centers of Faces.
   * There is no concept of Vertices in the file. The vertices that forms a face
   * are specified by X,Y coordinates in the "Face Contours" arrays. The "lines"
   * represent the face's edges and are again specified by X,Y coordinate of the
   * line center. Data is specified on calculation nodes (i.e. dataset defined on faces)
   * and on lines (i.e. dataset defined on edges - not implemented yet)
   *
   * The 1D Mesh is present too, but not parsed yet.
   */
  class Driver3Di: public DriverCF
  {
    public:
      Driver3Di();
      ~Driver3Di() override = default;
      Driver3Di *create() override;
      std::string buildUri( const std::string &meshFile ) override;
    private:
      CFDimensions populateDimensions( ) override;
      void populate2DMeshDimensions( MDAL::CFDimensions &dims );
      void populateElements( Vertices &vertices, Edges &edges, Faces &faces ) override;
      void populateMesh2DElements( Vertices &vertices, Faces &faces );
      void addBedElevation( MemoryMesh *mesh ) override;
      std::string getCoordinateSystemVariableName() override;
      std::string getTimeVariableName() const override;
      std::set<std::string> ignoreNetCDFVariables() override;
      void parseNetCDFVariableMetadata( int varid,
                                        std::string &variableName,
                                        std::string &name,
                                        bool *is_vector,
                                        bool *isPolar,
                                        bool *invertedDirection,
                                        bool *is_x ) override;
      std::vector<std::pair<double, double>> parseClassification( int varid ) const override;

      //! Returns number of vertices
      size_t parse2DMesh();

      void addBedElevationDatasetOnFaces();

      void populate1DMeshDimensions( MDAL::CFDimensions &dims );
      void populateMesh1DElements( Vertices &vertices, Edges &edges );
      bool check1DConnection( std::string fileName );
      void parse1DConnection( const std::vector<int> &nodesId, const std::vector<int> &edgesId, Edges &edges );
  };

} // namespace MDAL

#endif // MDAL_3DI_HPP
