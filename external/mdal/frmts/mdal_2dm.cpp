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

#include "mdal_2dm.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"

#define DRIVER_NAME "2DM"

MDAL::Mesh2dm::Mesh2dm( size_t verticesCount,
                        size_t facesCount,
                        size_t faceVerticesMaximumCount,
                        MDAL::BBox extent,
                        const std::string &uri,
                        const std::map<size_t, size_t> vertexIDtoIndex )
  : MemoryMesh( DRIVER_NAME,
                verticesCount,
                facesCount,
                faceVerticesMaximumCount,
                extent,
                uri )
  , mVertexIDtoIndex( vertexIDtoIndex )
{
}

MDAL::Mesh2dm::~Mesh2dm() = default;

bool _parse_vertex_id_gaps( std::map<size_t, size_t> &vertexIDtoIndex, size_t vertexIndex, size_t vertexID, MDAL_Status *status )
{
  if ( vertexIndex == vertexID )
    return false;

  std::map<size_t, size_t>::iterator search = vertexIDtoIndex.find( vertexID );
  if ( search != vertexIDtoIndex.end() )
  {
    if ( status ) *status = MDAL_Status::Warn_ElementNotUnique;
    return true;
  }

  vertexIDtoIndex[vertexID] = vertexIndex;
  return false;
}

size_t MDAL::Mesh2dm::vertexIndex( size_t vertexID ) const
{
  auto ni2i = mVertexIDtoIndex.find( vertexID );
  if ( ni2i != mVertexIDtoIndex.end() )
  {
    return  ni2i->second; // convert from ID to index
  }
  return vertexID;
}


MDAL::Driver2dm::Driver2dm():
  Driver( DRIVER_NAME,
          "2DM Mesh File",
          "*.2dm",
          Capability::ReadMesh
        )
{
}

MDAL::Driver2dm *MDAL::Driver2dm::create()
{
  return new Driver2dm();
}

MDAL::Driver2dm::~Driver2dm() = default;

bool MDAL::Driver2dm::canRead( const std::string &uri )
{
  std::ifstream in( uri, std::ifstream::in );
  std::string line;
  if ( !std::getline( in, line ) || !startsWith( line, "MESH2D" ) )
  {
    return false;
  }
  return true;
}

std::unique_ptr<MDAL::Mesh> MDAL::Driver2dm::load( const std::string &meshFile, MDAL_Status *status )
{
  mMeshFile = meshFile;

  if ( status ) *status = MDAL_Status::None;

  std::ifstream in( mMeshFile, std::ifstream::in );
  std::string line;
  if ( !std::getline( in, line ) || !startsWith( line, "MESH2D" ) )
  {
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return nullptr;
  }

  size_t faceCount = 0;
  size_t vertexCount = 0;

  // Find out how many nodes and elements are contained in the .2dm mesh file
  while ( std::getline( in, line ) )
  {
    if ( startsWith( line, "E4Q" ) ||
         startsWith( line, "E3T" ) )
    {
      faceCount++;
    }
    else if ( startsWith( line, "ND" ) )
    {
      vertexCount++;
    }
    else if ( startsWith( line, "E2L" ) ||
              startsWith( line, "E3L" ) ||
              startsWith( line, "E6T" ) ||
              startsWith( line, "E8Q" ) ||
              startsWith( line, "E9Q" ) )
    {
      if ( status ) *status = MDAL_Status::Warn_UnsupportedElement;
      faceCount += 1; // We still count them as elements
    }
  }

  // Allocate memory
  std::vector<Vertex> vertices( vertexCount );
  std::vector<Face> faces( faceCount );

  in.clear();
  in.seekg( 0, std::ios::beg );

  std::vector<std::string> chunks;

  size_t faceIndex = 0;
  size_t vertexIndex = 0;
  std::map<size_t, size_t> vertexIDtoIndex;

  while ( std::getline( in, line ) )
  {
    if ( startsWith( line, "E4Q" ) )
    {
      chunks = split( line,  ' ' );
      assert( faceIndex < faceCount );

      Face &face = faces[faceIndex];
      face.resize( 4 );
      // Right now we just store node IDs here - we will convert them to node indices afterwards
      for ( size_t i = 0; i < 4; ++i )
        face[i] = toSizeT( chunks[i + 2] ) - 1; // 2dm is numbered from 1

      faceIndex++;
    }
    else if ( startsWith( line, "E3T" ) )
    {
      chunks = split( line,  ' ' );
      assert( faceIndex < faceCount );

      Face &face = faces[faceIndex];
      face.resize( 3 );
      // Right now we just store node IDs here - we will convert them to node indices afterwards
      for ( size_t i = 0; i < 3; ++i )
      {
        face[i] = toSizeT( chunks[i + 2] ) - 1; // 2dm is numbered from 1
      }

      faceIndex++;
    }
    else if ( startsWith( line, "E2L" ) ||
              startsWith( line, "E3L" ) ||
              startsWith( line, "E6T" ) ||
              startsWith( line, "E8Q" ) ||
              startsWith( line, "E9Q" ) )
    {
      // We do not yet support these elements
      chunks = split( line,  ' ' );
      assert( faceIndex < faceCount );

      //size_t elemID = toSizeT( chunks[1] );
      assert( false ); //TODO mark element as unusable

      faceIndex++;
    }
    else if ( startsWith( line, "ND" ) )
    {
      chunks = split( line,  ' ' );
      size_t nodeID = toSizeT( chunks[1] ) - 1; // 2dm is numbered from 1
      _parse_vertex_id_gaps( vertexIDtoIndex, vertexIndex, nodeID, status );
      assert( vertexIndex < vertexCount );
      Vertex &vertex = vertices[vertexIndex];
      vertex.x = toDouble( chunks[2] );
      vertex.y = toDouble( chunks[3] );
      vertex.z = toDouble( chunks[4] );
      vertexIndex++;
    }
  }

  for ( std::vector<Face>::iterator it = faces.begin(); it != faces.end(); ++it )
  {
    Face &face = *it;
    for ( Face::size_type nd = 0; nd < face.size(); ++nd )
    {
      size_t nodeID = face[nd];

      std::map<size_t, size_t>::iterator ni2i = vertexIDtoIndex.find( nodeID );
      if ( ni2i != vertexIDtoIndex.end() )
      {
        face[nd] = ni2i->second; // convert from ID to index
      }
      else if ( vertices.size() < nodeID )
      {
        if ( status ) *status = MDAL_Status::Warn_ElementWithInvalidNode;
      }
    }
    //TODO check validity of the face
    //check that we have distinct nodes
  }

  std::unique_ptr< Mesh2dm > mesh(
    new Mesh2dm(
      vertices.size(),
      faces.size(),
      4, //maximum quads
      computeExtent( vertices ),
      mMeshFile,
      vertexIDtoIndex
    )
  );
  mesh->faces = faces;
  mesh->vertices = vertices;
  MDAL::addBedElevationDatasetGroup( mesh.get(), vertices );
  return std::unique_ptr<Mesh>( mesh.release() );
}
