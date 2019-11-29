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
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_2dm.hpp"

#include <math.h>

#define EXIT_WITH_ERROR(error)       {  if (status) *status = (error); return; }

MDAL::DriverAsciiDat::DriverAsciiDat( ):
  Driver( "ASCII_DAT",
          "DAT",
          "*.dat",
          Capability::ReadDatasets | Capability::WriteDatasetsOnFaces2D | Capability::WriteDatasetsOnVertices2D
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
  std::ifstream in( uri, std::ifstream::in );
  std::string line;
  if ( !std::getline( in, line ) )
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
         MDAL::contains( line, "TS" );
}

bool MDAL::DriverAsciiDat::canReadNewFormat( const std::string &line ) const
{
  return line == "DATASET";
}


void MDAL::DriverAsciiDat::loadOldFormat( std::ifstream &in,
    Mesh *mesh,
    MDAL_Status *status ) const
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
  group->setDataLocation( MDAL_DataLocation::DataOnVertices2D );

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
        EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh )
      }
    }
    else if ( cardType == "SCALAR" || cardType == "VECTOR" )
    {
      // just ignore - we know the type from earlier...
    }
    else if ( cardType == "TS" && items.size() >=  2 )
    {
      double t = toDouble( items[ 1 ] );
      readVertexTimestep( mesh, group, t, isVector, false, in );
    }
    else
    {
      std::stringstream str;
      str << " Unknown card:" << line;
      debug( str.str() );
    }
  }
  while ( std::getline( in, line ) );

  if ( !group || group->datasets.size() == 0 )
  {
    EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat )
  }

  group->setStatistics( MDAL::calculateStatistics( group ) );
  mesh->datasetGroups.push_back( group );
  group.reset();
}

