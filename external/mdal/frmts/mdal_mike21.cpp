/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2023 Lutra Consulting Ltd.
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
#include <regex>

#include "mdal_mike21.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_logger.hpp"

#define DRIVER_NAME "Mike21"

// function to split using regex, by default split on whitespace characters
std::vector<std::string> regex_split( const std::string &input, const std::regex &split_regex = std::regex{"\\s+"} )
{
  std::sregex_token_iterator iter( input.begin(), input.end(), split_regex, -1 );
  std::sregex_token_iterator end;
  return {iter, end};
}

static bool parse_vertex_id_gaps( std::map<size_t, size_t> &vertexIDtoIndex, size_t vertexIndex, size_t vertexID )
{
  if ( vertexIndex == vertexID )
    return false;

  std::map<size_t, size_t>::iterator search = vertexIDtoIndex.find( vertexID );
  if ( search != vertexIDtoIndex.end() )
  {
    MDAL::Log::warning( Warn_ElementNotUnique, DRIVER_NAME, "could not find vertex" );
    return true;
  }

  vertexIDtoIndex[vertexID] = vertexIndex;
  return false;
}

static void persist_native_index( std::vector<double> &arr, size_t nativeID, size_t ourId, size_t maxOurId )
{
  if ( !arr.empty() || ( nativeID != ourId + 1 ) )
  {
    // we have gaps in face indexing
    if ( arr.empty() )
    {
      arr.resize( maxOurId );
      for ( size_t i = 0; i < ourId; ++i )
        arr[i] = static_cast<double>( i + 1 );
    }
    arr[ourId] = static_cast<double>( nativeID );
  }
}

MDAL::MeshMike21::MeshMike21( size_t faceVerticesMaximumCount,
                              const std::string &uri,
                              const std::map<size_t, size_t> vertexIDtoIndex )
  : MemoryMesh( DRIVER_NAME,
                faceVerticesMaximumCount,
                uri )
  , mVertexIDtoIndex( vertexIDtoIndex )
{
}

MDAL::MeshMike21::~MeshMike21() = default;

size_t MDAL::MeshMike21::vertexIndex( size_t vertexID ) const
{
  auto ni2i = mVertexIDtoIndex.find( vertexID );
  if ( ni2i != mVertexIDtoIndex.end() )
  {
    return  ni2i->second; // convert from ID to index
  }
  return vertexID;
}

size_t MDAL::MeshMike21::maximumVertexId() const
{
  size_t maxIndex = verticesCount() - 1;
  if ( mVertexIDtoIndex.empty() )
    return maxIndex;
  else
  {
    // std::map is sorted!
    size_t maxID = mVertexIDtoIndex.rbegin()->first;
    return std::max( maxIndex, maxID );
  }
}

MDAL::DriverMike21::DriverMike21( ):
  Driver( DRIVER_NAME,
          "Mike21 Mesh File",
          "*.mesh",
          Capability::ReadMesh | Capability::SaveMesh
        )
{
}

MDAL::DriverMike21 *MDAL::DriverMike21::create()
{
  return new DriverMike21();
}

MDAL::DriverMike21::~DriverMike21() = default;

bool MDAL::DriverMike21::canReadHeader( const std::string &line )
{
  bool header2012 = std::regex_match( line, mRegexHeader2012 );
  bool header2011 = std::regex_match( line, mRegexHeader2011 );
  return header2011 || header2012;
}

bool MDAL::DriverMike21::canReadMesh( const std::string &uri )
{
  std::ifstream in = MDAL::openInputFile( uri );
  std::string line;

  if ( !MDAL::getHeaderLine( in, line ) || !canReadHeader( line ) ||  !MDAL::contains( filters(), MDAL::fileExtension( uri ) ) )
  {
    return false;
  }

  return true;
}

