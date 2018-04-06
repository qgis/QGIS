/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
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

MDAL::Mesh *MDAL::Loader2dm::load( Status *status )
{
  if ( status ) *status = Status::None;

  if ( !MDAL::fileExists( mMeshFile ) )
  {
    if ( status ) *status = Status::Err_FileNotFound;
    return 0;
  }

  std::ifstream in( mMeshFile, std::ifstream::in );
  std::string line;
  if ( !std::getline( in, line ) || !startsWith( line, "MESH2D" ) )
  {
    if ( status ) *status = Status::Err_UnknownFormat;
    return 0;
  }

  size_t elemCount = 0;
  size_t nodeCount = 0;

  // Find out how many nodes and elements are contained in the .2dm mesh file
  while ( std::getline( in, line ) )
  {
    if ( startsWith( line, "E4Q" ) ||
         startsWith( line, "E3T" ) )
    {
      elemCount++;
    }
    else if ( startsWith( line, "ND" ) )
    {
      nodeCount++;
    }
    else if ( startsWith( line, "E2L" ) ||
              startsWith( line, "E3L" ) ||
              startsWith( line, "E6T" ) ||
              startsWith( line, "E8Q" ) ||
              startsWith( line, "E9Q" ) )
    {
      if ( status ) *status = Status::Warn_UnsupportedElement;
      elemCount += 1; // We still count them as elements
    }
  }

  // Allocate memory
  std::vector<Vertex> vertices( nodeCount );
  std::vector<Face> faces( elemCount );

  in.clear();
  in.seekg( 0, std::ios::beg );

  std::vector<std::string> chunks;

  size_t elemIndex = 0;
  size_t nodeIndex = 0;
  std::map<size_t, size_t> elemIDtoIndex;
  std::map<size_t, size_t> nodeIDtoIndex;

  while ( std::getline( in, line ) )
  {
    if ( startsWith( line, "E4Q" ) )
    {
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      assert( elemIndex < elemCount );

      size_t elemID = toSizeT( chunks[1] );

      std::map<size_t, size_t>::iterator search = elemIDtoIndex.find( elemID );
      if ( search != elemIDtoIndex.end() )
      {
        if ( status ) *status = Status::Warn_ElementNotUnique;
        continue;
      }
      elemIDtoIndex[elemID] = elemIndex;
      Face &face = faces[elemIndex];
      face.resize( 4 );
      // Right now we just store node IDs here - we will convert them to node indices afterwards
      for ( size_t i = 0; i < 4; ++i )
        face[i] = toSizeT( chunks[i + 2] );

      elemIndex++;
    }
    else if ( startsWith( line, "E3T" ) )
    {
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      assert( elemIndex < elemCount );

      size_t elemID = toSizeT( chunks[1] );

      std::map<size_t, size_t>::iterator search = elemIDtoIndex.find( elemID );
      if ( search != elemIDtoIndex.end() )
      {
        if ( status ) *status = Status::Warn_ElementNotUnique;
        continue;
      }
      elemIDtoIndex[elemID] = elemIndex;
      Face &face = faces[elemIndex];
      face.resize( 3 );
      // Right now we just store node IDs here - we will convert them to node indices afterwards
      for ( size_t i = 0; i < 3; ++i )
      {
        face[i] = toSizeT( chunks[i + 2] );
      }

      elemIndex++;
    }
    else if ( startsWith( line, "E2L" ) ||
              startsWith( line, "E3L" ) ||
              startsWith( line, "E6T" ) ||
              startsWith( line, "E8Q" ) ||
              startsWith( line, "E9Q" ) )
    {
      // We do not yet support these elements
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      assert( elemIndex < elemCount );

      size_t elemID = toSizeT( chunks[1] );

      std::map<size_t, size_t>::iterator search = elemIDtoIndex.find( elemID );
      if ( search != elemIDtoIndex.end() )
      {
        if ( status ) *status = Status::Warn_ElementNotUnique;
        continue;
      }
      elemIDtoIndex[elemID] = elemIndex;
      assert( false ); //TODO mark element as unusable

      elemIndex++;
    }
    else if ( startsWith( line, "ND" ) )
    {
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      size_t nodeID = toSizeT( chunks[1] );

      std::map<size_t, size_t>::iterator search = nodeIDtoIndex.find( nodeID );
      if ( search != nodeIDtoIndex.end() )
      {
        if ( status ) *status = Status::Warn_NodeNotUnique;
        continue;
      }
      nodeIDtoIndex[nodeID] = nodeIndex;
      assert( nodeIndex < nodeCount );
      Vertex &vertex = vertices[nodeIndex];
      vertex.x = toDouble( chunks[2] );
      vertex.y = toDouble( chunks[3] );

      nodeIndex++;
    }
  }

  for ( std::vector<Face>::iterator it = faces.begin(); it != faces.end(); ++it )
  {
    Face &face = *it;
    for ( Face::size_type nd = 0; nd < face.size(); ++nd )
    {
      size_t nodeID = face[nd];

      std::map<size_t, size_t>::iterator ni2i = nodeIDtoIndex.find( nodeID );
      if ( ni2i != nodeIDtoIndex.end() )
      {
        face[nd] = ni2i->second; // convert from ID to index
      }
      else
      {
        assert( false ); //TODO mark element as unusable

        if ( status ) *status = Status::Warn_ElementWithInvalidNode;
      }
    }

    //TODO check validity of the face
    //check that we have distinct nodes
  }

  Mesh *mesh = new Mesh;
  mesh->faces = faces;
  mesh->vertices = vertices;

  return mesh;
}