void MDAL::DriverAsciiDat::loadNewFormat(
  std::ifstream &in,
  Mesh *mesh,
  MDAL_Status *status ) const
{
  bool isVector = false;
  std::shared_ptr<DatasetGroup> group; // DAT outputs data
  std::string groupName( MDAL::baseName( mDatFile ) );
  std::string line;
  std::string referenceTime;
  // see if it contains face-centered results - supported by BASEMENT
  bool faceCentered = false;
  if ( contains( groupName, "_els" ) )
    faceCentered = true;

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
        EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh )
      }
    else if ( cardType == "NC" && items.size() >= 2 )
    {
      size_t fileElemCount = toSizeT( items[1] );
      if ( mesh->facesCount() != fileElemCount )
        EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh )
      }
    else if ( cardType == "OBJTYPE" )
    {
      if ( items[1] != "mesh2d" && items[1] != "\"mesh2d\"" )
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat )
      }
    else if ( cardType == "BEGSCL" || cardType == "BEGVEC" )
    {
      if ( group )
      {
        debug( "New dataset while previous one is still active!" );
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat )
      }
      isVector = cardType == "BEGVEC";

      group = std::make_shared< DatasetGroup >(
                name(),
                mesh,
                mDatFile,
                groupName
              );
      group->setIsScalar( !isVector );
      group->setDataLocation( faceCentered ? MDAL_DataLocation::DataOnFaces2D : MDAL_DataLocation::DataOnVertices2D );
      group->setReferenceTime( referenceTime );
    }
    else if ( cardType == "ENDDS" )
    {
      if ( !group )
      {
        debug( "ENDDS card for no active dataset!" );
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat )
      }
      group->setStatistics( MDAL::calculateStatistics( group ) );
      mesh->datasetGroups.push_back( group );
      group.reset();
    }
    else if ( cardType == "NAME" && items.size() >= 2 )
    {
      if ( !group )
      {
        debug( "NAME card for no active dataset!" );
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat )
      }

      size_t quoteIdx1 = line.find( '\"' );
      size_t quoteIdx2 = line.find( '\"', quoteIdx1 + 1 );
      if ( quoteIdx1 != std::string::npos && quoteIdx2 != std::string::npos )
        group->setName( line.substr( quoteIdx1 + 1, quoteIdx2 - quoteIdx1 - 1 ) );
    }
    else if ( cardType == "RT_JULIAN" && items.size() >= 2 )
    {
      referenceTime = "JULIAN " + items[1];
    }
    else if ( cardType == "TIMEUNITS" && items.size() >= 2 )
    {
      if ( !group )
      {
        debug( "TIMEUNITS card for no active dataset!" );
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat )
      }

      group->setMetadata( "TIMEUNITS", items[1] );
    }
    else if ( cardType == "TS" && items.size() >= 3 )
    {
      double t = toDouble( items[2] );
      t = convertTimeDataToHours( t, group->getMetadata( "TIMEUNITS" ) );

      if ( faceCentered )
      {
        readFaceTimestep( mesh, group, t, isVector, in );
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
      debug( str.str() );
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
void MDAL::DriverAsciiDat::load( const std::string &datFile, MDAL::Mesh *mesh, MDAL_Status *status )
{
  mDatFile = datFile;
  if ( status ) *status = MDAL_Status::None;

  if ( !MDAL::fileExists( mDatFile ) )
  {
    if ( status ) *status = MDAL_Status::Err_FileNotFound;
    return;
  }

  size_t mID = maximumId( mesh );
  if ( mID == std::numeric_limits<size_t>::max() )
  {
    // This happens when mesh is 2DM and vertices are numbered from 0
    if ( status ) *status = MDAL_Status::Err_IncompatibleMesh;
    return;
  }

  std::ifstream in( mDatFile, std::ifstream::in );
  std::string line;
  if ( !std::getline( in, line ) )
  {
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return;
  }
  line = trim( line );
  if ( canReadNewFormat( line ) )
  {
    // we do not need to parse first line again
    loadNewFormat( in, mesh, status );
  }
  else
  {
    // we need to parse first line again to see
    // scalar/vector flag or timestep flag
    in.clear();
    in.seekg( 0 );
    loadOldFormat( in, mesh, status );
  }
}

void MDAL::DriverAsciiDat::readVertexTimestep(
  const MDAL::Mesh *mesh,
  std::shared_ptr<DatasetGroup> group,
  double t,
  bool isVector,
  bool hasStatus,
  std::ifstream &stream ) const
{
  assert( group );
  size_t faceCount = mesh->facesCount();
  size_t vertexCount = mesh->verticesCount();

  std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MDAL::MemoryDataset2D >( group.get() );
  dataset->setTime( t );

  int *active = dataset->active();
  // only for new format
  for ( size_t i = 0; i < faceCount; ++i )
  {
    if ( hasStatus )
    {
      std::string line;
      std::getline( stream, line );
      active[i] = toBool( line );
    }
  }

  const Mesh2dm *m2dm = dynamic_cast<const Mesh2dm *>( mesh );
  double *values = dataset->values();
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
        values[2 * index] = toDouble( tsItems[0] );
        values[2 * index + 1] = toDouble( tsItems[1] );
      }
      else
      {
        debug( "invalid timestep line" );
      }
    }
    else
    {
      if ( tsItems.size() >= 1 )
        values[index] = toDouble( tsItems[0] );
      else
      {
        debug( "invalid timestep line" );
      }
    }
  }

  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
}

void MDAL::DriverAsciiDat::readFaceTimestep(
  const MDAL::Mesh *mesh,
  std::shared_ptr<DatasetGroup> group,
  double t,
  bool isVector,
  std::ifstream &stream ) const
{
  assert( group );

  size_t faceCount = mesh->facesCount();

  std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MDAL::MemoryDataset2D >( group.get() );
  dataset->setTime( t );
  double *values = dataset->values();
  // TODO: hasStatus
  for ( size_t index = 0; index < faceCount; ++index )
  {
    std::string line;
    std::getline( stream, line );
    std::vector<std::string> tsItems = split( line, ' ' );

    if ( isVector )
    {
      if ( tsItems.size() >= 2 ) // BASEMENT files with vectors have 3 columns
      {
        values[2 * index] = toDouble( tsItems[0] );
        values[2 * index + 1] = toDouble( tsItems[1] );
      }
      else
      {
        debug( "invalid timestep line" );
      }
    }
    else
    {
      if ( tsItems.size() >= 1 )
        values[index] = toDouble( tsItems[0] );
      else
      {
        debug( "invalid timestep line" );
      }
    }
  }

  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
}

