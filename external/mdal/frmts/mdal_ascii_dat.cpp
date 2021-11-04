/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Ltd.
*/

#include <stddef.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <memory>
#include <limits>

#include "mdal_ascii_dat.hpp"
#include "mdal_utils.hpp"
#include "mdal_2dm.hpp"
#include "mdal.h"
#include "mdal_logger.hpp"

#include <math.h>

#define EXIT_WITH_ERROR(error, mssg)       {  MDAL::Log::errorf( error, "ASCII_DAT", mssg); return; }

MDAL::DriverAsciiDat::DriverAsciiDat( ):
  Driver( "ASCII_DAT",
          "DAT",
          "*.dat",
          Capability::ReadDatasets | Capability::WriteDatasetsOnFaces |
          Capability::WriteDatasetsOnVertices | Capability::WriteDatasetsOnEdges
        )
{
}

MDAL::DriverAsciiDat *MDAL::DriverAsciiDat::create()
{
  return new DriverAsciiDat();
}

MDAL::DriverAsciiDat::~DriverAsciiDat( ) = default;

bool MDAL::DriverAsciiDat::canReadDatasets( const std::string &uri )
{
  std::ifstream in = MDAL::openInputFile( uri );
  std::string line;
  if ( !MDAL::getHeaderLine( in, line ) )
  {
    return false;
  }
  line = trim( line );

  return canReadNewFormat( line ) || canReadOldFormat( line );
}

bool MDAL::DriverAsciiDat::canReadOldFormat( const std::string &line ) const
{
  return MDAL::contains( line, "SCALAR" ) ||
         MDAL::contains( line, "VECTOR" ) ||
         MDAL::contains( line, "TS" ) ||
         MDAL::contains( line, "TIMEUNITS" );
}

bool MDAL::DriverAsciiDat::canReadNewFormat( const std::string &line ) const
{
  return line == "DATASET";
}


void MDAL::DriverAsciiDat::loadOldFormat( std::ifstream &in,
    Mesh *mesh ) const
{
  std::shared_ptr<DatasetGroup> group; // DAT outputs data
  std::string groupName( MDAL::baseName( mDatFile ) );
  std::string line;
  std::getline( in, line );

// Read the first line
  bool isVector = MDAL::contains( line, "VECTOR" );
  group = std::make_shared< DatasetGroup >(
            name(),
            mesh,
            mDatFile,
            groupName
          );
  group->setIsScalar( !isVector );
  group->setDataLocation( MDAL_DataLocation::DataOnVertices );
  MDAL::RelativeTimestamp::Unit timeUnits = MDAL::RelativeTimestamp::hours;
  do
  {
    // Replace tabs by spaces,
    // since basement v.2.8 uses tabs instead of spaces (e.g. 'TS 0\t0.0')
    line = replace( line, "\t", " " );

    // Trim string for cases when file has inconsistent new line symbols
    line = MDAL::trim( line );

    // Split to tokens
    std::vector<std::string> items = split( line,  ' ' );
    if ( items.size() < 1 )
      continue; // empty line?? let's skip it

    std::string cardType = items[0];
    if ( cardType == "ND" && items.size() >= 2 )
    {
      size_t fileNodeCount = toSizeT( items[1] );
      size_t meshIdCount = maximumId( mesh ) + 1;
      if ( meshIdCount != fileNodeCount )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), "Loading old format failed" );
        return;
      }
    }
    else if ( cardType == "SCALAR" || cardType == "VECTOR" )
    {
      // just ignore - we know the type from earlier...
    }
    else if ( cardType == "TIMEUNITS" && items.size() >= 2 )
    {
      timeUnits = MDAL::parseDurationTimeUnit( items[1] );
    }
    else if ( cardType == "TS" && items.size() >=  2 )
    {
      double rawTime = toDouble( items[ 1 ] );
      MDAL::RelativeTimestamp t( rawTime, timeUnits );
      readVertexTimestep( mesh, group, t, isVector, false, in );
    }
    else
    {
      std::stringstream str;
      str << " Unknown card:" << line;
      MDAL::Log::debug( str.str() );
    }
  }
  while ( std::getline( in, line ) );

  if ( !group || group->datasets.size() == 0 )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "Dataset group is not valid (null) or has zero datasets" );
    return;
  }

  group->setStatistics( MDAL::calculateStatistics( group ) );
  mesh->datasetGroups.push_back( group );
  group.reset();
}