void MDAL::DriverMike21::parseHeader( const std::string &line )
{
  auto matchResults = std::smatch{};
  if ( std::regex_search( line, matchResults, mRegexHeader2012 ) )
  {
    if ( matchResults.size() > 4 )
    {
      mDataType = matchResults[1].str();
      mDataUnit = matchResults[2].str();
      mVertexCount = std::stoi( matchResults[3].str() );
      mCrs = matchResults[4].str();
      return;
    }
  }

  if ( std::regex_search( line, matchResults, mRegexHeader2011 ) )
  {
    if ( matchResults.size() > 2 )
    {
      mVertexCount = std::stoi( matchResults[1].str() );
      mCrs = matchResults[2].str();
      return;
    }
  }
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverMike21::load( const std::string &meshFile, const std::string & )
{
  mMeshFile = meshFile;

  MDAL::Log::resetLastStatus();

  std::ifstream in = MDAL::openInputFile( meshFile );

  std::string line;
  if ( !std::getline( in, line ) || !canReadHeader( line ) )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), meshFile + " could not be opened" );
    return nullptr;
  }

  parseHeader( line );

  size_t faceCount = 0;
  size_t maxVerticesPerFace = 2;

  size_t lineNumber = 1;

  while ( std::getline( in, line ) )
  {
    if ( lineNumber == mVertexCount + 1 )
    {
      auto matchResults = std::smatch{};
      if ( std::regex_search( line, matchResults, mRegexElementHeader ) )
      {
        if ( matchResults.size() >= 4 )
        {
          faceCount = MDAL::toSizeT( matchResults[1].str() );
          maxVerticesPerFace = MDAL::toSizeT( matchResults[2].str() );
          size_t meshType = MDAL::toSizeT( matchResults[3].str() );

          if ( !( meshType == 21 || meshType == 25 ) )
          {
            MDAL::Log::error( MDAL_Status::Err_InvalidData, name(), "unknow mesh type." );
            return nullptr;
          }
        }
        else
        {
          MDAL::Log::error( MDAL_Status::Err_InvalidData, name(), "element header not in valid format." );
          return nullptr;
        }
      }
      else
      {
        MDAL::Log::error( MDAL_Status::Err_InvalidData, name(), "element header not in valid format." );
        return nullptr;
      }

    }
    lineNumber++;
  }

  // number of lines in file does not match number of vertices and faces specifed in first and element line
  if ( lineNumber > 2 + mVertexCount + faceCount )
  {
    MDAL::Log::error( MDAL_Status::Err_InvalidData, name(), "Number of lines in file does not fit with number of vertexes and faces specified." );
    return nullptr;
  }

  in.clear();
  in.seekg( 0, std::ios::beg );

  Vertices vertices( mVertexCount );
  Faces faces( faceCount );

  std::map<size_t, size_t> vertexIDtoIndex;
  std::vector<double> vertexType( mVertexCount );

  std::vector<double> nativeVertexIds;
  std::vector<double> nativeFaceIds;

  size_t lastVertexID = 0;
  size_t faceIndex = 0;
  size_t vertexIndex = 0;

  std::vector<std::string> chunks;
  lineNumber = 0;

  while ( std::getline( in, line ) )
  {
    if ( 0 < lineNumber && lineNumber < mVertexCount + 1 )
    {
      chunks = regex_split( MDAL::trim( line ) );
      if ( chunks.size() != 5 )
      {
        MDAL::Log::error( MDAL_Status::Err_InvalidData, name(), "vertex line in invalid format." );
        return nullptr;
      }

      size_t nodeID = toSizeT( chunks[0] );

      if ( nodeID != 0 )
      {
        // specification of Mike21 does not state if vertexIDs need to continuos, expect that they might be not
        // in the same way as in 2DM
        if ( ( lastVertexID != 0 ) && ( nodeID <= lastVertexID ) )
        {
          // the algorithm requires that the file has points orderer by index
          MDAL::Log::error( MDAL_Status::Err_InvalidData, name(), "nodes are not ordered by index" );
          return nullptr;
        }
        lastVertexID = nodeID;
      }

      // in case we have gaps/reorders in native indexes, store it
      persist_native_index( nativeVertexIds, nodeID, vertexIndex, mVertexCount );
      parse_vertex_id_gaps( vertexIDtoIndex, vertexIndex, nodeID - 1 );

      assert( vertexIndex < mVertexCount );
      Vertex &vertex = vertices[vertexIndex];
      vertex.x = toDouble( chunks[1] );
      vertex.y = toDouble( chunks[2] );
      vertex.z = toDouble( chunks[3] );
      vertexType[vertexIndex] = MDAL::toInt( chunks[4] );
      vertexIndex++;
    }

    if ( mVertexCount + 1 < lineNumber )
    {
      chunks = regex_split( MDAL::trim( line ) );
      assert( faceIndex < faceCount );

      size_t faceVertexCount = chunks.size() - 1;
      // if the face should have 4 vertexes last chunk has value 0
      // it actually means that there are only 3 vertexes
      if ( faceVertexCount == 4 && chunks.size() == 5 )
      {
        if ( MDAL::toSizeT( chunks[4] ) == 0 )
          faceVertexCount = faceVertexCount - 1;
      }

      assert( ( faceVertexCount == 3 ) || ( faceVertexCount == 4 ) );
      if ( maxVerticesPerFace < faceVertexCount )
        maxVerticesPerFace = faceVertexCount;

      Face &face = faces[faceIndex];
      face.resize( faceVertexCount );

      // in case we have gaps/reorders in native indexes, store it
      size_t nativeID = MDAL::toSizeT( chunks[0] );
      persist_native_index( nativeFaceIds, nativeID, faceIndex, faceCount );

      for ( size_t i = 0; i < faceVertexCount; ++i )
        face[i] = MDAL::toSizeT( chunks[i + 1] ) - 1; // Mike21 is numbered from 1

      faceIndex++;
    }

    lineNumber++;
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
        MDAL::Log::warning( MDAL_Status::Warn_ElementWithInvalidNode, name(), "found invalid node" );
      }
    }
  }

  // create the mesh and set the required data
  std::unique_ptr< MeshMike21 > mesh(
    new MeshMike21(
      maxVerticesPerFace,
      mMeshFile,
      vertexIDtoIndex
    )
  );
  mesh->setFaces( std::move( faces ) );
  mesh->setVertices( std::move( vertices ) );

  // Add Vertex Type
  MDAL::addVertexScalarDatasetGroup( mesh.get(), vertexType, "VertexType" );

  // Add Bed Elevation
  MDAL::addBedElevationDatasetGroup( mesh.get(), mesh->vertices() );

  if ( !nativeFaceIds.empty() )
    MDAL::addFaceScalarDatasetGroup( mesh.get(), nativeFaceIds, "NativeFaceIds" );
  if ( !nativeVertexIds.empty() )
    MDAL::addVertexScalarDatasetGroup( mesh.get(), nativeVertexIds, "NativeVertexIds" );

  mesh->setSourceCrs( mCrs );
  mesh->setMetadata( "crs", mCrs );
  if ( !mDataType.empty() )
    mesh->setMetadata( "data_type", mDataType );

  if ( !mDataUnit.empty() )
    mesh->setMetadata( "data_unit", mDataUnit );

  return std::unique_ptr<Mesh>( mesh.release() );
}

