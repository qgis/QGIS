/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include <vector>
#include <string>
#include <netcdf.h>
#include "math.h"
#include <stdlib.h>
#include <assert.h>
#include <cstring>

#include "mdal_data_model.hpp"
#include "mdal_cf.hpp"
#include "mdal_utils.hpp"

#define CF_THROW_ERR throw MDAL_Status::Err_UnknownFormat

MDAL::cfdataset_info_map MDAL::DriverCF::parseDatasetGroupInfo()
{
  /*
   * list of datasets:
   *   Getting the full list of variables from the file and then grouping them in two steps:
   *   - Grouping (or filtering) based on whether they’re time-dependent (find time dimension id,
   *     and check whether each of the data variables has that dimension id in its own dimensions).
   *   - Next, filtering them on whether they’re space-dependent, possibly grouping them based on
   *     their topological location: this can be inquired by getting their “:location” attribute
   *     which has either the value “face” (often), “edge” (sometimes), or “node” (rarely).
   *
   * naming:
   *     You could use the long_name to print a human readable variable name. When that is absent,
   *     use the standard_name of the variable and use your own lookup table for a human readable
   *     variable name (e.g.: sea_surface_level_above_geoid could translate into “Water level”).
   *     Finally, if also standard_name is absent, fall back to the bare variable name (e.g. “mesh2d_s1”).
   */


  /* PHASE 1 - gather all variables to be used for node/element datasets */
  cfdataset_info_map dsinfo_map;
  int varid = -1;

  std::set<std::string> ignoreVariables = ignoreNetCDFVariables();

  do
  {
    ++varid;

    // get variable name
    char variable_name_c[NC_MAX_NAME];
    if ( nc_inq_varname( mNcFile->handle(), varid, variable_name_c ) ) break; // probably we are at the end of available arrays, quit endless loop
    std::string variable_name( variable_name_c );

    if ( ignoreVariables.find( variable_name ) == ignoreVariables.end() )
    {
      // get number of dimensions
      int ndims;
      if ( nc_inq_varndims( mNcFile->handle(), varid, &ndims ) ) continue;

      // we parse either time-dependent or time-independent (e.g. Bed/Maximums)
      if ( ( ndims < 1 ) || ( ndims > 2 ) ) continue;
      int dimids[2];
      if ( nc_inq_vardimid( mNcFile->handle(), varid, dimids ) ) continue;

      int dimid;
      size_t nTimesteps;
      CFDatasetGroupInfo::TimeLocation timeLocation;

      if ( ndims == 1 )
      {
        nTimesteps = 1;
        dimid = dimids[0];
        timeLocation = CFDatasetGroupInfo::NoTimeDimension;
      }
      else
      {
        /*
         * UGRID convention says "The order of dimensions on a data variable is arbitrary",
         * So time dimension is not necessary the first dimension, event if the convention recommands it.
         * And prevent that any of the the two dimensions is not time dimension
         */
        if ( mDimensions.type( dimids[0] ) == CFDimensions::Time )
        {
          timeLocation = CFDatasetGroupInfo::TimeDimensionFirst;
          dimid = dimids[1];
        }
        else if ( mDimensions.type( dimids[1] ) == CFDimensions::Time )
        {
          timeLocation = CFDatasetGroupInfo::TimeDimensionLast;
          dimid = dimids[0];
        }
        else
        {
          // unknown array
          continue;
        }

        nTimesteps = mDimensions.size( CFDimensions::Time );
      }

      if ( !mDimensions.isDatasetType( mDimensions.type( dimid ) ) )
        continue;

      // Get name, if it is vector and if it is x or y
      std::string name;
      bool is_vector = true;
      bool is_x = false;

      parseNetCDFVariableMetadata( varid, variable_name, name, &is_vector, &is_x );

      // Add it to the map
      auto it = dsinfo_map.find( name );
      if ( it != dsinfo_map.end() )
      {
        if ( is_x )
        {
          it->second.ncid_x = varid;
        }
        else
        {
          it->second.ncid_y = varid;
        }
      }
      else
      {
        CFDatasetGroupInfo dsInfo;
        dsInfo.nTimesteps = nTimesteps;
        dsInfo.is_vector = is_vector;
        if ( is_x )
        {
          dsInfo.ncid_x = varid;
        }
        else
        {
          dsInfo.ncid_y = varid;
        }
        dsInfo.outputType = mDimensions.type( dimid );
        dsInfo.name = name;
        dsInfo.nValues = mDimensions.size( mDimensions.type( dimid ) );
        dsInfo.timeLocation = timeLocation;
        dsinfo_map[name] = dsInfo;
      }
    }
  }
  while ( true );

  if ( dsinfo_map.size() == 0 ) throw MDAL_Status::Err_InvalidData;

  return dsinfo_map;
}

