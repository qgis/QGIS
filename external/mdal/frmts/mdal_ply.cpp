/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Runette Software Ltd.
*/

#include <stddef.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <cassert>
#include <limits>
#include <algorithm>
#include <string.h>

#include "mdal_ply.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_logger.hpp"
#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"

#define DRIVER_NAME "PLY"

MDAL::DriverPly::DriverPly() :
  Driver( DRIVER_NAME,
          "Stanford PLY Ascii Mesh File",
          "*.ply",
          Capability::ReadMesh
        )
{
}

MDAL::DriverPly *MDAL::DriverPly::create()
{
  return new DriverPly();
}

MDAL::DriverPly::~DriverPly() = default;

size_t MDAL::DriverPly::getIndex( std::vector<std::string> v, std::string in )
{
  std::vector<std::string>::iterator it = std::find( v.begin(), v.end(), in );
  return ( size_t )std::distance( v.begin(), it );
}

// check for the magic number which in  a PLY file is "ply"
bool MDAL::DriverPly::canReadMesh( const std::string &uri )
{
  std::ifstream in( uri, std::ifstream::in );
  std::string line;
  if ( !MDAL::getHeaderLine( in, line ) || !startsWith( line, "ply" ) )
  {
    return false;
  }
  return true;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverPly::load( const std::string &meshFile, const std::string & )
{
  MDAL::Log::resetLastStatus();

  std::ifstream in( meshFile, std::ifstream::in );
  std::string line;

  // Read header
  size_t vertexCount = 0;
  size_t faceCount = 0;
  size_t edgeCount = 0;
  std::vector<std::string> chunks;
  std::vector<MDAL::DriverPly::element> elements; // we will decant the element specifications into here
  std::string proj = "";

  if ( !std::getline( in, line ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " Header is corrupt" );
    return nullptr;
  }

  /*
  * The header is a format defintion and a series of element definitions and/or comment lines
  * cycle through these until end-header
  */
  do
  {
    /*
    * iterate through all of the  element blocks in the header and enumerate the number of members and the properties
    */
    if ( startsWith( line, "element" ) )
    {
      chunks = split( line, ' ' );
      if ( ( chunks.size() != 3 ) )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " invalid number of vertexes" );
        return nullptr;
      }
      MDAL::DriverPly::element element;
      element.name = chunks[1];
      element.size = MDAL::toSizeT( chunks[2] );
      do
      {
        if ( !std::getline( in, line ) )
        {
          MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " Header is corrupt" );
          return nullptr;
        }
        if ( startsWith( line, "property" ) )
        {
          chunks = split( line, ' ' );
          switch ( chunks.size() )
          {
            case 3:
              element.properties.push_back( chunks[2] );
              element.types.push_back( chunks[1] );
              element.list.push_back( false );
              break;
            case 5:
              element.properties.push_back( chunks[4] );
              element.types.push_back( chunks[3] );
              element.list.push_back( true );
              break;
            default:
              MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " invalid element line" + line );
              return nullptr;
          }
        }
        else
        {
          break;
        }
      }
      while ( true );
      elements.push_back( element );
    }

    // if end - stop looping
    else if ( startsWith( line, "end_header" ) )
    {
      break;
    }

    //if binary - give up

    else if ( startsWith( line, "binary" ) )
    {
      MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " only ASCII format PLY files are supported" );
      return nullptr;
    }

    // if "comment crs" assume that the rest is the crs data
    else if ( startsWith( line, "comment crs " ) )
    {
      line.erase( 0, 12 );
      proj = line;
      if ( !std::getline( in, line ) )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " Header is corrupt" );
        return nullptr;
      }
    }

    // probably a comment line
    else
    {
      if ( !std::getline( in, line ) )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " Header is corrupt" );
        return nullptr;
      }
    }
  }
  while ( true );

  Vertices vertices( 0 );
  Faces faces( 0 );
  Edges edges( 0 );
  size_t maxSizeFace = 0;
  size_t faceSize = 0;

  //datastructures that will contain all of the datasets, categorised by vertex, face and edge datasets
  std::vector<std::vector<double>> vertexDatasets; // conatins the data
  std::vector<std::string> vProp2Ds; // contains the dataset names
  std::vector<std::vector<double>> faceDatasets;
  std::vector<std::string> fProp2Ds;
  std::vector<std::vector<double>> edgeDatasets;
  std::vector<std::string> eProp2Ds;

  /*
  * load the elements in order
  */
  if ( !std::getline( in, line ) )
  {
    MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " does not contain any definitions" );
    return nullptr;
  }
  chunks = split( line, ' ' );

  // configure the vectors to hold the data

  for ( size_t elid = 0; elid < elements.size(); ++elid )
  {
    MDAL::DriverPly::element element = elements[elid];
    if ( element.name == "vertex" )
    {
      vertexCount = element.size;
      vertices.resize( vertexCount );
      for ( size_t i = 0; i < element.properties.size(); ++i )
      {
        if ( element.properties[i] != "x" &&
             element.properties[i] != "y" &&
             element.properties[i] != "z" &&
             !element.list[i] )
        {
          vertexDatasets.push_back( * new std::vector<double> );
          vProp2Ds.push_back( element.properties[i] );
        }
      }
    }
    else if ( element.name == "face" )
    {
      faceCount = element.size;
      faces.resize( faceCount );
      for ( size_t i = 0; i < element.properties.size(); ++i )
      {
        if ( element.properties[i] != "vertex_indices" &&
             !element.list[i] )
        {
          faceDatasets.push_back( * new std::vector<double> );
          fProp2Ds.push_back( element.properties[i] );
        }
      }
    }
    else if ( element.name == "edge" )
    {
      edgeCount = element.size;
      edges.resize( edgeCount );
      for ( size_t i = 0; i < element.properties.size(); ++i )
      {
        if ( element.properties[i] != "vertex1" &&
             element.properties[i] != "vertex2" &&
             !element.list[i] )
        {
          edgeDatasets.push_back( * new std::vector<double> );
          eProp2Ds.push_back( element.properties[i] );
        }
      }
    }

    // load the data
    for ( size_t i = 0; i < element.size; ++i )
    {
      /*
      * set the line size - we will only deal with one list at the begining of the line
      */
      size_t nChunks = element.properties.size();
      if ( element.list[0] ) nChunks += MDAL::toSizeT( chunks[0] );
      if ( chunks.size() != nChunks )
      {
        MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " contains invalid line : " + line );
        return nullptr;
      }

      /*
      * Load the vertexes
      */
      if ( element.name == "vertex" )
      {
        Vertex &vertex = vertices[i];
        vertex.x = MDAL::toDouble( chunks[MDAL::DriverPly::getIndex( element.properties, "x" )] );
        vertex.y = MDAL::toDouble( chunks[MDAL::DriverPly::getIndex( element.properties, "y" )] );
        vertex.z = MDAL::toDouble( chunks[MDAL::DriverPly::getIndex( element.properties, "z" )] );
        for ( size_t j = 0; j < vProp2Ds.size(); ++j )
        {
          double value = MDAL::toDouble( chunks[MDAL::DriverPly::getIndex( element.properties, vProp2Ds[j] )] );
          vertexDatasets[j].push_back( value );
        }
      }

      /*
      *load the faces
      */
      else if ( element.name == "face" )
      {
        faceSize = MDAL::toSizeT( chunks[0] );
        Face &face = faces[i];
        face.resize( faceSize );
        for ( size_t j = 0; j < faceSize; ++j )
        {
          face[j] = MDAL::toSizeT( chunks[j + 1] );
        }
        if ( faceSize > maxSizeFace ) maxSizeFace = faceSize;
        for ( size_t j = 0; j < fProp2Ds.size(); ++j )
        {
          double value = MDAL::toDouble( chunks[MDAL::DriverPly::getIndex( element.properties, fProp2Ds[j] ) + faceSize] );
          faceDatasets[j].push_back( value );
        }
      }

      /*
      load the edges
      */
      else if ( element.name == "edge" )
      {
        Edge &edge = edges[i];
        edge.startVertex = MDAL::toSizeT( chunks[MDAL::DriverPly::getIndex( element.properties, "vertex1" )] );
        edge.endVertex = MDAL::toSizeT( chunks[MDAL::DriverPly::getIndex( element.properties, "vertex2" )] );
        for ( size_t j = 0; j < eProp2Ds.size(); ++j )
        {
          double value = MDAL::toDouble( chunks[MDAL::DriverPly::getIndex( element.properties, eProp2Ds[j] )] );
          edgeDatasets[j].push_back( value );
        }
      }

      // any other element ignore and move to the next line
      if ( !std::getline( in, line ) )
      {
        // if it is supposed to be the last line - don't look further
        if ( elid != ( elements.size() - 1 ) && i != ( element.size - 1 ) )
        {
          MDAL::Log::error( MDAL_Status::Err_IncompatibleMesh, name(), meshFile + " does not contain enough definitions of type " + element.name );
          return nullptr;
        }
      }
      chunks = split( line, ' ' );
    }
  }

  std::unique_ptr< MemoryMesh > mesh(
    new MemoryMesh(
      DRIVER_NAME,
      maxSizeFace,
      meshFile
    )
  );
  mesh->setFaces( std::move( faces ) );
  mesh->setVertices( std::move( vertices ) );
  mesh->setEdges( std::move( edges ) );
  mesh->setSourceCrs( proj );

  // Add Bed Elevation
  MDAL::addBedElevationDatasetGroup( mesh.get(), mesh->vertices() );

  for ( size_t i = 0; i < vertexDatasets.size(); ++i )
  {
    std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), vProp2Ds[i], DataOnVertices, true );
    addDataset( group.get(), vertexDatasets[i] );
  }

  for ( size_t i = 0; i < faceDatasets.size(); ++i )
  {
    std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), fProp2Ds[i], DataOnFaces, true );
    addDataset( group.get(), faceDatasets[i] );
  }

  for ( size_t i = 0; i < edgeDatasets.size(); ++i )
  {
    std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), eProp2Ds[i], DataOnEdges, true );
    addDataset( group.get(), edgeDatasets[i] );
  }

  /*
  * Clean up
  */
  for ( size_t i = 0; i < vertexDatasets.size(); ++i )
  {
    vertexDatasets.pop_back();
  };

  for ( size_t i = 0; i < faceDatasets.size(); ++i )
  {
    faceDatasets.pop_back();
  };

  for ( size_t i = 0; i < edgeDatasets.size(); ++i )
  {
    edgeDatasets.pop_back();
  };

  return std::unique_ptr<Mesh>( mesh.release() );

}

