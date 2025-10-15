/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2016 Lutra Consulting
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include <string>
#include <string.h>
#include <vector>
#include <netcdf.h>
#include <set>

#include "mdal_sww.hpp"
#include "mdal_utils.hpp"
#include "mdal_logger.hpp"

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

size_t MDAL::DriverSWW::getVertexCount( const NetCDFFile &ncFile ) const
{
  int nPointsId;
  size_t res;
  ncFile.getDimension( "number_of_points", &res, &nPointsId );
  return res;
}


bool MDAL::DriverSWW::canReadMesh( const std::string &uri )
{
  NetCDFFile ncFile;

  try
  {
    ncFile.openFile( uri );
    getVertexCount( ncFile );
  }
  catch ( MDAL_Status )
  {
    return false;
  }
  catch ( MDAL::Error & )
  {
    return false;
  }
  return true;
}

std::vector<double> MDAL::DriverSWW::readZCoords( const NetCDFFile &ncFile ) const
{
  size_t nPoints = getVertexCount( ncFile );

  std::vector<double> pz;
  // newer sww files does have elevation array that is time-dependent
  if ( ncFile.hasArr( "z" ) )
  {
    pz = ncFile.readDoubleArr( "z", nPoints );
  }
  else if ( ncFile.hasArr( "elevation" ) )
  {
    int zDims = 0;
    int zid;
    if ( nc_inq_varid( ncFile.handle(), "elevation", &zid ) == NC_NOERR &&
         nc_inq_varndims( ncFile.handle(), zid, &zDims ) == NC_NOERR )
    {
      if ( zDims == 1 )
      {
        // just one elevation for all times, treat as z coord
        pz = ncFile.readDoubleArr( "elevation", nPoints );
      }
      else
      {
        pz.resize( nPoints );
        // fetching "elevation" data for the first timestep,
        // and threat it as z coord
        size_t start[2], count[2];
        const ptrdiff_t stride[2] = {1, 1};
        start[0] = 0; // t = 0
        start[1] = 0;
        count[0] = 1;
        count[1] = nPoints;
        nc_get_vars_double( ncFile.handle(), zid, start, count, stride, pz.data() );
      }
    }
  }

  return pz;
}

void MDAL::DriverSWW::addBedElevation( const NetCDFFile &ncFile,
                                       MDAL::MemoryMesh *mesh,
                                       const std::vector<double> &times
                                     ) const
{
  if ( ncFile.hasArr( "elevation" ) )
  {
    std::shared_ptr<MDAL::DatasetGroup> grp = readScalarGroup( ncFile,
        mesh,
        times,
        "Bed Elevation",
        "elevation" );
    mesh->datasetGroups.emplace_back( std::move( grp ) );
  }
  else
  {
    MDAL::addBedElevationDatasetGroup( mesh, mesh->vertices() );
  }
}

MDAL::Vertices MDAL::DriverSWW::readVertices( const NetCDFFile &ncFile ) const
{
  size_t nPoints = getVertexCount( ncFile );

  // load mesh data
  std::vector<double> px = ncFile.readDoubleArr( "x", nPoints );
  std::vector<double> py = ncFile.readDoubleArr( "y", nPoints );
  std::vector<double> pz = readZCoords( ncFile );

  // we may need to apply a shift to the X,Y coordinates
  double xLLcorner = 0.0;
  if ( ncFile.hasAttrDouble( NC_GLOBAL, "xllcorner" ) )
    xLLcorner = ncFile.getAttrDouble( NC_GLOBAL, "xllcorner" );
  double yLLcorner = 0.0;
  if ( ncFile.hasAttrDouble( NC_GLOBAL, "yllcorner" ) )
    yLLcorner = ncFile.getAttrDouble( NC_GLOBAL, "yllcorner" );

  MDAL::Vertices vertices( nPoints );
  Vertex *vertexPtr = vertices.data();
  for ( size_t i = 0; i < nPoints; ++i, ++vertexPtr )
  {
    vertexPtr->x = px[i] + xLLcorner ;
    vertexPtr->y = py[i] + yLLcorner ;
    if ( !pz.empty() ) // missing both "z" and "elevation"
      vertexPtr->z = pz[i];
  }
  return vertices;
}