bool MDAL::DriverAsciiDat::persist( MDAL::DatasetGroup *group )
{
  assert( ( group->dataLocation() == MDAL_DataLocation::DataOnFaces2D ) ||
          ( group->dataLocation() == MDAL_DataLocation::DataOnVertices2D ) );

  const bool isScalar = group->isScalar();
  const bool isOnVertices = group->dataLocation() == MDAL_DataLocation::DataOnVertices2D;
  std::string uri = group->uri();

  if ( !MDAL::contains( uri, "_els" ) && isOnVertices == false )
  {
    // Should contain _els in name but it does not
    uri.insert( uri.size() - 4, "_els" );
  }

  std::ofstream out( uri, std::ofstream::out );

  // implementation based on information from:
  // https://www.xmswiki.com/wiki/SMS:ASCII_Dataset_Files_*.dat
  if ( !out )
    return true; // Couldn't open the file

  const Mesh *mesh = group->mesh();
  size_t nodeCount = mesh->verticesCount();
  size_t elemCount = mesh->facesCount();

  out << "DATASET\n";
  out << "OBJTYPE \"mesh2d\"\n";

  if ( isScalar )
    out << "BEGSCL\n";
  else
    out << "BEGVEC\n";

  out << "ND " << nodeCount << "\n";
  out << "NC " << elemCount << "\n";
  out << "NAME " "\"" << group->name() << "\"" "\n";
  std::string referenceTimeStr = group->referenceTime();

  if ( !referenceTimeStr.empty() )
  {
    // Cutting of the JULIAN prefix
    std::vector<std::string> referenceTimeStrWords = split( referenceTimeStr,  ' ' );

    if ( referenceTimeStrWords.size() > 1 )
      out << "RT_JULIAN " << referenceTimeStrWords[1] << "\n";
    else
      out << "RT_JULIAN " << referenceTimeStr << "\n";
  }

  out << "TIMEUNITS " << 0 << "\n";

  for ( size_t time_index = 0; time_index < group->datasets.size(); ++ time_index )
  {
    const std::shared_ptr<MDAL::MemoryDataset2D> dataset
      = std::dynamic_pointer_cast<MDAL::MemoryDataset2D>( group->datasets[time_index] );

    bool hasActiveStatus = isOnVertices && dataset->active();
    out << "TS " << hasActiveStatus << " " << std::to_string( dataset->time() ) << "\n";

    if ( hasActiveStatus )
    {
      // Fill the active data
      for ( size_t i = 0; i < elemCount; ++i )
      {
        int active = dataset->active()[i];
        out << ( active == 1 ? true : false ) << "\n";
      }
    }

    size_t valuesToWrite = isOnVertices ? nodeCount : elemCount;

    for ( size_t i = 0; i < valuesToWrite; ++i )
    {
      // Read values flags
      if ( isScalar )
        out << dataset->values()[i] << "\n";
      else
      {
        out << dataset->values()[2 * i] << " " << dataset->values()[2 * i + 1 ]  << "\n";
      }
    }
  }

  out << "ENDDS";

  return false;
}

double MDAL::DriverAsciiDat::convertTimeDataToHours( double time, const std::string &originalTimeDataUnit ) const
{
  if ( originalTimeDataUnit == "se" || originalTimeDataUnit == "2" || originalTimeDataUnit == "Seconds"
       || originalTimeDataUnit.empty() )
  {
    time /= 3600.0;
  }
  else if ( originalTimeDataUnit == "mi" || originalTimeDataUnit == "1" || originalTimeDataUnit == "Minutes" )
  {
    time /= 60.0;
  }
  else if ( originalTimeDataUnit == "days" )
  {
    time *= 24;
  }
  return time;
}
