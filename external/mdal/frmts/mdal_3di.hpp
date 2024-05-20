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
  class CF3DiDataset2D: public CFDataset2D
  {
    public:
      CF3DiDataset2D( DatasetGroup *parent,
                      double fill_val_x,
                      double fill_val_y,
                      int ncid_x,
                      int ncid_y,
                      Classification classification_x,
                      Classification classification_y,
                      CFDatasetGroupInfo::TimeLocation timeLocation,
                      size_t timesteps,
                      size_t values,
                      size_t ts,
                      std::shared_ptr<NetCDFFile> ncFile,
                      std::vector<size_t> mask
                    );
      virtual ~CF3DiDataset2D() override;

      virtual size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      virtual size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;

    private:
      std::vector< size_t > mRequestedMeshFaceIds;
  };

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
   * The 2D mesh consists of two mostly overlapping meshes, one for groundwater data and one
   * for surface water. Each face belongs to either one of them depending on its Node_type values.
   * While the whole mesh data can be accessed using the "Mesh2D" mesh name, the two sub-meshes
   * are individually accessible using "Mesh2D_groundwater" and "Mesh2D_surface_water"
   * mesh names respectively.
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
      std::shared_ptr<MDAL::Dataset> create2DDataset(
        std::shared_ptr<MDAL::DatasetGroup> group,
        size_t ts,
        const MDAL::CFDatasetGroupInfo &dsi,
        double fill_val_x, double fill_val_y ) override;
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

      //! Holds the subset of face ids if the requested mesh is groundwater/surface_water, empty otherwise
      std::vector<size_t> mRequestedMeshFaceIds;
  };

} // namespace MDAL

#endif // MDAL_3DI_HPP
