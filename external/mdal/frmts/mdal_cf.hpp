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
        Time, //!< Time steps
        MaxVerticesInFace //!< Maximum number of vertices in a face
      };

      //! Returns type of dimensions based on NetCDF array id
      Type type( int ncid ) const;
      //! Returns number of items for the type
      size_t size( Type type ) const;
      //! Sets a dimension
      void setDimension( Type type, size_t count, int ncid = -1 );
      //! Returns number of faces (1D + 2D)
      size_t faceCount() const;
      //! Returns number of vertices (1D + 2D)
      size_t vertexCount() const;
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
    size_t nTimesteps;
    int ncid_x; //!< NetCDF variable id
    int ncid_y; //!< NetCDF variable id
    size_t arr_size;
  };
  typedef std::map<std::string, CFDatasetGroupInfo> cfdataset_info_map; // name -> DatasetInfo

  //! NetCDF Climate and Forecast (CF) Metadata Conventions
  //! http://cfconventions.org
  class LoaderCF
  {
    public:
      LoaderCF( const std::string &fileName );
      virtual ~LoaderCF() = default;

      std::unique_ptr< Mesh > load( MDAL_Status *status );

    protected:
      virtual CFDimensions populateDimensions() = 0;
      virtual void populateFacesAndVertices( MDAL::Mesh *mesh ) = 0;
      virtual void addBedElevation( MDAL::Mesh *mesh ) = 0;
      virtual std::string getCoordinateSystemVariableName() = 0;
      virtual std::set<std::string> ignoreNetCDFVariables() = 0;
      virtual std::string nameSuffix( CFDimensions::Type type ) = 0;
      virtual void parseNetCDFVariableMetadata( int varid, const std::string &variableName,
          std::string &name, bool *is_vector, bool *is_x ) = 0;

      void setProjection( MDAL::Mesh *m );
      cfdataset_info_map parseDatasetGroupInfo();
      void parseTime( std::vector<double> &times );
      std::shared_ptr<MDAL::Dataset> createFace2DDataset( size_t ts, const MDAL::CFDatasetGroupInfo &dsi,
          const std::vector<double> &vals_x,
          const std::vector<double> &vals_y,
          double fill_val_x, double fill_val_y );

      void addDatasetGroups( Mesh *mesh,
                             const std::vector<double> &times,
                             const cfdataset_info_map &dsinfo_map );


      const std::string mFileName;
      NetCDFFile mNcFile;
      CFDimensions mDimensions;
  };

} // namespace MDAL

#endif // MDAL_CF_HPP
