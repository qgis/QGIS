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
          Capability::ReadDatasets
        )
{
}

MDAL::DriverAsciiDat *MDAL::DriverAsciiDat::create()
{
  return new DriverAsciiDat();
}

MDAL::DriverAsciiDat::~DriverAsciiDat( ) = default;

bool MDAL::DriverAsciiDat::canRead( const std::string &uri )
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
  group->setIsOnVertices( true );

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
      if ( mesh->verticesCount() != fileNodeCount )
        EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh );
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
    EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

  group->setStatistics( MDAL::calculateStatistics( group ) );
  mesh->datasetGroups.push_back( group );
  group.reset();
}

void MDAL::DriverAsciiDat::loadNewFormat( std::ifstream &in,
    Mesh *mesh,
    MDAL_Status *status ) const
{
  bool isVector = false;
  std::shared_ptr<DatasetGroup> group; // DAT outputs data
  std::string groupName( MDAL::baseName( mDatFile ) );
  std::string line;

  // see if it contains face-centered results - supported by BASEMENT
  bool faceCentered = false;
  if ( contains( groupName, "_els_" ) )
    faceCentered = true;

  if ( group )
    group->setIsOnVertices( !faceCentered );

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
      if ( mesh->verticesCount() != fileNodeCount )
        EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh );
    }
    else if ( cardType == "NC" && items.size() >= 2 )
    {
      size_t fileElemCount = toSizeT( items[1] );
      if ( mesh->facesCount() != fileElemCount )
        EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh );
    }
    else if ( cardType == "OBJTYPE" )
    {
      if ( items[1] != "mesh2d" && items[1] != "\"mesh2d\"" )
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
    }
    else if ( cardType == "BEGSCL" || cardType == "BEGVEC" )
    {
      if ( group )
      {
        debug( "New dataset while previous one is still active!" );
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
      }
      isVector = cardType == "BEGVEC";

      group = std::make_shared< DatasetGroup >(
                name(),
                mesh,
                mDatFile,
                groupName
              );
      group->setIsScalar( !isVector );
      group->setIsOnVertices( !faceCentered );
    }
    else if ( cardType == "ENDDS" )
    {
      if ( !group )
      {
        debug( "ENDDS card for no active dataset!" );
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
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
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
      }

      size_t quoteIdx1 = line.find( '\"' );
      size_t quoteIdx2 = line.find( '\"', quoteIdx1 + 1 );
      if ( quoteIdx1 != std::string::npos && quoteIdx2 != std::string::npos )
        group->setName( line.substr( quoteIdx1 + 1, quoteIdx2 - quoteIdx1 - 1 ) );
    }
    else if ( cardType == "TS" && items.size() >= 3 )
    {
      double t = toDouble( items[2] );

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

  std::shared_ptr<MDAL::MemoryDataset> dataset = std::make_shared< MDAL::MemoryDataset >( group.get() );
  dataset->setTime( t / 3600. ); // TODO read TIMEUNITS

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
  for ( size_t i = 0; i < mesh->verticesCount(); ++i )
  {
    std::string line;
    std::getline( stream, line );
    std::vector<std::string> tsItems = split( line,  ' ' );

    size_t index;
    if ( m2dm )
      index = m2dm->vertexIndex( i );
    else
      index = i;

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

  std::shared_ptr<MDAL::MemoryDataset> dataset = std::make_shared< MDAL::MemoryDataset >( group.get() );
  dataset->setTime( t / 3600. );
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
