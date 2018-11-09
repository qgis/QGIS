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

#include <math.h>

#define EXIT_WITH_ERROR(error)       {  if (status) *status = (error); return; }

MDAL::LoaderAsciiDat::LoaderAsciiDat( const std::string &datFile ):
  mDatFile( datFile )
{
}

/**
 * The DAT format contains "datasets" and each dataset has N-outputs. One output
 * represents data for all vertices/faces for one timestep
 *
 * In MDAL we convert one output to one MDAL dataset;
 *
 */
void MDAL::LoaderAsciiDat::load( MDAL::Mesh *mesh, MDAL_Status *status )
{
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

  // http://www.xmswiki.com/xms/SMS:ASCII_Dataset_Files_*.dat
  // Apart from the format specified above, there is an older supported format used in BASEMENT (and SMS?)
  // which is simpler (has only one dataset in one file, no status flags etc)
  bool oldFormat;
  bool isVector = false;

  std::shared_ptr<DatasetGroup> group; // DAT outputs data
  std::string name( MDAL::baseName( mDatFile ) );

  if ( line == "DATASET" )
    oldFormat = false;
  else if ( line == "SCALAR" || line == "VECTOR" )
  {
    oldFormat = true;
    isVector = ( line == "VECTOR" );

    group = std::make_shared< DatasetGroup >();
    group->uri = mDatFile;
    group->setName( name );
    group->isScalar = !isVector;
  }
  else
    EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

  // see if it contains face-centered results - supported by BASEMENT
  bool faceCentered = false;
  if ( !oldFormat && contains( name, "_els_" ) )
    faceCentered = true;

  if ( group )
    group->isOnVertices = !faceCentered;

  while ( std::getline( in, line ) )
  {
    std::vector<std::string> items = split( line,  " ", SplitBehaviour::SkipEmptyParts );
    if ( items.size() < 1 )
      continue; // empty line?? let's skip it

    std::string cardType = items[0];
    if ( cardType == "ND" && items.size() >= 2 )
    {
      size_t fileNodeCount = toSizeT( items[1] );
      if ( mesh->vertexIDtoIndex.size() != fileNodeCount )
        EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh );
    }
    else if ( !oldFormat && cardType == "NC" && items.size() >= 2 )
    {
      size_t fileElemCount = toSizeT( items[1] );
      if ( mesh->faceIDtoIndex.size() != fileElemCount )
        EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh );
    }
    else if ( !oldFormat && cardType == "OBJTYPE" )
    {
      if ( items[1] != "mesh2d" && items[1] != "\"mesh2d\"" )
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
    }
    else if ( !oldFormat && ( cardType == "BEGSCL" || cardType == "BEGVEC" ) )
    {
      if ( group )
      {
        debug( "New dataset while previous one is still active!" );
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
      }
      isVector = cardType == "BEGVEC";

      group = std::make_shared< DatasetGroup >();
      group->uri = mDatFile;
      group->setName( name );
      group->isScalar = !isVector;
      group->isOnVertices = !faceCentered;
    }
    else if ( !oldFormat && cardType == "ENDDS" )
    {
      if ( !group )
      {
        debug( "ENDDS card for no active dataset!" );
        EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
      }
      mesh->datasetGroups.push_back( group );
      group.reset();
    }
    else if ( !oldFormat && cardType == "NAME" && items.size() >= 2 )
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
    else if ( oldFormat && ( cardType == "SCALAR" || cardType == "VECTOR" ) )
    {
      // just ignore - we know the type from earlier...
    }
    else if ( cardType == "TS" && items.size() >= ( oldFormat ? 2 : 3 ) )
    {
      double t = toDouble( items[oldFormat ? 1 : 2] );

      if ( faceCentered )
      {
        readFaceTimestep( mesh, group, t, isVector, in );
      }
      else
      {
        bool hasStatus = ( oldFormat ? false : toBool( items[1] ) );
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

  if ( oldFormat )
  {
    if ( !group || group->datasets.size() == 0 )
      EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

    mesh->datasetGroups.push_back( group );
    group.reset();
  }
}

void MDAL::LoaderAsciiDat::readVertexTimestep(
  const MDAL::Mesh *mesh,
  std::shared_ptr<DatasetGroup> group,
  double t,
  bool isVector,
  bool hasStatus,
  std::ifstream &stream )
{
  assert( group );

  size_t vertexCount = mesh->vertices.size();
  size_t faceCount = mesh->faces.size();

  std::shared_ptr<MDAL::Dataset> dataset = std::make_shared< MDAL::Dataset >();
  dataset->time = t / 3600.; // TODO read TIMEUNITS
  dataset->values.resize( vertexCount );
  dataset->active.resize( faceCount );
  dataset->parent = group.get();

  // only for new format
  for ( size_t i = 0; i < faceCount; ++i )
  {
    if ( hasStatus )
    {
      std::string line;
      std::getline( stream, line );
      dataset->active[i] = toBool( line );
    }
    else
      dataset->active[i] = true;
  }

  for ( size_t i = 0; i < mesh->vertexIDtoIndex.size(); ++i )
  {
    std::string line;
    std::getline( stream, line );
    std::vector<std::string> tsItems = split( line,  " ", SplitBehaviour::SkipEmptyParts );

    auto idx = mesh->vertexIDtoIndex.find( i + 1 ); // ID in 2dm are numbered from 1
    if ( idx == mesh->vertexIDtoIndex.end() )
      continue; // node ID that does not exist in the mesh

    size_t index = idx->second;

    if ( isVector )
    {
      if ( tsItems.size() >= 2 ) // BASEMENT files with vectors have 3 columns
      {
        dataset->values[index].x = toDouble( tsItems[0] );
        dataset->values[index].y = toDouble( tsItems[1] );
      }
      else
      {
        debug( "invalid timestep line" );
        dataset->values[index].noData = true;
      }
    }
    else
    {
      if ( tsItems.size() >= 1 )
        dataset->values[index].x = toDouble( tsItems[0] );
      else
      {
        debug( "invalid timestep line" );
        dataset->values[index].noData = true;
      }
    }
  }

  group->datasets.push_back( dataset );
}

void MDAL::LoaderAsciiDat::readFaceTimestep(
  const MDAL::Mesh *mesh,
  std::shared_ptr<DatasetGroup> group,
  double t,
  bool isVector,
  std::ifstream &stream )
{
  assert( group );

  size_t faceCount = mesh->faces.size();

  std::shared_ptr<MDAL::Dataset> dataset = std::make_shared< MDAL::Dataset >();
  dataset->time = t / 3600.;
  dataset->values.resize( faceCount );
  dataset->parent = group.get();

  // TODO: hasStatus
  for ( size_t index = 0; index < faceCount; ++index )
  {
    std::string line;
    std::getline( stream, line );
    std::vector<std::string> tsItems = split( line,  " ", SplitBehaviour::SkipEmptyParts );

    if ( isVector )
    {
      if ( tsItems.size() >= 2 ) // BASEMENT files with vectors have 3 columns
      {
        dataset->values[index].x = toDouble( tsItems[0] );
        dataset->values[index].y = toDouble( tsItems[1] );
      }
      else
      {
        debug( "invalid timestep line" );
        dataset->values[index].noData = true;
      }
    }
    else
    {
      if ( tsItems.size() >= 1 )
        dataset->values[index].x = toDouble( tsItems[0] );
      else
      {
        debug( "invalid timestep line" );
        dataset->values[index].noData = true;
      }
    }
  }

  group->datasets.push_back( dataset );
}
