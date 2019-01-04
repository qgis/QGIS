/*
 MDAL - mMesh Data Abstraction Library (MIT License)
 Copyright (C) 2016 Lutra Consulting
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include <string>
#include <string.h>
#include <vector>

#include "mdal_netcdf.hpp"
#include "mdal_sww.hpp"

// threshold for determining whether an element is active (wet)
// the format does not explicitly store that information so we
// determine that when loading data
#define DEPTH_THRESHOLD   0.0001   // in meters

MDAL::DriverSWW::DriverSWW()
  : Driver( "SWW",
            "AnuGA",
            "*.sww",
            Capability::ReadMesh )
{
}

MDAL::DriverSWW *MDAL::DriverSWW::create()
{
  return new DriverSWW();
}

bool MDAL::DriverSWW::canRead( const std::string &uri )
{
  int ncid;
  int res;

  // open
  res = nc_open( uri.c_str(), NC_NOWRITE, &ncid );
  if ( res != NC_NOERR )
  {
    MDAL::debug( nc_strerror( res ) );
    nc_close( ncid );
    return false;
  }

  // get dimensions
  int nVolumesId, nVerticesId, nPointsId, nTimestepsId;
  if ( nc_inq_dimid( ncid, "number_of_volumes", &nVolumesId ) != NC_NOERR ||
       nc_inq_dimid( ncid, "number_of_vertices", &nVerticesId ) != NC_NOERR ||
       nc_inq_dimid( ncid, "number_of_points", &nPointsId ) != NC_NOERR ||
       nc_inq_dimid( ncid, "number_of_timesteps", &nTimestepsId ) != NC_NOERR )
  {
    nc_close( ncid );
    return false;
  }

  return true;
}


std::unique_ptr<MDAL::Mesh> MDAL::DriverSWW::load( const std::string &resultsFile,
    MDAL_Status *status )
{
  mFileName = resultsFile;
  if ( status ) *status = MDAL_Status::None;

  int ncid;
  int res;

  res = nc_open( mFileName.c_str(), NC_NOWRITE, &ncid );
  if ( res != NC_NOERR )
  {
    MDAL::debug( nc_strerror( res ) );
    nc_close( ncid );
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return std::unique_ptr< MDAL::Mesh >();
  }

  // get dimensions
  int nVolumesId, nVerticesId, nPointsId, nTimestepsId;
  size_t nVolumes, nVertices, nPoints, nTimesteps;
  if ( nc_inq_dimid( ncid, "number_of_volumes", &nVolumesId ) != NC_NOERR ||
       nc_inq_dimid( ncid, "number_of_vertices", &nVerticesId ) != NC_NOERR ||
       nc_inq_dimid( ncid, "number_of_points", &nPointsId ) != NC_NOERR ||
       nc_inq_dimid( ncid, "number_of_timesteps", &nTimestepsId ) != NC_NOERR )
  {
    nc_close( ncid );
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return std::unique_ptr< MDAL::Mesh >();
  }
  if ( nc_inq_dimlen( ncid, nVolumesId, &nVolumes ) != NC_NOERR ||
       nc_inq_dimlen( ncid, nVerticesId, &nVertices ) != NC_NOERR ||
       nc_inq_dimlen( ncid, nPointsId, &nPoints ) != NC_NOERR ||
       nc_inq_dimlen( ncid, nTimestepsId, &nTimesteps ) != NC_NOERR )
  {
    nc_close( ncid );
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return std::unique_ptr< MDAL::Mesh >();
  }

  if ( nVertices != 3 )
  {
    MDAL::debug( "Expecting triangular elements!" );
    nc_close( ncid );
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return std::unique_ptr< MDAL::Mesh >();
  }

  int xid, yid, zid, volumesid, timeid, stageid;
  if ( nc_inq_varid( ncid, "x", &xid ) != NC_NOERR ||
       nc_inq_varid( ncid, "y", &yid ) != NC_NOERR ||
       nc_inq_varid( ncid, "volumes", &volumesid ) != NC_NOERR ||
       nc_inq_varid( ncid, "time", &timeid ) != NC_NOERR ||
       nc_inq_varid( ncid, "stage", &stageid ) != NC_NOERR )
  {
    nc_close( ncid );
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return std::unique_ptr< MDAL::Mesh >();
  }

  // load mesh data
  std::vector<float> px( nPoints ), py( nPoints ), pz( nPoints );
  std::vector<int> pvolumes( nVertices * nVolumes );
  if ( nc_get_var_float( ncid, xid, px.data() ) != NC_NOERR ||
       nc_get_var_float( ncid, yid, py.data() ) != NC_NOERR ||
       nc_get_var_int( ncid, volumesid, pvolumes.data() ) != NC_NOERR )
  {
    nc_close( ncid );
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return std::unique_ptr< MDAL::Mesh >();
  }

  // we may need to apply a shift to the X,Y coordinates
  float xLLcorner = 0, yLLcorner = 0;
  nc_get_att_float( ncid, NC_GLOBAL, "xllcorner", &xLLcorner );
  nc_get_att_float( ncid, NC_GLOBAL, "yllcorner", &yLLcorner );

  MDAL::Vertices nodes( nPoints );
  Vertex *nodesPtr = nodes.data();
  for ( size_t i = 0; i < nPoints; ++i, ++nodesPtr )
  {
    nodesPtr->x = static_cast<double>( px[i] + xLLcorner );
    nodesPtr->y = static_cast<double>( py[i] + yLLcorner );
  }

  std::vector<float> times( nTimesteps );
  nc_get_var_float( ncid, timeid, times.data() );

  int zDims = 0;
  if ( nc_inq_varid( ncid, "z", &zid ) == NC_NOERR &&
       nc_get_var_float( ncid, zid, pz.data() ) == NC_NOERR )
  {
    // older SWW format: elevation is constant over time

    zDims = 1;
  }
  else if ( nc_inq_varid( ncid, "elevation", &zid ) == NC_NOERR &&
            nc_inq_varndims( ncid, zid, &zDims ) == NC_NOERR &&
            ( ( zDims == 1 && nc_get_var_float( ncid, zid, pz.data() ) == NC_NOERR ) || zDims == 2 ) )
  {
    // we're good
  }
  else
  {
    // neither "z" nor "elevation" are present -> something is going wrong
    nc_close( ncid );
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return std::unique_ptr< MDAL::Mesh >();
  }

  MDAL::Faces elements( nVolumes );
  for ( size_t i = 0; i < nVolumes; ++i )
  {
    elements[i].resize( 3 );
    elements[i][0] = static_cast<size_t>( pvolumes[3 * i + 0] );
    elements[i][1] = static_cast<size_t>( pvolumes[3 * i + 1] );
    elements[i][2] = static_cast<size_t>( pvolumes[3 * i + 2] );
  }
  std::unique_ptr< MDAL::MemoryMesh > mesh(
    new MemoryMesh(
      name(),
      nodes.size(),
      elements.size(),
      3, // triangles
      computeExtent( nodes ),
      mFileName
    )
  );
  mesh->faces = elements;
  mesh->vertices = nodes;

  // Create a dataset for the bed elevation
  std::shared_ptr<MDAL::DatasetGroup> bedDs = std::make_shared<MDAL::DatasetGroup> (
        name(),
        mesh.get(),
        mFileName,
        "Bed Elevation" );
  bedDs->setIsOnVertices( true );
  bedDs->setIsScalar( true );

  // read bed elevations
  std::vector<std::shared_ptr<MDAL::MemoryDataset>> elevationOutputs;
  if ( zDims == 1 )
  {
    // either "z" or "elevation" with 1 dimension
    std::shared_ptr<MDAL::MemoryDataset> o = std::make_shared<MDAL::MemoryDataset>( bedDs.get() );
    o->setTime( 0.0 );
    double *values = o->values();
    for ( size_t i = 0; i < nPoints; ++i )
    {
      double z = static_cast<double>( pz[i] );
      values[i] = z;
      mesh->vertices[i].z = z;
    }
    o->setStatistics( MDAL::calculateStatistics( o ) );
    bedDs->datasets.push_back( o );
    elevationOutputs.push_back( o );
  }
  else if ( zDims == 2 )
  {
    // newer SWW format: elevation may change over time
    for ( size_t t = 0; t < nTimesteps; ++t )
    {
      std::shared_ptr<MDAL::MemoryDataset> toe = std::make_shared<MDAL::MemoryDataset>( bedDs.get() );
      toe->setTime( static_cast<double>( times[t] ) / 3600. );
      double *elev = toe->values();

      // fetching "elevation" data for one timestep
      size_t start[2], count[2];
      const ptrdiff_t stride[2] = {1, 1};
      start[0] = t;
      start[1] = 0;
      count[0] = 1;
      count[1] = nPoints;
      std::vector<float> buffer( nPoints );
      nc_get_vars_float( ncid, zid, start, count, stride, buffer.data() );
      for ( size_t i = 0; i < nPoints; ++i )
      {
        double val = static_cast<double>( buffer[i] );
        elev[i] = val;
      }

      toe->setStatistics( MDAL::calculateStatistics( toe ) );
      bedDs->datasets.push_back( toe );
      elevationOutputs.push_back( toe );
    }
  }

  bedDs->setStatistics( MDAL::calculateStatistics( bedDs ) );
  mesh->datasetGroups.push_back( bedDs );

  // load results
  std::shared_ptr<MDAL::DatasetGroup> dss = std::make_shared<MDAL::DatasetGroup> (
        name(),
        mesh.get(),
        mFileName,
        "Stage" );
  dss->setIsOnVertices( true );
  dss->setIsScalar( true );

  std::shared_ptr<MDAL::DatasetGroup> dsd = std::make_shared<MDAL::DatasetGroup> (
        name(),
        mesh.get(),
        mFileName,
        "Depth" );
  dsd->setIsOnVertices( true );
  dsd->setIsScalar( true );

  for ( size_t t = 0; t < nTimesteps; ++t )
  {
    const std::shared_ptr<MDAL::MemoryDataset> elevO = elevationOutputs.size() > 1 ? elevationOutputs[t] : elevationOutputs[0];
    const double *elev = elevO->constValues();

    std::shared_ptr<MDAL::MemoryDataset> tos = std::make_shared<MDAL::MemoryDataset>( dss.get() );
    tos->setTime( static_cast<double>( times[t] ) / 3600. );
    double *values = tos->values();

    // fetching "stage" data for one timestep
    size_t start[2], count[2];
    const ptrdiff_t stride[2] = {1, 1};
    start[0] = t;
    start[1] = 0;
    count[0] = 1;
    count[1] = nPoints;
    std::vector<float> buffer( nPoints );
    nc_get_vars_float( ncid, stageid, start, count, stride, buffer.data() );
    for ( size_t i = 0; i < nPoints; ++i )
    {
      double val = static_cast<double>( buffer[i] );
      values[i] = val;
    }

    // derived data: depth = stage - elevation
    std::shared_ptr<MDAL::MemoryDataset> tod = std::make_shared<MDAL::MemoryDataset>( dsd.get() );
    tod->setTime( tos->time() );
    double *depths = tod->values();
    int *activeTos = tos->active();
    int *activeTod = tod->active();

    for ( size_t j = 0; j < nPoints; ++j )
      depths[j] = values[j] - elev[j];

    // determine which elements are active (wet)
    for ( size_t elemidx = 0; elemidx < nVolumes; ++elemidx )
    {
      const Face &elem = mesh->faces[elemidx];
      double v0 = depths[elem[0]];
      double v1 = depths[elem[1]];
      double v2 = depths[elem[2]];
      activeTos[elemidx] = v0 > DEPTH_THRESHOLD && v1 > DEPTH_THRESHOLD && v2 > DEPTH_THRESHOLD;
      activeTod[elemidx] = activeTos[elemidx];
    }

    tos->setStatistics( MDAL::calculateStatistics( tos ) );
    dss->datasets.push_back( tos );

    tod->setStatistics( MDAL::calculateStatistics( tod ) );
    dsd->datasets.push_back( tod );
  }

  dss->setStatistics( MDAL::calculateStatistics( dss ) );
  mesh->datasetGroups.push_back( dss );

  dsd->setStatistics( MDAL::calculateStatistics( dsd ) );
  mesh->datasetGroups.push_back( dsd );


  int momentumxid, momentumyid;
  if ( nc_inq_varid( ncid, "xmomentum", &momentumxid ) == NC_NOERR &&
       nc_inq_varid( ncid, "ymomentum", &momentumyid ) == NC_NOERR )
  {
    std::shared_ptr<MDAL::DatasetGroup> mds = std::make_shared<MDAL::DatasetGroup> (
          name(),
          mesh.get(),
          mFileName,
          "Momentum" );
    mds->setIsOnVertices( true );
    mds->setIsScalar( false );

    std::vector<float> valuesX( nPoints ), valuesY( nPoints );
    for ( size_t t = 0; t < nTimesteps; ++t )
    {
      std::shared_ptr<MDAL::MemoryDataset> mto = std::make_shared<MDAL::MemoryDataset>( mds.get() );
      mto->setTime( static_cast<double>( times[t] ) / 3600. );
      double *values = mto->values();

      std::shared_ptr<MDAL::MemoryDataset> mto0 = std::static_pointer_cast<MDAL::MemoryDataset>( dsd->datasets[t] );
      memcpy( mto->active(), mto0->active(), mesh->facesCount() * sizeof( int ) );

      // fetching "stage" data for one timestep
      size_t start[2], count[2];
      const ptrdiff_t stride[2] = {1, 1};
      start[0] = t;
      start[1] = 0;
      count[0] = 1;
      count[1] = nPoints;
      nc_get_vars_float( ncid, momentumxid, start, count, stride, valuesX.data() );
      nc_get_vars_float( ncid, momentumyid, start, count, stride, valuesY.data() );

      for ( size_t i = 0; i < nPoints; ++i )
      {
        values[2 * i] = static_cast<double>( valuesX[i] );
        values[2 * i + 1] = static_cast<double>( valuesY[i] );
      }

      mto->setStatistics( MDAL::calculateStatistics( mto ) );
      mds->datasets.push_back( mto );
    }


    mds->setStatistics( MDAL::calculateStatistics( mds ) );
    mesh->datasetGroups.push_back( mds );
  }

  nc_close( ncid );

  return std::unique_ptr<Mesh>( mesh.release() );
}