void MDAL::DriverAsciiDat::loadNewFormat(
  std::ifstream &in,
  Mesh *mesh ) const
{
  bool isVector = false;
  MDAL_DataLocation dataLocation = MDAL_DataLocation::DataOnVertices;
  std::shared_ptr<DatasetGroup> group; // DAT outputs data
  std::string groupName( MDAL::baseName( mDatFile ) );
  std::string line;
  MDAL::DateTime referenceTime;
  // see if it contains face-centered results - supported by BASEMENT
  // or results defined on edges
  if ( contains( groupName, "_els" ) )
  {
    if ( ( mesh->facesCount() > 0 ) && ( mesh->edgesCount() > 0 ) )
    {
      // unable to read for mixed 1D, 2D mesh
      MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), "unable to read for mixed 1D, 2D mesh" );
      return;
    }

    if ( mesh->facesCount() > 0 )
      dataLocation = MDAL_DataLocation::DataOnFaces;
    else
      dataLocation = MDAL_DataLocation::DataOnEdges;
  }

  while ( std::getline( in, line ) )
  {
    // Replace tabs by spaces,
    // since basement v.2.8 uses tabs instead of spaces (e.g. 'TS 0\t0.0')
    line = replace( line, "\t", " " );

    // Trim string for cases when file has inconsistent new line symbols
    line = MDAL::trim( line );

    // Split to tokens
    std::vector<std::string> items = split( line,  ' ' );
    if ( items.size() < 1 )
      continue; // empty line?? let's skip it

    std::string cardType = items[0];
    if ( cardType == "ND" && items.size() >= 2 )
    {
      size_t fileNodeCount = toSizeT( items[1] );
      size_t meshIdCount = maximumId( mesh ) + 1;
      if ( meshIdCount != fileNodeCount )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), "IDs in mesh and nodes in file are not equal" );
        return;
      }

    }
    else if ( cardType == "NC" && items.size() >= 2 )
    {
      size_t fileElemCount = toSizeT( items[1] );
      size_t elemCount = mesh->facesCount() + mesh->edgesCount();
      if ( elemCount != fileElemCount )
      {
        MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "Elements in mesh and elements in file are not equal" );
        return;
      }
    }
    else if ( cardType == "OBJTYPE" )
    {
      if ( items[1] != "mesh2d" && items[1] != "\"mesh2d\"" )
      {
        MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "Error in file" );
        return;
      }
    }
    else if ( cardType == "BEGSCL" || cardType == "BEGVEC" )
    {
      if ( group )
      {
        MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "New dataset while previous one is still active!" );
        return;
      }
      isVector = cardType == "BEGVEC";

      group = std::make_shared< DatasetGroup >(
                name(),
                mesh,
                mDatFile,
                groupName
              );
      group->setIsScalar( !isVector );
      group->setDataLocation( dataLocation );
      group->setReferenceTime( referenceTime );
    }
    else if ( cardType == "ENDDS" )
    {
      if ( !group )
      {
        MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "ENDDS card for no active dataset!" );
        return;
      }
      group->setStatistics( MDAL::calculateStatistics( group ) );
      mesh->datasetGroups.push_back( group );
      group.reset();
    }
    else if ( cardType == "NAME" && items.size() >= 2 )
    {
      if ( !group )
      {
        MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "NAME card for no active dataset!" );
        return;
      }

      size_t quoteIdx1 = line.find( '\"' );
      size_t quoteIdx2 = line.find( '\"', quoteIdx1 + 1 );
      if ( quoteIdx1 != std::string::npos && quoteIdx2 != std::string::npos )
        group->setName( line.substr( quoteIdx1 + 1, quoteIdx2 - quoteIdx1 - 1 ) );
    }
    else if ( cardType == "RT_JULIAN" && items.size() >= 2 )
    {
      referenceTime = DateTime( MDAL::toDouble( items[1] ), DateTime::JulianDay );
    }
    else if ( cardType == "TIMEUNITS" && items.size() >= 2 )
    {
      if ( !group )
      {
        MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "TIMEUNITS card for no active dataset!" );
        return;
      }

      group->setMetadata( "TIMEUNITS", items[1] );
    }
    else if ( cardType == "TS" && items.size() >= 3 )
    {
      double rawTime = toDouble( items[2] );
      MDAL::RelativeTimestamp t( rawTime, MDAL::parseDurationTimeUnit( group->getMetadata( "TIMEUNITS" ) ) );

      if ( dataLocation != MDAL_DataLocation::DataOnVertices )
      {
        readElementTimestep( mesh, group, t, isVector, in );
      }
      else
      {
        bool hasStatus = ( toBool( items[1] ) );
        readVertexTimestep( mesh, group, t, isVector, hasStatus, in );
      }

    }
    else
    {
      std::stringstream str;
      str << " Unknown card:" << line;
      MDAL::Log::debug( str.str() );
    }
  }
}