static void populate_vals( bool is_vector, double *vals, size_t i,
                           const std::vector<double> &vals_x, const std::vector<double> &vals_y,
                           size_t idx, double fill_val_x, double fill_val_y )
{
  if ( is_vector )
  {
    vals[2 * i] = MDAL::safeValue( vals_x[idx], fill_val_x );
    vals[2 * i + 1] = MDAL::safeValue( vals_y[idx], fill_val_y );
  }
  else
  {
    vals[i] = MDAL::safeValue( vals_x[idx], fill_val_x );
  }
}

void MDAL::DriverCF::addDatasetGroups( MDAL::Mesh *mesh, const std::vector<RelativeTimestamp> &times, const MDAL::cfdataset_info_map &dsinfo_map, const MDAL::DateTime &referenceTime )
{
  /* PHASE 2 - add dataset groups */
  for ( const auto &it : dsinfo_map )
  {
    const CFDatasetGroupInfo dsi = it.second;
    // Create a dataset group
    std::shared_ptr<MDAL::DatasetGroup> group = std::make_shared<MDAL::DatasetGroup>(
          name(),
          mesh,
          mFileName,
          dsi.name
        );
    group->setIsScalar( !dsi.is_vector );

    if ( dsi.outputType == CFDimensions::Vertex2D )
      group->setDataLocation( MDAL_DataLocation::DataOnVertices2D );
    else if ( dsi.outputType == CFDimensions::Face2D )
      group->setDataLocation( MDAL_DataLocation::DataOnFaces2D );
    else if ( dsi.outputType == CFDimensions::Volume3D )
      group->setDataLocation( MDAL_DataLocation::DataOnVolumes3D );
    else
    {
      // unsupported
      continue;
    }

    // read X data
    double fill_val_x = mNcFile->getFillValue( dsi.ncid_x );

    // read Y data if vector
    double fill_val_y = std::numeric_limits<double>::quiet_NaN();
    std::vector<double> vals_y;
    if ( dsi.is_vector )
    {
      fill_val_y = mNcFile->getFillValue( dsi.ncid_y );
    }

    // Create dataset
    for ( size_t ts = 0; ts < dsi.nTimesteps; ++ts )
    {
      std::shared_ptr<MDAL::Dataset> dataset;
      if ( dsi.outputType == CFDimensions::Volume3D )
      {
        dataset = create3DDataset(
                    group, ts, dsi, fill_val_x, fill_val_y );
      }
      else
      {
        dataset = create2DDataset(
                    group,
                    ts,
                    dsi, fill_val_x, fill_val_y
                  );
      }

      if ( dataset )
      {
        dataset->setTime( times[ts] );
        group->datasets.push_back( dataset );
      }
    }

    // Add to mesh
    if ( !group->datasets.empty() )
    {
      group->setStatistics( MDAL::calculateStatistics( group ) );
      group->setReferenceTime( referenceTime );
      mesh->datasetGroups.push_back( group );
    }
  }
}

MDAL::DateTime MDAL::DriverCF::parseTime( std::vector<RelativeTimestamp> &times )
{

  size_t nTimesteps = mDimensions.size( CFDimensions::Time );
  if ( 0 == nTimesteps )
  {
    //if no time dimension is present creates only one time step to store the potential time-independent variable
    nTimesteps = 1;
    times = std::vector<RelativeTimestamp>( 1, RelativeTimestamp() );
    return MDAL::DateTime();
  }
  const std::string timeArrName = getTimeVariableName();
  std::vector<double> rawTimes = mNcFile->readDoubleArr( timeArrName, nTimesteps );

  std::string timeUnitInformation = mNcFile->getAttrStr( timeArrName, "units" );
  std::string calendar = mNcFile->getAttrStr( timeArrName, "calendar" );
  MDAL::DateTime referenceTime = parseCFReferenceTime( timeUnitInformation, calendar );
  if ( !referenceTime.isValid() )
    referenceTime = defaultReferenceTime();

  MDAL::RelativeTimestamp::Unit unit = parseCFTimeUnit( timeUnitInformation );

  times = std::vector<RelativeTimestamp>( nTimesteps );
  for ( size_t i = 0; i < nTimesteps; ++i )
  {
    times[i] = RelativeTimestamp( rawTimes[i], unit );
  }

  return referenceTime;
}

std::shared_ptr<MDAL::Dataset> MDAL::DriverCF::create2DDataset( std::shared_ptr<MDAL::DatasetGroup> group, size_t ts, const MDAL::CFDatasetGroupInfo &dsi, double fill_val_x, double fill_val_y )
{
  std::shared_ptr<MDAL::CFDataset2D> dataset = std::make_shared<MDAL::CFDataset2D>(
        group.get(),
        fill_val_x,
        fill_val_y,
        dsi.ncid_x,
        dsi.ncid_y,
        dsi.timeLocation,
        dsi.nTimesteps,
        dsi.nValues,
        ts,
        mNcFile
      );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  return std::move( dataset );
}

