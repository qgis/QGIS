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

MDAL::Loader2dm::Loader2dm( const std::string &meshFile ):
  mMeshFile( meshFile )
{
}

std::unique_ptr<MDAL::Mesh> MDAL::Loader2dm::load( MDAL_Status *status )
{
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
  std::map<size_t, size_t> faceIDtoIndex;
  std::map<size_t, size_t> vertexIDtoIndex;

  while ( std::getline( in, line ) )
  {
    if ( startsWith( line, "E4Q" ) )
    {
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      assert( faceIndex < faceCount );

      size_t elemID = toSizeT( chunks[1] );

      std::map<size_t, size_t>::iterator search = faceIDtoIndex.find( elemID );
      if ( search != faceIDtoIndex.end() )
      {
        if ( status ) *status = MDAL_Status::Warn_ElementNotUnique;
        continue;
      }
      faceIDtoIndex[elemID] = faceIndex;
      Face &face = faces[faceIndex];
      face.resize( 4 );
      // Right now we just store node IDs here - we will convert them to node indices afterwards
      for ( size_t i = 0; i < 4; ++i )
        face[i] = toSizeT( chunks[i + 2] );

      faceIndex++;
    }
    else if ( startsWith( line, "E3T" ) )
    {
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      assert( faceIndex < faceCount );

      size_t elemID = toSizeT( chunks[1] );

      std::map<size_t, size_t>::iterator search = faceIDtoIndex.find( elemID );
      if ( search != faceIDtoIndex.end() )
      {
        if ( status ) *status = MDAL_Status::Warn_ElementNotUnique;
        continue;
      }
      faceIDtoIndex[elemID] = faceIndex;
      Face &face = faces[faceIndex];
      face.resize( 3 );
      // Right now we just store node IDs here - we will convert them to node indices afterwards
      for ( size_t i = 0; i < 3; ++i )
      {
        face[i] = toSizeT( chunks[i + 2] );
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
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      assert( faceIndex < faceCount );

      size_t elemID = toSizeT( chunks[1] );

      std::map<size_t, size_t>::iterator search = faceIDtoIndex.find( elemID );
      if ( search != faceIDtoIndex.end() )
      {
        if ( status ) *status = MDAL_Status::Warn_ElementNotUnique;
        continue;
      }
      faceIDtoIndex[elemID] = faceIndex;
      assert( false ); //TODO mark element as unusable

      faceIndex++;
    }
    else if ( startsWith( line, "ND" ) )
    {
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      size_t nodeID = toSizeT( chunks[1] );

      std::map<size_t, size_t>::iterator search = vertexIDtoIndex.find( nodeID );
      if ( search != vertexIDtoIndex.end() )
      {
        if ( status ) *status = MDAL_Status::Warn_NodeNotUnique;
        continue;
      }
      vertexIDtoIndex[nodeID] = vertexIndex;
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
      else
      {
        assert( false ); //TODO mark element as unusable

        if ( status ) *status = MDAL_Status::Warn_ElementWithInvalidNode;
      }
    }

    //TODO check validity of the face
    //check that we have distinct nodes
  }

  std::unique_ptr< Mesh > mesh( new Mesh );
  mesh->uri = mMeshFile;
  mesh->faces = faces;
  mesh->vertices = vertices;
  mesh->faceIDtoIndex = faceIDtoIndex;
  mesh->vertexIDtoIndex = vertexIDtoIndex;
  mesh->addBedElevationDataset();

  return mesh;
}