size_t MDAL::DriverAsciiDat::maximumId( const MDAL::Mesh *mesh ) const
{
  const Mesh2dm *m2dm = dynamic_cast<const Mesh2dm *>( mesh );
  if ( m2dm )
    return m2dm->maximumVertexId();
  else
    return mesh->verticesCount() - 1;
}

/**
 * The DAT format contains "datasets" and each dataset has N-outputs. One output
 * represents data for all vertices/faces for one timestep
 *
 * In MDAL we convert one output to one MDAL dataset;
 *
 */
void MDAL::DriverAsciiDat::load( const std::string &datFile, MDAL::Mesh *mesh )
{
  mDatFile = datFile;
  MDAL::Log::resetLastStatus();

  if ( !MDAL::fileExists( mDatFile ) )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, name(), "could not find file " + datFile );
    return;
  }

  size_t mID = maximumId( mesh );
  if ( mID == std::numeric_limits<size_t>::max() )
  {
    // This happens when mesh is 2DM and vertices are numbered from 0
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), "mesh is 2DM and vertices are numbered from 0" );
    return;
  }

  std::ifstream in = MDAL::openInputFile( mDatFile );
  std::string line;
  if ( !std::getline( in, line ) )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), "could not read file " +  mDatFile );
    return;
  }
  line = trim( line );
  if ( canReadNewFormat( line ) )
  {
    // we do not need to parse first line again
    loadNewFormat( in, mesh );
  }
  else
  {
    // we need to parse first line again to see
    // scalar/vector flag or timestep flag
    in.clear();
    in.seekg( 0 );
    loadOldFormat( in, mesh );
  }
}

void MDAL::DriverAsciiDat::readVertexTimestep(
  const MDAL::Mesh *mesh,
  std::shared_ptr<DatasetGroup> group,
  MDAL::RelativeTimestamp t,
  bool isVector,
  bool hasStatus,
  std::ifstream &stream ) const
{
  assert( group );
  size_t faceCount = mesh->facesCount();
  size_t vertexCount = mesh->verticesCount();

  std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MDAL::MemoryDataset2D >( group.get(), hasStatus );
  dataset->setTime( t );

  // only for new format
  for ( size_t i = 0; i < faceCount; ++i )
  {
    if ( hasStatus )
    {
      std::string line;
      std::getline( stream, line );
      dataset->setActive( i, toBool( line ) );
    }
  }

  const Mesh2dm *m2dm = dynamic_cast<const Mesh2dm *>( mesh );
  size_t meshIdCount = maximumId( mesh ) + 1; // these are native format indexes (IDs). For formats without gaps it equals vertex array index

  for ( size_t id = 0; id < meshIdCount; ++id )
  {
    std::string line;
    std::getline( stream, line );
    std::vector<std::string> tsItems = split( line,  ' ' );

    size_t index;
    if ( m2dm )
      index = m2dm->vertexIndex( id ); //this index may be out of values array
    else
      index = id;

    if ( index >= vertexCount ) continue;

    if ( isVector )
    {
      if ( tsItems.size() >= 2 ) // BASEMENT files with vectors have 3 columns
      {
        dataset->setVectorValue( index, toDouble( tsItems[0] ), toDouble( tsItems[1] ) );
      }
      else
      {
        MDAL::Log::debug( "invalid timestep line" );
      }
    }
    else
    {
      if ( tsItems.size() >= 1 )
        dataset->setScalarValue( index, toDouble( tsItems[0] ) );
      else
      {
        MDAL::Log::debug( "invalid timestep line" );
      }
    }
  }

  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
}