std::shared_ptr<MDAL::Dataset> MDAL::DriverCF::create3DDataset( std::shared_ptr<MDAL::DatasetGroup>,
    size_t, const MDAL::CFDatasetGroupInfo &,
    double, double )
{
  std::shared_ptr<MDAL::Dataset> dataset;
  return dataset;
}


MDAL::DriverCF::DriverCF( const std::string &name,
                          const std::string &longName,
                          const std::string &filters,
                          const int capabilities ):
  Driver( name, longName, filters, capabilities )
{
}

MDAL::DriverCF::~DriverCF() = default;

bool MDAL::DriverCF::canReadMesh( const std::string &uri )
{
  try
  {
    mNcFile.reset( new NetCDFFile );
    mNcFile->openFile( uri );
    populateDimensions( );
  }
  catch ( MDAL_Status )
  {
    return false;
  }
  return true;
}

MDAL::DateTime MDAL::DriverCF::defaultReferenceTime() const
{
  // return invalid reference time
  return DateTime();
}

void MDAL::DriverCF::setProjection( MDAL::Mesh *mesh )
{
  std::string coordinate_system_variable = getCoordinateSystemVariableName();
  // not present
  if ( coordinate_system_variable.empty() )
  {
    return;
  }

  // in file
  if ( MDAL::startsWith( coordinate_system_variable, "file://" ) )
  {
    const std::string filename = MDAL::replace( coordinate_system_variable, "file://", "" );
    mesh->setSourceCrsFromPrjFile( filename );
    return;
  }

  // in NetCDF attribute
  try
  {
    if ( !coordinate_system_variable.empty() )
    {
      std::string wkt = mNcFile->getAttrStr( coordinate_system_variable, "wkt" );
      if ( wkt.empty() )
      {
        std::string epsg_code = mNcFile->getAttrStr( coordinate_system_variable, "EPSG_code" );
        if ( epsg_code.empty() )
        {
          int epsg = mNcFile->getAttrInt( coordinate_system_variable, "epsg" );
          if ( epsg != 0 )
          {
            mesh->setSourceCrsFromEPSG( epsg );
          }
        }
        else
        {
          mesh->setSourceCrs( epsg_code );
        }
      }
      else
      {
        mesh->setSourceCrsFromWKT( wkt );
      }
    }

  }
  catch ( MDAL_Status )
  {
    return;
  }
}

std::unique_ptr< MDAL::Mesh > MDAL::DriverCF::load( const std::string &fileName, MDAL_Status *status )
{
  mNcFile.reset( new NetCDFFile );

  mFileName = fileName;

  if ( status ) *status = MDAL_Status::None;

  //Dimensions dims;
  std::vector<MDAL::RelativeTimestamp> times;

  try
  {
    // Open file
    mNcFile->openFile( mFileName );

    // Parse dimensions
    mDimensions = populateDimensions( );

    // Create mMesh
    Faces faces;
    Vertices vertices;
    populateFacesAndVertices( vertices, faces );
    std::unique_ptr< MemoryMesh > mesh(
      new MemoryMesh(
        name(),
        vertices.size(),
        faces.size(),
        mDimensions.size( mDimensions.MaxVerticesInFace ),
        computeExtent( vertices ),
        mFileName
      )
    );
    mesh->faces = faces;
    mesh->vertices = vertices;
    addBedElevation( mesh.get() );
    setProjection( mesh.get() );

    // Parse time array
    MDAL::DateTime referenceTime = parseTime( times );

    // Parse dataset info
    cfdataset_info_map dsinfo_map = parseDatasetGroupInfo();

    // Create datasets
    addDatasetGroups( mesh.get(), times, dsinfo_map, referenceTime );

    return std::unique_ptr<Mesh>( mesh.release() );
  }
  catch ( MDAL_Status error )
  {
    if ( status ) *status = ( error );
    return std::unique_ptr<Mesh>();
  }
}

//////////////////////////////////////////////////////////////////////////////////////

MDAL::CFDimensions::Type MDAL::CFDimensions::type( int ncid ) const
{
  const auto it = mNcId.find( ncid );
  if ( it == mNcId.end() )
    return UnknownType;
  else
    return it->second;
}

size_t MDAL::CFDimensions::size( MDAL::CFDimensions::Type type ) const
{
  const auto it = mCount.find( type );
  if ( it == mCount.end() )
    return 0;
  else
    return it->second;
}