void MDAL::DriverMike21::save( const std::string &fileName, const std::string &, MDAL::Mesh *mesh )
{
  MDAL::Log::resetLastStatus();

  std::ofstream file = MDAL::openOutputFile( fileName, std::ofstream::out );

  if ( !file.is_open() )
  {
    MDAL::Log::error( MDAL_Status::Err_FailToWriteToDisk, name(), "Could not open file " + fileName );
  }

  std::string line;

  const std::string dataType = mesh->getMetadata( "data_type" );
  const std::string dataUnit = mesh->getMetadata( "data_unit" );
  if ( !dataType.empty() && !dataUnit.empty() )
    line.append( dataType + " " + dataUnit + " " );

  line.append( std::to_string( mesh->verticesCount() ) + " " + mesh->getMetadata( "crs" ) );

  file << line << std::endl;

  std::vector<double> vertexTypes;

  std::shared_ptr<MDAL::DatasetGroup> vertexTypeDG = mesh->group( "VertexType" );
  if ( vertexTypeDG )
  {
    vertexTypes.resize( mesh->verticesCount() );
    auto d = vertexTypeDG->datasets[0];
    d->scalarData( 0, mesh->verticesCount(), vertexTypes.data() );
  }

  // write vertices
  std::unique_ptr<MDAL::MeshVertexIterator> vertexIterator = mesh->readVertices();
  double vertex[3];
  for ( size_t i = 0; i < mesh->verticesCount(); ++i )
  {
    vertexIterator->next( 1, vertex );
    line = "";
    line.append( std::to_string( i + 1 ) );
    for ( size_t j = 0; j < 2; ++j )
    {
      line.append( " " );
      line.append( MDAL::coordinateToString( vertex[j] ) );
    }
    line.append( " " );
    line.append( MDAL::doubleToString( vertex[2] ) );

    line.append( " " );
    if ( vertexTypes.size() == mesh->verticesCount() )
    {
      line.append( MDAL::doubleToString( vertexTypes.at( i ) ) );
    }
    else
    {
      line.append( MDAL::doubleToString( 0 ) );
    }

    file << line << std::endl;
  }

  //write element header line
  size_t elementType = 0;
  if ( mesh->faceVerticesMaximumCount() == 3 )
  {
    elementType = 21;
  }
  else if ( mesh->faceVerticesMaximumCount() == 4 )
  {
    elementType = 25;
  }

  line = std::to_string( mesh->facesCount() );
  line.append( " " );
  line.append( std::to_string( mesh->faceVerticesMaximumCount() ) );
  line.append( " " );
  line.append( std::to_string( elementType ) );
  file << line << std::endl;

  // write faces
  std::vector<int> vertexIndices( mesh->faceVerticesMaximumCount() );
  std::unique_ptr<MDAL::MeshFaceIterator> faceIterator = mesh->readFaces();
  for ( size_t i = 0; i < mesh->facesCount(); ++i )
  {
    int faceOffsets[1];
    faceIterator->next( 1, faceOffsets, 4, vertexIndices.data() );

    if ( faceOffsets[0] > 2 && faceOffsets[0] < 5 )
    {
      line = "";
      line.append( std::to_string( i + 1 ) );

      for ( int j = 0; j < faceOffsets[0]; ++j )
      {
        line.append( " " );
        line.append( std::to_string( vertexIndices[j] + 1 ) );
      }

      // if face has 3 vertexes but the mesh as whole is marked as having
      // 4 vertex at maximum, the last element should 0 - indicating no vertex there
      if ( faceOffsets[0] == 3 && mesh->faceVerticesMaximumCount() == 4 )
      {
        line.append( " " );
        line.append( "0" );
      }

    }
    file << line << std::endl;
  }

  file.close();
}

std::string MDAL::DriverMike21::saveMeshOnFileSuffix() const
{
  return "mesh";
}
