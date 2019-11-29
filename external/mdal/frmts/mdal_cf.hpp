/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_CF_HPP
#define MDAL_CF_HPP

#include <string>
#include <vector>
#include <map>
#include <stddef.h>
#include <set>

#include "mdal_data_model.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_netcdf.hpp"
#include "mdal_driver.hpp"

namespace MDAL
{
  class CFDimensions
  {
    public:
      enum Type
      {
        UnknownType = 0, //!< Unknown
        Vertex1D, //!< Vertex in 1D mesh
        Vertex2D, //!< Vertex in 2D mesh
        Line1D, //!< Line joining 1D vertices
        Face2DEdge, //!< Edge of 2D Face
        Face2D, //!< 2D (Polygon) Face
        Volume3D, //!< 3D (stacked) volumes
        StackedFace3D, //! 3D (stacked) Faces
        Time, //!< Time steps
        MaxVerticesInFace //!< Maximum number of vertices in a face
      };

      //! Returns type of dimensions based on NetCDF array id
      Type type( int ncid ) const;
      //! Returns number of items for the type
      size_t size( Type type ) const;
      //! Sets a dimension
      void setDimension( Type type, size_t count, int ncid = -1 );
      //! Returns whether the type is one that case be used for datasets definition
      bool isDatasetType( Type type ) const;

    private:
      std::map<Type, size_t> mCount;
      std::map<int, Type> mNcId;
  };

  struct CFDatasetGroupInfo
  {
    std::string name; //!< Dataset group name
    CFDimensions::Type outputType;
    bool is_vector;
    bool time_first_dim;
    size_t nTimesteps;
    size_t nValues;
    int ncid_x; //!< NetCDF variable id
    int ncid_y; //!< NetCDF variable id
  };
  typedef std::map<std::string, CFDatasetGroupInfo> cfdataset_info_map; // name -> DatasetInfo

  class CFDataset2D: public Dataset2D
  {
    public:
      CFDataset2D( DatasetGroup *parent,
                   double fill_val_x,
                   double fill_val_y,
                   int ncid_x,
                   int ncid_y,
                   bool time_first_dim,
                   size_t timesteps,
                   size_t values,
                   size_t ts,
                   std::shared_ptr<NetCDFFile> ncFile
                 );
      virtual ~CFDataset2D() override;

      virtual size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      virtual size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;
      virtual size_t activeData( size_t indexStart, size_t count, int *buffer ) override;
    private:
      double mFillValX;
      double mFillValY;
      int mNcidX; //!< NetCDF variable id
      int mNcidY; //!< NetCDF variable id
      bool mTimeFirstDim;
      size_t mTimesteps;
      size_t mValues;
      size_t mTs;
      std::shared_ptr<NetCDFFile> mNcFile;
  };

  //! NetCDF Climate and Forecast (CF) Metadata Conventions
  //! http://cfconventions.org
  //! and http://ugrid-conventions.github.io/ugrid-conventions/
  class DriverCF: public Driver
  {
    public:
      DriverCF( const std::string &name,
                const std::string &longName,
                const std::string &filters,
                const int capabilities );
      virtual ~DriverCF() override;
      bool canReadMesh( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &fileName, MDAL_Status *status ) override;

    protected:
      virtual CFDimensions populateDimensions( ) = 0;
      virtual void populateFacesAndVertices( Vertices &vertices, Faces &faces ) = 0;
      virtual void addBedElevation( MDAL::MemoryMesh *mesh ) = 0;
      virtual std::string getCoordinateSystemVariableName() = 0;
      virtual std::set<std::string> ignoreNetCDFVariables() = 0;
      virtual void parseNetCDFVariableMetadata( int varid, const std::string &variableName,
          std::string &name, bool *is_vector, bool *is_x ) = 0;
      virtual std::string getTimeVariableName() const = 0;
      virtual std::shared_ptr<MDAL::Dataset> create2DDataset(
        std::shared_ptr<MDAL::DatasetGroup> group,
        size_t ts,
        const MDAL::CFDatasetGroupInfo &dsi,
        double fill_val_x, double fill_val_y );

      virtual std::shared_ptr<MDAL::Dataset> create3DDataset(
        std::shared_ptr<MDAL::DatasetGroup> group,
        size_t ts,
        const MDAL::CFDatasetGroupInfo &dsi,
        double fill_val_x, double fill_val_y );

      void setProjection( MDAL::Mesh *m );
      cfdataset_info_map parseDatasetGroupInfo();
      void parseTime( std::vector<double> &times );
      void addDatasetGroups( Mesh *mesh,
                             const std::vector<double> &times,
                             const cfdataset_info_map &dsinfo_map );

      std::string mFileName;
      std::shared_ptr<NetCDFFile> mNcFile;
      CFDimensions mDimensions;
  };

} // namespace MDAL

#endif // MDAL_CF_HPP