void MDAL::DriverAsciiDat::readElementTimestep(
  const MDAL::Mesh *mesh,
  std::shared_ptr<DatasetGroup> group,
  MDAL::RelativeTimestamp t,
  bool isVector,
  std::ifstream &stream ) const
{
  assert( group );

  // element is either edge of face, mixed meshes are not supported
  size_t elementCount = mesh->edgesCount() + mesh->facesCount();
  std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MDAL::MemoryDataset2D >( group.get() );
  dataset->setTime( t );
  for ( size_t index = 0; index < elementCount; ++index )
  {
    std::string line;
    std::getline( stream, line );
    std::vector<std::string> tsItems = split( line, ' ' );

    if ( isVector )
    {
      if ( tsItems.size() >= 2 ) // BASEMENT files with vectors have 3 columns
      {
        dataset->setVectorValue( index, toDouble( tsItems[0] ), toDouble( tsItems[1] ) );
      }
      else
      {
        MDAL::Log::debug( "invalid timestep line" );
      }
    }
    else
    {
      if ( tsItems.size() >= 1 )
        dataset->setScalarValue( index, toDouble( tsItems[0] ) ) ;
      else
      {
        MDAL::Log::debug( "invalid timestep line" );
      }
    }
  }

  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
}

bool MDAL::DriverAsciiDat::persist( MDAL::DatasetGroup *group )
{
  assert( ( group->dataLocation() == MDAL_DataLocation::DataOnFaces ) ||
          ( group->dataLocation() == MDAL_DataLocation::DataOnVertices ) ||
          ( group->dataLocation() == MDAL_DataLocation::DataOnEdges ) );

  const Mesh *mesh = group->mesh();
  const bool isScalar = group->isScalar();
  std::string uri = group->uri();

  if ( !MDAL::contains( uri, "_els" ) && group->dataLocation() != MDAL_DataLocation::DataOnVertices )
  {
    // Should contain _els in name for edges/faces dataset but it does not
    int pos = MDAL::toInt( uri.size() ) - 4;
    uri.insert( std::max( 0, pos ), "_els" );
    group->replaceUri( uri );
  }

  if ( ( mesh->facesCount() > 0 ) && ( mesh->edgesCount() > 0 ) )
  {
    // not possible to use for mixed 1D and 2D meshes
    return true;
  }

  std::ofstream out = MDAL::openOutputFile( uri, std::ofstream::out );

  // implementation based on information from:
  // https://www.xmswiki.com/wiki/SMS:ASCII_Dataset_Files_*.dat
  if ( !out )
    return true; // Couldn't open the file


  size_t nodeCount = mesh->verticesCount();
  size_t elemCount = mesh->facesCount() + mesh->edgesCount();

  out << "DATASET\n";
  out << "OBJTYPE \"mesh2d\"\n";

  if ( isScalar )
    out << "BEGSCL\n";
  else
    out << "BEGVEC\n";

  out << "ND " << nodeCount << "\n";
  out << "NC " << elemCount << "\n";
  out << "NAME " "\"" << group->name() << "\"" "\n";
  std::string referenceTimeStr = group->referenceTime().toJulianDayString();

  if ( !referenceTimeStr.empty() )
  {
    out << "RT_JULIAN " << referenceTimeStr << "\n";
  }

  out << "TIMEUNITS " << 0 << "\n";

  for ( size_t time_index = 0; time_index < group->datasets.size(); ++ time_index )
  {
    const std::shared_ptr<MDAL::MemoryDataset2D> dataset
      = std::dynamic_pointer_cast<MDAL::MemoryDataset2D>( group->datasets[time_index] );

    bool hasActiveStatus = ( group->dataLocation() == MDAL_DataLocation::DataOnVertices ) && dataset->supportsActiveFlag();
    out << "TS " << hasActiveStatus << " " << std::to_string( dataset->time( RelativeTimestamp::hours ) ) << "\n";

    if ( hasActiveStatus )
    {
      // Fill the active data
      for ( size_t i = 0; i < elemCount; ++i )
      {
        int active = dataset->active( i );
        out << ( active == 1 ? true : false ) << "\n";
      }
    }

    size_t valuesToWrite = ( group->dataLocation() == MDAL_DataLocation::DataOnVertices ) ? nodeCount : elemCount;

    for ( size_t i = 0; i < valuesToWrite; ++i )
    {
      // Read values flags
      if ( isScalar )
        out << dataset->scalarValue( i ) << "\n";
      else
      {
        out << dataset->valueX( i ) << " " << dataset->valueY( i )  << "\n";
      }
    }
  }

  out << "ENDDS";

  return false;
}

std::string MDAL::DriverAsciiDat::writeDatasetOnFileSuffix() const
{
  return "dat";
}
