/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_data_model.hpp"
#include <assert.h>
#include <algorithm>

bool MDAL::Dataset::isActive( size_t faceIndex )
{
  assert( parent );
  if ( parent->isOnVertices )
  {
    if ( active.size() > faceIndex )
      return active[faceIndex];
    else
      return false;
  }
  else
  {
    return true;
  }
}

std::string MDAL::DatasetGroup::getMetadata( const std::string &key )
{
  for ( auto &pair : metadata )
  {
    if ( pair.first == key )
    {
      return pair.second;
    }
  }
  return std::string();
}

void MDAL::DatasetGroup::setMetadata( const std::string &key, const std::string &val )
{
  bool found = false;
  for ( auto &pair : metadata )
  {
    if ( pair.first == key )
    {
      found = true;
      pair.second = val;
    }
  }
  if ( !found )
    metadata.push_back( std::make_pair( key, val ) );
}

std::string MDAL::DatasetGroup::name()
{
  return getMetadata( "name" );
}

void MDAL::DatasetGroup::setName( const std::string &name )
{
  setMetadata( "name", name );
}

void MDAL::Mesh::addBedElevationDataset()
{
  if ( faces.empty() )
    return;

  std::shared_ptr<DatasetGroup> group( new DatasetGroup );
  group->isOnVertices = true;
  group->isScalar = true;
  group->setName( "Bed Elevation" );
  group->uri = uri;
  std::shared_ptr<MDAL::Dataset> dataset( new Dataset );
  dataset->time = 0.0;
  dataset->values.resize( vertices.size() );
  dataset->active.resize( faces.size() );
  dataset->parent = group.get();
  std::fill( dataset->active.begin(), dataset->active.end(), 1 );
  for ( size_t i = 0; i < vertices.size(); ++i )
  {
    dataset->values[i].x = vertices[i].z;
  }
  group->datasets.push_back( dataset );
  datasetGroups.push_back( group );
}
