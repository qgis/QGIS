/*
 MDAL - mMesh Data Abstraction Library (MIT License)
 Copyright (C) 2016 Lutra Consulting
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_flo2d.hpp"
#include <vector>
#include <map>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstring>

#include "mdal_utils.hpp"
#include "mdal_hdf5.hpp"

struct VertexCompare
{
  bool operator()( const MDAL::Vertex &lhs, const MDAL::Vertex &rhs ) const
  {
    double resX = 0;
    resX += lhs.x * 1000000;
    resX += lhs.y * 1000;

    double resY = 0;
    resY += rhs.x * 1000000;
    resY += rhs.y * 1000;

    return resX < resY;
  }
};

static inline bool is_nodata( double val )
{
  return MDAL::equals( val, -9999.0, 1e-8 );
}

static std::string fileNameFromDir( const std::string &mainFileName, const std::string &name )
{
  std::string dir = MDAL::dirName( mainFileName );
  return MDAL::pathJoin( dir, name );
}

static double getDouble( double val )
{
  if ( is_nodata( val ) )
  {
    return MDAL_NAN;
  }
  else
  {
    return val;
  }
}

static double getDouble( const std::string &val )
{
  if ( MDAL::isNumber( val ) )
  {
    double valF = MDAL::toDouble( val );
    return getDouble( valF );
  }
  else
  {
    return MDAL_NAN;
  }
}

void MDAL::DriverFlo2D::addStaticDataset(
  bool isOnVertices,
  std::vector<double> &vals,
  const std::string &groupName,
  const std::string &datFileName )
{
  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                          name(),
                                          mMesh.get(),
                                          datFileName,
                                          groupName
                                        );
  group->setIsOnVertices( isOnVertices );
  group->setIsScalar( true );

  std::shared_ptr<MDAL::MemoryDataset> dataset = std::make_shared< MemoryDataset >( group.get() );
  assert( vals.size() == dataset->valuesCount() );
  dataset->setTime( 0.0 );
  double *values = dataset->values();
  memcpy( values, vals.data(), vals.size() * sizeof( double ) );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mMesh->datasetGroups.push_back( group );
}

void MDAL::DriverFlo2D::parseCADPTSFile( const std::string &datFileName, std::vector<CellCenter> &cells )
{
  std::string cadptsFile( fileNameFromDir( datFileName, "CADPTS.DAT" ) );
  if ( !MDAL::fileExists( cadptsFile ) )
  {
    throw MDAL_Status::Err_FileNotFound;
  }

  std::ifstream cadptsStream( cadptsFile, std::ifstream::in );
  std::string line;
  // CADPTS.DAT - COORDINATES OF CELL CENTERS (ELEM NUM, X, Y)
  while ( std::getline( cadptsStream, line ) )
  {
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != 3 )
    {
      throw MDAL_Status::Err_UnknownFormat;
    }
    CellCenter cc;
    cc.id = MDAL::toSizeT( lineParts[1] ) - 1; //numbered from 1
    cc.x = MDAL::toDouble( lineParts[1] );
    cc.y = MDAL::toDouble( lineParts[2] );
    cc.conn.resize( 4 );
    cells.push_back( cc );
  }
}

void MDAL::DriverFlo2D::parseFPLAINFile( std::vector<double> &elevations,
    const std::string &datFileName,
    std::vector<CellCenter> &cells )
{
  elevations.clear();
  // FPLAIN.DAT - CONNECTIVITY (ELEM NUM, ELEM N, ELEM E, ELEM S, ELEM W, MANNING-N, BED ELEVATION)
  std::string fplainFile( fileNameFromDir( datFileName, "FPLAIN.DAT" ) );
  if ( !MDAL::fileExists( fplainFile ) )
  {
    throw MDAL_Status::Err_FileNotFound;
  }

  std::ifstream fplainStream( fplainFile, std::ifstream::in );
  std::string line;

  while ( std::getline( fplainStream, line ) )
  {
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != 7 )
    {
      throw MDAL_Status::Err_UnknownFormat;
    }
    size_t cc_i = MDAL::toSizeT( lineParts[0] ) - 1; //numbered from 1
    for ( size_t j = 0; j < 4; ++j )
    {
      cells[cc_i].conn[j] = MDAL::toInt( lineParts[j + 1] ) - 1; //numbered from 1, 0 boundary Vertex
    }
    elevations.push_back( MDAL::toDouble( lineParts[6] ) );
  }
}

static void addDatasetToGroup( std::shared_ptr<MDAL::DatasetGroup> group, std::shared_ptr<MDAL::MemoryDataset> dataset )
{
  if ( group && dataset && dataset->valuesCount() > 0 )
  {
    dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
    group->datasets.push_back( dataset );
  }
}

void MDAL::DriverFlo2D::parseTIMDEPFile( const std::string &datFileName, const std::vector<double> &elevations )
{
  // TIMDEP.OUT
  // this file is optional, so if not present, reading is skipped
  // time (separate line)
  // For every Vertex:
  // FLO2D: ELEM NUMber (indexed from 1), depth, velocity, velocity x, velocity y
  // FLO2DPro: ELEM NUMber (indexed from 1), depth, velocity, velocity x, velocity y, water surface elevation
  std::string inFile( fileNameFromDir( datFileName, "TIMDEP.OUT" ) );
  if ( !MDAL::fileExists( inFile ) )
  {
    return;
  }

  std::ifstream inStream( inFile, std::ifstream::in );
  std::string line;

  size_t nVertexs = mMesh->verticesCount();
  size_t ntimes = 0;

  double time = 0.0;
  size_t face_idx = 0;

  std::shared_ptr<DatasetGroup> depthDsGroup = std::make_shared< DatasetGroup >(
        name(),
        mMesh.get(),
        datFileName,
        "Depth"
      );
  depthDsGroup->setIsOnVertices( false );
  depthDsGroup->setIsScalar( true );


  std::shared_ptr<DatasetGroup> waterLevelDsGroup = std::make_shared< DatasetGroup >(
        name(),
        mMesh.get(),
        datFileName,
        "Water Level"
      );
  waterLevelDsGroup->setIsOnVertices( false );
  waterLevelDsGroup->setIsScalar( true );

  std::shared_ptr<DatasetGroup> flowDsGroup = std::make_shared< DatasetGroup >(
        name(),
        mMesh.get(),
        datFileName,
        "Velocity"
      );
  flowDsGroup->setIsOnVertices( false );
  flowDsGroup->setIsScalar( false );

  std::shared_ptr<MDAL::MemoryDataset> flowDataset;
  std::shared_ptr<MDAL::MemoryDataset> depthDataset;
  std::shared_ptr<MDAL::MemoryDataset> waterLevelDataset;

  while ( std::getline( inStream, line ) )
  {
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() == 1 )
    {
      time = MDAL::toDouble( line );
      ntimes++;

      if ( depthDataset ) addDatasetToGroup( depthDsGroup, depthDataset );
      if ( flowDataset ) addDatasetToGroup( flowDsGroup, flowDataset );
      if ( waterLevelDataset ) addDatasetToGroup( waterLevelDsGroup, waterLevelDataset );

      depthDataset  = std::make_shared< MemoryDataset >( depthDsGroup.get() );
      flowDataset = std::make_shared< MemoryDataset >( flowDsGroup.get() );
      waterLevelDataset = std::make_shared< MemoryDataset >( waterLevelDsGroup.get() );

      depthDataset->setTime( time );
      flowDataset->setTime( time );
      waterLevelDataset->setTime( time );

      face_idx = 0;

    }
    else if ( ( lineParts.size() == 5 ) || ( lineParts.size() == 6 ) )
    {
      // new Vertex for time
      if ( !depthDataset || !flowDataset || !waterLevelDataset ) throw MDAL_Status::Err_UnknownFormat;
      if ( face_idx == nVertexs ) throw MDAL_Status::Err_IncompatibleMesh;

      // this is magnitude: getDouble(lineParts[2]);
      flowDataset->values()[2 * face_idx] = getDouble( lineParts[3] );
      flowDataset->values()[2 * face_idx + 1] = getDouble( lineParts[4] );

      double depth = getDouble( lineParts[1] );
      depthDataset->values()[face_idx] = depth;

      if ( !is_nodata( depth ) ) depth += elevations[face_idx];
      waterLevelDataset->values()[face_idx] = depth;

      face_idx ++;

    }
    else
    {
      throw MDAL_Status::Err_UnknownFormat;
    }
  }

  if ( depthDataset ) addDatasetToGroup( depthDsGroup, depthDataset );
  if ( flowDataset ) addDatasetToGroup( flowDsGroup, flowDataset );
  if ( waterLevelDataset ) addDatasetToGroup( waterLevelDsGroup, waterLevelDataset );

  depthDsGroup->setStatistics( MDAL::calculateStatistics( depthDsGroup ) );
  flowDsGroup->setStatistics( MDAL::calculateStatistics( flowDsGroup ) );
  waterLevelDsGroup->setStatistics( MDAL::calculateStatistics( waterLevelDsGroup ) );

  mMesh->datasetGroups.push_back( depthDsGroup );
  mMesh->datasetGroups.push_back( flowDsGroup );
  mMesh->datasetGroups.push_back( waterLevelDsGroup );
}


void MDAL::DriverFlo2D::parseDEPTHFile( const std::string &datFileName, const std::vector<double> &elevations )
{
  // this file is optional, so if not present, reading is skipped
  std::string depthFile( fileNameFromDir( datFileName, "DEPTH.OUT" ) );
  if ( !MDAL::fileExists( depthFile ) )
  {
    return; //optional file
  }

  std::ifstream depthStream( depthFile, std::ifstream::in );
  std::string line;

  size_t nVertices = mMesh->verticesCount();
  std::vector<double> maxDepth( nVertices );
  std::vector<double> maxWaterLevel( nVertices );

  size_t vertex_idx = 0;

  // DEPTH.OUT - COORDINATES (ELEM NUM, X, Y, MAX DEPTH)
  while ( std::getline( depthStream, line ) )
  {
    if ( vertex_idx == nVertices ) throw MDAL_Status::Err_IncompatibleMesh;

    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != 4 )
    {
      throw MDAL_Status::Err_UnknownFormat;
    }

    double val = getDouble( lineParts[3] );
    maxDepth[vertex_idx] = val;

    //water level
    if ( !is_nodata( val ) ) val += elevations[vertex_idx];
    maxWaterLevel[vertex_idx] = val;


    vertex_idx++;
  }

  addStaticDataset( true, maxDepth, "Depth/Maximums", datFileName );
  addStaticDataset( true, maxWaterLevel, "Water Level/Maximums", datFileName );
}


void MDAL::DriverFlo2D::parseVELFPVELOCFile( const std::string &datFileName )
{
  // these files are optional, so if not present, reading is skipped
  size_t nVertices = mMesh->verticesCount();
  std::vector<double> maxVel( nVertices );

  {
    std::string velocityFile( fileNameFromDir( datFileName, "VELFP.OUT" ) );
    if ( !MDAL::fileExists( velocityFile ) )
    {
      return; //optional file
    }

    std::ifstream velocityStream( velocityFile, std::ifstream::in );
    std::string line;

    size_t vertex_idx = 0;

    // VELFP.OUT - COORDINATES (ELEM NUM, X, Y, MAX VEL) - Maximum floodplain flow velocity;
    while ( std::getline( velocityStream, line ) )
    {
      if ( vertex_idx == nVertices ) throw MDAL_Status::Err_IncompatibleMesh;

      std::vector<std::string> lineParts = MDAL::split( line, ' ' );
      if ( lineParts.size() != 4 )
      {
        throw MDAL_Status::Err_UnknownFormat;
      }

      double val = getDouble( lineParts[3] );
      maxVel[vertex_idx] = val;

      vertex_idx++;
    }
  }

  {
    std::string velocityFile( fileNameFromDir( datFileName, "VELOC.OUT" ) );
    if ( !MDAL::fileExists( velocityFile ) )
    {
      return; //optional file
    }

    std::ifstream velocityStream( velocityFile, std::ifstream::in );
    std::string line;

    size_t vertex_idx = 0;

    // VELOC.OUT - COORDINATES (ELEM NUM, X, Y, MAX VEL)  - Maximum channel flow velocity
    while ( std::getline( velocityStream, line ) )
    {
      if ( vertex_idx == nVertices ) throw MDAL_Status::Err_IncompatibleMesh;

      std::vector<std::string> lineParts = MDAL::split( line, ' ' );
      if ( lineParts.size() != 4 )
      {
        throw MDAL_Status::Err_UnknownFormat;
      }

      double val = getDouble( lineParts[3] );
      if ( !is_nodata( val ) ) // overwrite value from VELFP if it is not 0
      {
        maxVel[vertex_idx] = val;
      }

      vertex_idx++;
    }
  }

  addStaticDataset( true, maxVel, "Velocity/Maximums", datFileName );
}

double MDAL::DriverFlo2D::calcCellSize( const std::vector<CellCenter> &cells )
{
  // find first cell that is not izolated from the others
  // and return its distance to the neighbor's cell center
  for ( size_t i = 0; i < cells.size(); ++i )
  {
    for ( size_t j = 0; j < 4; ++j )
    {
      int idx = cells[i].conn[0];
      if ( idx > -1 )
      {
        if ( ( j == 0 ) || ( j == 2 ) )
        {
          return fabs( cells[static_cast<size_t>( idx )].y - cells[i].y );
        }
        else
        {
          return fabs( cells[static_cast<size_t>( idx )].x - cells[i].x );
        }
      }
    }
  }
  throw MDAL_Status::Err_IncompatibleMesh;
}

MDAL::Vertex MDAL::DriverFlo2D::createVertex( size_t position, double half_cell_size, const CellCenter &cell )
{
  MDAL::Vertex n;
  n.x = cell.x;
  n.y = cell.y;

  switch ( position )
  {
    case 0:
      n.x += half_cell_size;
      n.y -= half_cell_size;
      break;

    case 1:
      n.x += half_cell_size;
      n.y += half_cell_size;
      break;

    case 2:
      n.x -= half_cell_size;
      n.y += half_cell_size;
      break;

    case 3:
      n.x -= half_cell_size;
      n.y -= half_cell_size;
      break;
  }

  return n;
}

void MDAL::DriverFlo2D::createMesh( const std::vector<CellCenter> &cells, double half_cell_size )
{
  // Create all Faces from cell centers.
  // Vertexs must be also created, they are not stored in FLO-2D files
  // try to reuse Vertexs already created for other Faces by usage of unique_Vertexs set.
  Faces faces;
  Vertices vertices;
  std::map<Vertex, size_t, VertexCompare> unique_vertices; //vertex -> id
  size_t vertex_idx = 0;

  for ( size_t i = 0; i < cells.size(); ++i )
  {
    Face e( 4 );

    for ( size_t position = 0; position < 4; ++position )
    {
      Vertex n = createVertex( position, half_cell_size, cells[i] );
      const auto iter = unique_vertices.find( n );
      if ( iter == unique_vertices.end() )
      {
        unique_vertices[n] = vertex_idx;
        vertices.push_back( n );
        e[position] = vertex_idx;
        ++vertex_idx;
      }
      else
      {
        e[position] = iter->second;
      }
    }

    faces.push_back( e );
  }

  mMesh.reset(
    new MemoryMesh(
      name(),
      vertices.size(),
      faces.size(),
      4, //maximum quads
      computeExtent( vertices ),
      mDatFileName
    )
  );
  mMesh->faces = faces;
  mMesh->vertices = vertices;
}

bool MDAL::DriverFlo2D::isFlo2DFile( const std::string &fileName )
{
  std::vector<std::string> required_files =
  {
    "CADPTS.DAT",
    "FPLAIN.DAT"
  };

  for ( const std::string &str : required_files )
  {
    std::string fn( fileNameFromDir( fileName, str ) );
    if ( !fileExists( fn ) )
      return false;
  }
  return true;
}

bool MDAL::DriverFlo2D::parseHDF5Datasets( const std::string &datFileName )
{
  //return true on error

  size_t nFaces =  mMesh->facesCount();

  std::string timedepFileName = fileNameFromDir( datFileName, "TIMDEP.HDF5" );
  if ( !fileExists( timedepFileName ) ) return true;

  HdfFile file( timedepFileName );
  if ( !file.isValid() ) return true;

  HdfGroup timedataGroup = file.group( "TIMDEP NETCDF OUTPUT RESULTS" );
  if ( !timedataGroup.isValid() ) return true;

  std::vector<std::string> groupNames = timedataGroup.groups();

  for ( const std::string &grpName : groupNames )
  {
    HdfGroup grp = timedataGroup.group( grpName );
    if ( !grp.isValid() ) return true;

    HdfAttribute groupType = grp.attribute( "Grouptype" );
    if ( !groupType.isValid() ) return true;

    /* Min and Max arrays in TIMDEP.HDF5 files have dimensions 1xntimesteps .
        HdfDataset minDs = grp.dataset("Mins");
        if (!minDs.isValid()) return true;

        HdfDataset maxDs = grp.dataset("Maxs");
        if (!maxDs.isValid()) return true;
    */

    HdfDataset timesDs = grp.dataset( "Times" );
    if ( !timesDs.isValid() ) return true;
    size_t timesteps = timesDs.elementCount();

    HdfDataset valuesDs = grp.dataset( "Values" );
    if ( !valuesDs.isValid() ) return true;

    bool isVector = MDAL::contains( groupType.readString(), "vector", ContainsBehaviour::CaseInsensitive );

    // Some sanity checks
    size_t expectedSize = mMesh->facesCount() * timesteps;
    if ( isVector ) expectedSize *= 2;
    if ( valuesDs.elementCount() != expectedSize ) return true;

    // Read data
    std::vector<double> times = timesDs.readArrayDouble();
    std::vector<float> values = valuesDs.readArray();

    // Create dataset now
    std::shared_ptr<DatasetGroup> ds = std::make_shared< DatasetGroup >(
                                         name(),
                                         mMesh.get(),
                                         datFileName,
                                         grpName
                                       );
    ds->setIsOnVertices( false );
    ds->setIsScalar( !isVector );

    for ( size_t ts = 0; ts < timesteps; ++ts )
    {
      std::shared_ptr< MemoryDataset > output = std::make_shared< MemoryDataset >( ds.get() );
      output->setTime( times[ts] );

      if ( isVector )
      {
        // vector
        for ( size_t i = 0; i < nFaces; ++i )
        {
          size_t idx = 2 * ( ts * nFaces + i );
          double x = getDouble( static_cast<double>( values[idx] ) );
          double y = getDouble( static_cast<double>( values[idx + 1] ) );
          output->values()[2 * i] = x;
          output->values()[2 * i + 1] = y;
        }
      }
      else
      {
        // scalar
        for ( size_t i = 0; i < nFaces; ++i )
        {
          size_t idx = ts * nFaces + i;
          double val = getDouble( static_cast<double>( values[idx] ) );
          output->values()[i] = val;
        }
      }
      addDatasetToGroup( ds, output );
    }

    // TODO use mins & maxs arrays
    ds->setStatistics( MDAL::calculateStatistics( ds ) );
    mMesh->datasetGroups.push_back( ds );

  }

  return false;
}