MDAL::Faces MDAL::DriverSWW::readFaces( const NetCDFFile &ncFile ) const
{
  // get dimensions
  int nVolumesId, nVerticesId;
  size_t nVolumes, nVertices;
  ncFile.getDimension( "number_of_volumes", &nVolumes, &nVolumesId );
  ncFile.getDimension( "number_of_vertices", &nVertices, &nVerticesId );
  if ( nVertices != 3 ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Number of vertices is not allowed" );

  std::vector<int> pvolumes = ncFile.readIntArr( "volumes", nVertices * nVolumes );

  MDAL::Faces faces( nVolumes );
  for ( size_t i = 0; i < nVolumes; ++i )
  {
    faces[i].resize( 3 );
    faces[i][0] = static_cast<size_t>( pvolumes[3 * i + 0] );
    faces[i][1] = static_cast<size_t>( pvolumes[3 * i + 1] );
    faces[i][2] = static_cast<size_t>( pvolumes[3 * i + 2] );
  }
  return faces;
}

std::vector<double> MDAL::DriverSWW::readTimes( const NetCDFFile &ncFile ) const
{
  size_t nTimesteps;
  int nTimestepsId;
  ncFile.getDimension( "number_of_timesteps", &nTimesteps, &nTimestepsId );
  std::vector<double> times = ncFile.readDoubleArr( "time", nTimesteps );
  return times;
}

void MDAL::DriverSWW::readDatasetGroups(
  const NetCDFFile &ncFile,
  MDAL::MemoryMesh *mesh,
  const std::vector<double> &times
) const
{
  std::set<std::string> parsedVariableNames; // already parsed arrays somewhere else
  parsedVariableNames.insert( "x" );
  parsedVariableNames.insert( "y" );
  parsedVariableNames.insert( "z" );
  parsedVariableNames.insert( "volumes" );
  parsedVariableNames.insert( "time" );

  std::vector<std::string> names = ncFile.readArrNames();
  std::set<std::string> namesSet( names.begin(), names.end() );

  // Add bed elevation group
  parsedVariableNames.insert( "elevations" );
  addBedElevation( ncFile, mesh, times );

  for ( const std::string &name : names )
  {
    // currently we do not support variables like elevation_c, friction_c, stage_c, xmomentum_c, ymomentum_c
    // which contain values per volume instead of per vertex
    if ( MDAL::endsWith( name, "_c" ) )
      continue;

    // skip already parsed variables
    if ( parsedVariableNames.find( name ) == parsedVariableNames.cend() )
    {
      std::string xName, yName, groupName( name );
      bool isVector = parseGroupName( groupName, xName, yName );

      std::shared_ptr<MDAL::DatasetGroup> grp;
      if ( isVector && ncFile.hasArr( xName ) && ncFile.hasArr( yName ) )
      {
        // vector dataset group
        grp = readVectorGroup(
                ncFile,
                mesh,
                times,
                std::move( groupName ),
                xName,
                yName );
        parsedVariableNames.insert( std::move( xName ) );
        parsedVariableNames.insert( std::move( yName ) );
      }
      else
      {
        // scalar dataset group
        grp = readScalarGroup(
                ncFile,
                mesh,
                times,
                std::move( groupName ),
                name );
        parsedVariableNames.insert( name );
      }
      if ( grp )
        mesh->datasetGroups.emplace_back( std::move( grp ) );
    }
  }
}

bool MDAL::DriverSWW::parseGroupName( std::string &groupName,
                                      std::string &xName,
                                      std::string &yName ) const
{
  bool isVector = false;
  std::string baseName( groupName );

  // X and Y variables
  if ( groupName.size() > 1 )
  {
    if ( MDAL::startsWith( groupName, "x" ) )
    {
      baseName = groupName.substr( 1, groupName.size() - 1 );
      xName = groupName;
      yName = "y" + baseName;
      isVector = true;
    }
    else if ( MDAL::startsWith( groupName, "y" ) )
    {
      baseName = groupName.substr( 1, groupName.size() - 1 );
      xName = "x" + baseName;
      yName = groupName;
      isVector = true;
    }
  }

  // Maximums
  groupName = baseName;
  if ( MDAL::endsWith( baseName, "_range" ) )
  {
    groupName = groupName.substr( 0, baseName.size() - 6 ) + "/Maximums";
  }

  return isVector;
}

std::shared_ptr<MDAL::DatasetGroup> MDAL::DriverSWW::readScalarGroup(
  const NetCDFFile &ncFile,
  MDAL::MemoryMesh *mesh,
  const std::vector<double> &times,
  const std::string groupName,
  const std::string arrName
) const
{
  size_t nPoints = getVertexCount( ncFile );
  std::shared_ptr<MDAL::DatasetGroup> mds;

  int varxid;
  if ( nc_inq_varid( ncFile.handle(), arrName.c_str(), &varxid ) == NC_NOERR )
  {
    mds = std::make_shared<MDAL::DatasetGroup> (
            name(),
            mesh,
            mFileName,
            groupName );
    mds->setDataLocation( MDAL_DataLocation::DataOnVertices );
    mds->setIsScalar( true );

    int zDimsX = 0;
    if ( nc_inq_varndims( ncFile.handle(), varxid, &zDimsX ) != NC_NOERR )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Fail while reading scalar group: invalid variable id or bad ncid" );

    if ( zDimsX == 1 )
    {
      // TIME INDEPENDENT
      std::shared_ptr<MDAL::MemoryDataset2D> o = std::make_shared<MDAL::MemoryDataset2D>( mds.get() );
      o->setTime( RelativeTimestamp() );
      std::vector<double> valuesX = ncFile.readDoubleArr( arrName, nPoints );
      for ( size_t i = 0; i < nPoints; ++i )
      {
        o->setScalarValue( i, valuesX[i] );
      }
      o->setStatistics( MDAL::calculateStatistics( o ) );
      mds->datasets.push_back( o );
    }
    else
    {
      // TIME DEPENDENT
      for ( size_t t = 0; t < times.size(); ++t )
      {
        std::shared_ptr<MDAL::MemoryDataset2D> mto = std::make_shared<MDAL::MemoryDataset2D>( mds.get() );
        mto->setTime( static_cast<double>( times[t] ), RelativeTimestamp::seconds ); // Time is always in seconds
        double *values = mto->values();

        // fetching data for one timestep
        size_t start[2], count[2];
        const ptrdiff_t stride[2] = {1, 1};
        start[0] = t;
        start[1] = 0;
        count[0] = 1;
        count[1] = nPoints;
        nc_get_vars_double( ncFile.handle(), varxid, start, count, stride, values );
        mto->setStatistics( MDAL::calculateStatistics( mto ) );
        mds->datasets.push_back( mto );
      }
    }
    mds->setStatistics( MDAL::calculateStatistics( mds ) );
  }

  return mds;
}

std::shared_ptr<MDAL::DatasetGroup> MDAL::DriverSWW::readVectorGroup(
  const NetCDFFile &ncFile,
  MDAL::MemoryMesh *mesh,
  const std::vector<double> &times,
  const std::string groupName,
  const std::string arrXName,
  const std::string arrYName
) const
{
  size_t nPoints = getVertexCount( ncFile );
  std::shared_ptr<MDAL::DatasetGroup> mds;

  int varxid, varyid;
  if ( nc_inq_varid( ncFile.handle(), arrXName.c_str(), &varxid ) == NC_NOERR &&
       nc_inq_varid( ncFile.handle(), arrYName.c_str(), &varyid ) == NC_NOERR )
  {
    mds = std::make_shared<MDAL::DatasetGroup> (
            name(),
            mesh,
            mFileName,
            groupName );
    mds->setDataLocation( MDAL_DataLocation::DataOnVertices );
    mds->setIsScalar( false );

    int zDimsX = 0;
    int zDimsY = 0;
    if ( nc_inq_varndims( ncFile.handle(), varxid, &zDimsX ) != NC_NOERR )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Fail while reading vector group: invalid variable id or bad ncid" );

    if ( nc_inq_varndims( ncFile.handle(), varyid, &zDimsY ) != NC_NOERR )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Fail while reading vector group: invalid variable id or bad ncid" );

    if ( zDimsX != zDimsY )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Fail while reading vector group: dimensions do not match" );

    if ( zDimsX == 1 )
    {
      // TIME INDEPENDENT
      std::shared_ptr<MDAL::MemoryDataset2D> o = std::make_shared<MDAL::MemoryDataset2D>( mds.get() );
      o->setTime( 0.0 );
      std::vector<double> valuesX = ncFile.readDoubleArr( arrXName, nPoints );
      std::vector<double> valuesY = ncFile.readDoubleArr( arrYName, nPoints );
      for ( size_t i = 0; i < nPoints; ++i )
      {
        o->setVectorValue( i, valuesX[i], valuesY[i] );
      }
      o->setStatistics( MDAL::calculateStatistics( o ) );
      mds->datasets.push_back( o );
    }
    else
    {
      std::vector<double> valuesX( nPoints ), valuesY( nPoints );
      // TIME DEPENDENT
      for ( size_t t = 0; t < times.size(); ++t )
      {
        std::shared_ptr<MDAL::MemoryDataset2D> mto = std::make_shared<MDAL::MemoryDataset2D>( mds.get() );
        mto->setTime( static_cast<double>( times[t] ) / 3600. );

        // fetching data for one timestep
        size_t start[2], count[2];
        const ptrdiff_t stride[2] = {1, 1};
        start[0] = t;
        start[1] = 0;
        count[0] = 1;
        count[1] = nPoints;
        nc_get_vars_double( ncFile.handle(), varxid, start, count, stride, valuesX.data() );
        nc_get_vars_double( ncFile.handle(), varyid, start, count, stride, valuesY.data() );

        for ( size_t i = 0; i < nPoints; ++i )
        {
          mto->setVectorValue( i, static_cast<double>( valuesX[i] ),  static_cast<double>( valuesY[i] ) );
        }

        mto->setStatistics( MDAL::calculateStatistics( mto ) );
        mds->datasets.push_back( mto );
      }
    }
    mds->setStatistics( MDAL::calculateStatistics( mds ) );
  }

  return mds;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverSWW::load(
  const std::string &resultsFile,
  const std::string & )
{
  mFileName = resultsFile;
  MDAL::Log::resetLastStatus();

  NetCDFFile ncFile;

  try
  {
    // Open file for reading
    ncFile.openFile( mFileName );

    // Read mesh
    MDAL::Vertices vertices = readVertices( ncFile );
    MDAL::Faces faces = readFaces( ncFile );
    std::unique_ptr< MDAL::MemoryMesh > mesh(
      new MemoryMesh(
        name(),
        3, // triangles
        mFileName
      )
    );
    mesh->setFaces( std::move( faces ) );
    mesh->setVertices( std::move( vertices ) );

    // Read times
    std::vector<double> times = readTimes( ncFile );

    // Create a dataset(s)
    readDatasetGroups( ncFile, mesh.get(), times );

    // Success!
    return std::unique_ptr<Mesh>( mesh.release() );
  }
  catch ( MDAL_Status err )
  {
    MDAL::Log::error( err, "Error while loading file " + resultsFile );
    return std::unique_ptr< MDAL::Mesh >();
  }
  catch ( MDAL::Error &err )
  {
    MDAL::Log::error( err, name() );
    return std::unique_ptr< MDAL::Mesh >();
  }
}
