/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include <vector>
#include <string>

#include "mdal_data_model.hpp"
#include "mdal_cf.hpp"
#include "mdal_utils.hpp"

#include "math.h"
#include <stdlib.h>

#define CF_THROW_ERR throw MDAL_Status::Err_UnknownFormat

MDAL::cfdataset_info_map MDAL::LoaderCF::parseDatasetGroupInfo()
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
    if ( nc_inq_varname( mNcFile.handle(), varid, variable_name_c ) ) break; // probably we are at the end of available arrays, quit endless loop
    std::string variable_name( variable_name_c );

    if ( ignoreVariables.find( variable_name ) == ignoreVariables.end() )
    {
      // get number of dimensions
      int ndims;
      if ( nc_inq_varndims( mNcFile.handle(), varid, &ndims ) ) continue;

      // we parse either time-dependent or time-independent (e.g. Bed/Maximums)
      if ( ( ndims < 1 ) || ( ndims > 2 ) ) continue;
      int dimids[2];
      if ( nc_inq_vardimid( mNcFile.handle(), varid, dimids ) ) continue;

      int dimid;
      size_t nTimesteps;

      if ( ndims == 1 )
      {
        nTimesteps = 1;
        dimid = dimids[0];
      }
      else
      {
        nTimesteps = mDimensions.size( CFDimensions::Time );
        dimid = dimids[1];
      }

      if ( !mDimensions.isDatasetType( mDimensions.type( dimid ) ) )
        continue;

      size_t arr_size = mDimensions.size( mDimensions.type( dimid ) ) * nTimesteps;
      std::string suffix = nameSuffix( mDimensions.type( dimid ) );

      // Get name, if it is vector and if it is x or y
      std::string name;
      bool is_vector = true;
      bool is_x = false;

      parseNetCDFVariableMetadata( varid, variable_name, name, &is_vector, &is_x );
      if ( !suffix.empty() )
        name = name + " (" + suffix + ")";

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
        dsInfo.arr_size = arr_size;
        dsinfo_map[name] = dsInfo;
      }
    }
  }
  while ( true );

  if ( dsinfo_map.size() == 0 )
  {
    throw MDAL_Status::Err_InvalidData;
  }

  return dsinfo_map;
}

static void populate_vals( bool is_vector, std::vector<MDAL::Value> &vals, size_t i,
                           const std::vector<double> &vals_x, const std::vector<double> &vals_y,
                           size_t idx, double fill_val_x, double fill_val_y )
{

  vals[i].x = MDAL::safeValue( vals_x[idx], fill_val_x );
  if ( is_vector )
  {
    vals[i].y = MDAL::safeValue( vals_y[idx], fill_val_y );
  }
}

static void populate_nodata( std::vector<MDAL::Value> &vals, size_t from_i, size_t to_i )
{
  for ( size_t i = from_i; i < to_i; ++i )
  {
    vals[i].noData = true;
    vals[i].x = std::numeric_limits<double>::quiet_NaN();
    vals[i].y = std::numeric_limits<double>::quiet_NaN();
  }
}


std::shared_ptr<MDAL::Dataset> MDAL::LoaderCF::createFace2DDataset( size_t ts, const MDAL::CFDatasetGroupInfo &dsi,
    const std::vector<double> &vals_x, const std::vector<double> &vals_y,
    double fill_val_x, double fill_val_y )
{
  assert( dsi.outputType == CFDimensions::Face2D );
  size_t nFaces2D = mDimensions.size( CFDimensions::Face2D );
  size_t nLine1D = mDimensions.size( CFDimensions::Line1D );

  std::shared_ptr<MDAL::Dataset> dataset = std::make_shared<MDAL::Dataset>();
  dataset->values.resize( mDimensions.faceCount() );

  populate_nodata( dataset->values,
                   0,
                   nLine1D );

  for ( size_t i = 0; i < nFaces2D; ++i )
  {
    size_t idx = ts * nFaces2D + i;
    populate_vals( dsi.is_vector,
                   dataset->values,
                   nLine1D + i,
                   vals_x,
                   vals_y,
                   idx,
                   fill_val_x,
                   fill_val_y );

  }

  return dataset;
}