void MDAL::CFDimensions::setDimension( MDAL::CFDimensions::Type type,
                                       size_t count, int ncid )
{
  mNcId[ncid] = type;
  mCount[type] = count;
}

bool MDAL::CFDimensions::isDatasetType( MDAL::CFDimensions::Type type ) const
{
  return (
           ( type == Vertex1D ) ||
           ( type == Vertex2D ) ||
           ( type == Line1D ) ||
           ( type == Face2DEdge ) ||
           ( type == Face2D ) ||
           ( type == Volume3D )
         );
}

//////////////////////////////////////////////////////////////////////////////////////
MDAL::CFDataset2D::CFDataset2D( MDAL::DatasetGroup *parent,
                                double fill_val_x, double fill_val_y,
                                int ncid_x, int ncid_y, CFDatasetGroupInfo::TimeLocation timeLocation,
                                size_t timesteps, size_t values, size_t ts, std::shared_ptr<NetCDFFile> ncFile )
  : Dataset2D( parent )
  , mFillValX( fill_val_x )
  , mFillValY( fill_val_y )
  , mNcidX( ncid_x )
  , mNcidY( ncid_y )
  , mTimeLocation( timeLocation )
  , mTimesteps( timesteps )
  , mValues( values )
  , mTs( ts )
  , mNcFile( ncFile )
{
}

MDAL::CFDataset2D::~CFDataset2D() = default;

size_t MDAL::CFDataset2D::scalarData( size_t indexStart, size_t count, double *buffer )
{
  assert( group()->isScalar() ); //checked in C API interface
  if ( ( count < 1 ) || ( indexStart >= mValues ) )
    return 0;
  if ( mTs >= mTimesteps )
    return 0;

  size_t copyValues = std::min( mValues - indexStart, count );
  std::vector<double> values_x;

  if ( mTimeLocation == CFDatasetGroupInfo::NoTimeDimension )
  {
    values_x = mNcFile->readDoubleArr(
                 mNcidX,
                 indexStart,
                 copyValues
               );
  }
  else
  {
    bool timeFirstDim = mTimeLocation == CFDatasetGroupInfo::TimeDimensionFirst;
    size_t start_dim1 = timeFirstDim ?  mTs : indexStart;
    size_t start_dim2 = timeFirstDim ?  indexStart : mTs;
    size_t count_dim1 = timeFirstDim ?  1 : copyValues;
    size_t count_dim2 = timeFirstDim ?  copyValues : 1;

    values_x = mNcFile->readDoubleArr(
                 mNcidX,
                 start_dim1,
                 start_dim2,
                 count_dim1,
                 count_dim2
               );
  }

  for ( size_t i = 0; i < copyValues; ++i )
  {
    populate_vals( false,
                   buffer,
                   i,
                   values_x,
                   std::vector<double>(),
                   i,
                   mFillValX,
                   mFillValY );
  }
  return copyValues;
}

size_t MDAL::CFDataset2D::vectorData( size_t indexStart, size_t count, double *buffer )
{
  assert( !group()->isScalar() ); //checked in C API interface
  if ( ( count < 1 ) || ( indexStart >= mValues ) )
    return 0;

  if ( mTs >= mTimesteps )
    return 0;

  size_t copyValues = std::min( mValues - indexStart, count );

  std::vector<double> values_x;
  std::vector<double> values_y;

  if ( mTimeLocation == CFDatasetGroupInfo::NoTimeDimension )
  {
    values_x = mNcFile->readDoubleArr(
                 mNcidX,
                 indexStart,
                 copyValues
               );

    values_y = mNcFile->readDoubleArr(
                 mNcidX,
                 indexStart,
                 copyValues
               );
  }
  else
  {
    bool timeFirstDim = mTimeLocation == CFDatasetGroupInfo::TimeDimensionFirst;
    size_t start_dim1 = timeFirstDim ?  mTs : indexStart;
    size_t start_dim2 = timeFirstDim ?  indexStart : mTs;
    size_t count_dim1 = timeFirstDim ?  1 : copyValues;
    size_t count_dim2 = timeFirstDim ?  copyValues : 1;

    values_x = mNcFile->readDoubleArr(
                 mNcidX,
                 start_dim1,
                 start_dim2,
                 count_dim1,
                 count_dim2
               );
    values_y = mNcFile->readDoubleArr(
                 mNcidY,
                 start_dim1,
                 start_dim2,
                 count_dim1,
                 count_dim2
               );
  }

  for ( size_t i = 0; i < copyValues; ++i )
  {
    populate_vals( true,
                   buffer,
                   i,
                   values_x,
                   values_y,
                   i,
                   mFillValX,
                   mFillValY );
  }

  return copyValues;
}