std::shared_ptr< MDAL::DatasetGroup> MDAL::DriverPly::addDatasetGroup( MDAL::Mesh *mesh, const std::string &name, const MDAL_DataLocation location, bool isScalar )
{
  if ( !mesh )
    return NULL;

  if ( location == DataOnFaces && mesh->facesCount() == 0 )
    return NULL;

  if ( location == DataOnEdges && mesh->edgesCount() == 0 )
    return NULL;

  std::shared_ptr< DatasetGroup > group = std::make_shared< DatasetGroup >( mesh->driverName(), mesh, name, name );
  group->setDataLocation( location );
  group->setIsScalar( isScalar );
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mesh->datasetGroups.push_back( group );
  return group;
}

void MDAL::DriverPly::addDataset( MDAL::DatasetGroup *group, const std::vector<double> &values )
{
  if ( !group )
    return;

  MDAL::Mesh *mesh = group->mesh();

  if ( values.empty() )
    return;

  if ( 0 == mesh->verticesCount() )
    return;

  if ( group->dataLocation() == DataOnVertices )
  {
    assert( values.size() == mesh->verticesCount() );
  }

  if ( group->dataLocation() == DataOnFaces )
  {
    assert( values.size() == mesh->facesCount() );
    if ( mesh->facesCount() == 0 )
      return;
  }

  if ( group->dataLocation() == DataOnEdges )
  {
    assert( values.size() == mesh->edgesCount() );
    if ( mesh->edgesCount() == 0 )
      return;
  }

  std::shared_ptr< MDAL::MemoryDataset2D > dataset = std::make_shared< MemoryDataset2D >( group );
  dataset->setTime( 0.0 );
  memcpy( dataset->values(), values.data(), sizeof( double ) * values.size() );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
  group->setStatistics( MDAL::calculateStatistics( group ) );
}