void MDAL::LoaderCF::addDatasetGroups( MDAL::Mesh *mesh, const std::vector<double> &times, const MDAL::cfdataset_info_map &dsinfo_map )
{
  /* PHASE 2 - add dataset groups */
  for ( const auto &it : dsinfo_map )
  {
    const CFDatasetGroupInfo dsi = it.second;
    // Create a dataset group
    std::shared_ptr<MDAL::DatasetGroup> group = std::make_shared<MDAL::DatasetGroup>();
    group->uri = mFileName;
    group->setName( dsi.name );
    group->isScalar = !dsi.is_vector;

    // read X data
    double fill_val_x = mNcFile.getFillValue( dsi.ncid_x );
    std::vector<double> vals_x( dsi.arr_size );
    if ( nc_get_var_double( mNcFile.handle(), dsi.ncid_x, vals_x.data() ) ) CF_THROW_ERR;

    // read Y data if vector
    double fill_val_y = mNcFile.getFillValue( dsi.ncid_y );
    std::vector<double> vals_y;
    if ( dsi.is_vector )
    {
      vals_y.resize( dsi.arr_size );
      if ( nc_get_var_double( mNcFile.handle(), dsi.ncid_y, vals_y.data() ) ) CF_THROW_ERR;
    }

    // Create dataset
    for ( size_t ts = 0; ts < dsi.nTimesteps; ++ts )
    {
      double time = times[ts];
      std::shared_ptr<MDAL::Dataset> dataset;

      if ( dsi.outputType == CFDimensions::Face2D )
      {
        group->isOnVertices = false;
        dataset = createFace2DDataset( ts, dsi, vals_x, vals_y, fill_val_x, fill_val_y );
      }

      dataset->parent = group.get();
      dataset->time = time;
      group->datasets.push_back( dataset );
    }

    // Add to mesh
    if ( !group->datasets.empty() )
      mesh->datasetGroups.push_back( group );
  }
}

void MDAL::LoaderCF::parseTime( std::vector<double> &times )
{

  size_t nTimesteps = mDimensions.size( CFDimensions::Time );
  times = mNcFile.readDoubleArr( "time", nTimesteps );
  std::string units = mNcFile.getAttrStr( "time", "units" );
  double div_by = MDAL::parseTimeUnits( units );
  for ( size_t i = 0; i < nTimesteps; ++i )
  {
    times[i] /= div_by;
  }
}


MDAL::LoaderCF::LoaderCF( const std::string &fileName ):
  mFileName( fileName )
{
}

void MDAL::LoaderCF::setProjection( MDAL::Mesh *mesh )
{
  std::string coordinate_system_variable = getCoordinateSystemVariableName();

  try
  {

    if ( !coordinate_system_variable.empty() )
    {
      std::string wkt = mNcFile.getAttrStr( coordinate_system_variable, "wkt" );
      if ( wkt.empty() )
      {
        std::string epsg_code = mNcFile.getAttrStr( coordinate_system_variable, "EPSG_code" );
        if ( epsg_code.empty() )
        {
          int epsg = mNcFile.getAttrInt( coordinate_system_variable, "epsg" );
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

std::unique_ptr< MDAL::Mesh > MDAL::LoaderCF::load( MDAL_Status *status )
{
  if ( status ) *status = MDAL_Status::None;

  std::unique_ptr< MDAL::Mesh > mesh( new MDAL::Mesh );
  mesh->uri = mFileName;

  //Dimensions dims;
  std::vector<double> times;

  try
  {
    // Open file
    mNcFile.openFile( mFileName );

    // Parse dimensions
    mDimensions = populateDimensions();

    // Create mMesh
    populateFacesAndVertices( mesh.get() );
    addBedElevation( mesh.get() );
    setProjection( mesh.get() );

    // Parse time array
    parseTime( times );

    // Parse dataset info
    cfdataset_info_map dsinfo_map = parseDatasetGroupInfo();

    // Create datasets
    addDatasetGroups( mesh.get(), times, dsinfo_map );
  }
  catch ( MDAL_Status error )
  {
    if ( status ) *status = ( error );
    if ( mesh ) mesh.reset();
  }

  return mesh;
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

size_t MDAL::CFDimensions::faceCount() const
{
  return size( Face2D ) + size( Line1D );
}

size_t MDAL::CFDimensions::vertexCount() const
{
  return size( Vertex1D ) + size( Vertex2D );
}

bool MDAL::CFDimensions::isDatasetType( MDAL::CFDimensions::Type type ) const
{
  return (
           ( type == Vertex1D ) ||
           ( type == Vertex2D ) ||
           ( type == Line1D ) ||
           ( type == Face2DEdge ) ||
           ( type == Face2D )
         );
}