void MDAL::DriverFlo2D::parseOUTDatasets( const std::string &datFileName, const std::vector<double> &elevations )
{
  // Create Depth and Velocity datasets Time varying datasets
  parseTIMDEPFile( datFileName, elevations );

  // Maximum Depth and Water Level
  parseDEPTHFile( datFileName, elevations );

  // Maximum Velocity
  parseVELFPVELOCFile( datFileName );
}

MDAL::DriverFlo2D::DriverFlo2D()
  : Driver(
      "FLO2D",
      "Flo2D",
      "*.nc",
      Capability::ReadMesh )
{

}

MDAL::DriverFlo2D *MDAL::DriverFlo2D::create()
{
  return new DriverFlo2D();
}

bool MDAL::DriverFlo2D::canRead( const std::string &uri )
{
  std::string cadptsFile( fileNameFromDir( uri, "CADPTS.DAT" ) );
  if ( !MDAL::fileExists( cadptsFile ) )
  {
    return false;
  }

  std::string fplainFile( fileNameFromDir( uri, "FPLAIN.DAT" ) );
  if ( !MDAL::fileExists( fplainFile ) )
  {
    return false;
  }

  return true;
}

std::unique_ptr< MDAL::Mesh > MDAL::DriverFlo2D::load( const std::string &resultsFile, MDAL_Status *status )
{
  mDatFileName = resultsFile;
  if ( status ) *status = MDAL_Status::None;
  mMesh.reset();
  std::vector<CellCenter> cells;

  try
  {
    // Parse mMesh info
    parseCADPTSFile( mDatFileName, cells );
    std::vector<double> elevations;
    parseFPLAINFile( elevations, mDatFileName, cells );
    double cell_size = calcCellSize( cells );

    // Create mMesh
    createMesh( cells, cell_size / 2.0 );

    // create output for bed elevation
    addStaticDataset( false, elevations, "Bed Elevation", mDatFileName );

    if ( parseHDF5Datasets( mDatFileName ) )
    {
      // some problem with HDF5 data, try text files
      parseOUTDatasets( mDatFileName, elevations );
    }
  }

  catch ( MDAL_Status error )
  {
    if ( status ) *status = ( error );
    mMesh.reset();
  }

  return std::unique_ptr<Mesh>( mMesh.release() );
}
