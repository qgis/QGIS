/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Lutra Consulting Ltd.
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
#include <limits>
#include <algorithm>

#include "mdal_xms_tin.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_logger.hpp"

#define DRIVER_NAME "XMS_TIN"

MDAL::DriverXmsTin::DriverXmsTin():
  Driver( DRIVER_NAME,
          "XMS Tin Mesh File",
          "*.tin",
          Capability::ReadMesh
        )
{
}

MDAL::DriverXmsTin *MDAL::DriverXmsTin::create()
{
  return new DriverXmsTin();
}

int MDAL::DriverXmsTin::faceVerticesMaximumCount() const
{
  return MAX_VERTICES_PER_FACE_TIN;
}

MDAL::DriverXmsTin::~DriverXmsTin() = default;

bool MDAL::DriverXmsTin::canReadMesh( const std::string &uri )
{
  std::ifstream in = MDAL::openInputFile( uri, std::ifstream::in );
  std::string line;
  if ( !MDAL::getHeaderLine( in, line ) || !startsWith( line, "TIN" ) )
  {
    return false;
  }
  return true;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverXmsTin::load( const std::string &meshFile, const std::string & )
{
  MDAL::Log::resetLastStatus();

  std::ifstream in = MDAL::openInputFile( meshFile, std::ifstream::in );
  std::string line;
  // skip first line with "TIN" already checked in the canReadMesh
  std::getline( in, line );

  // Read vertices
  if ( !std::getline( in, line ) || !startsWith( line, "BEGT" ) )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), meshFile + " second line does not start with BEGT keyword" );
    return nullptr;
  }
  if ( !std::getline( in, line ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " does not contain 3rd line" );
    return nullptr;
  }
  std::vector<std::string> chunks = split( line,  ' ' );
  if ( ( chunks.size() != 2 ) || ( chunks[0] != "VERT" ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " 4th line does not contain VERT keyword with number of vertices" );
    return nullptr;
  }
  size_t vertexCount = MDAL::toSizeT( chunks[1] );
  Vertices vertices( vertexCount );
  for ( size_t i = 0; i < vertexCount; ++i )
  {
    if ( !std::getline( in, line ) )
    {
      MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " does not contain enough vertex definitions" );
      return nullptr;
    }
    chunks = split( line,  ' ' );
    if ( chunks.size() != 4 )
    {
      MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " does not contain valid vertex definition" );
      return nullptr;
    }

    Vertex &vertex = vertices[i];
    vertex.x = MDAL::toDouble( chunks[0] );
    vertex.y = MDAL::toDouble( chunks[1] );
    vertex.z = MDAL::toDouble( chunks[2] );
  }

  // Read triangles
  if ( !std::getline( in, line ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " does not contain valid triangle definition" );
    return nullptr;
  }
  chunks = split( line,  ' ' );
  if ( ( chunks.size() != 2 ) || ( chunks[0] != "TRI" ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " does not contain TRI keyword" );
    return nullptr;
  }
  size_t faceCount = MDAL::toSizeT( chunks[1] );
  Faces faces( faceCount );
  for ( size_t i = 0; i < faceCount; ++i )
  {
    if ( !std::getline( in, line ) )
    {
      MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " does not contain enough triangle definitions" );
      return nullptr;
    }
    chunks = split( line,  ' ' );
    if ( chunks.size() != 3 )
    {
      // should have 3 indexes
      MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " does not contain valid triangle definition" );
      return nullptr;
    }

    Face &face = faces[i];
    face.resize( MAX_VERTICES_PER_FACE_TIN );
    face[0] = MDAL::toSizeT( chunks[0] ) - 1;
    face[1] = MDAL::toSizeT( chunks[1] ) - 1;
    face[2] = MDAL::toSizeT( chunks[2] ) - 1;
  }

  // Final keyword
  if ( !std::getline( in, line ) || !startsWith( line, "ENDT" ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " does not end with ENDT keyword" );
    return nullptr;
  }

  std::unique_ptr< MemoryMesh > mesh(
    new MemoryMesh(
      DRIVER_NAME,
      MAX_VERTICES_PER_FACE_TIN,
      meshFile
    )
  );
  mesh->setFaces( std::move( faces ) );
  mesh->setVertices( std::move( vertices ) );

  // Add Bed Elevation
  MDAL::addBedElevationDatasetGroup( mesh.get(), mesh->vertices() );

  return std::unique_ptr<Mesh>( mesh.release() );
}
