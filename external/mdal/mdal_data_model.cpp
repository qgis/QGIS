/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_data_model.hpp"
#include <assert.h>
#include <math.h>
#include <algorithm>
#include "mdal_utils.hpp"

MDAL::Dataset::~Dataset() = default;

MDAL::Dataset::Dataset( MDAL::DatasetGroup *parent )
  : mParent( parent )
{
  assert( mParent );
}

std::string MDAL::Dataset::driverName() const
{
  return group()->driverName();
}

size_t MDAL::Dataset::valuesCount() const
{
  if ( group()->isOnVertices() )
  {
    return mesh()->verticesCount();
  }
  else
  {
    return mesh()->facesCount();
  }
}

MDAL::Statistics MDAL::Dataset::statistics() const
{
  return mStatistics;
}

void MDAL::Dataset::setStatistics( const MDAL::Statistics &statistics )
{
  mStatistics = statistics;
}

MDAL::DatasetGroup *MDAL::Dataset::group() const
{
  return mParent;
}

MDAL::Mesh *MDAL::Dataset::mesh() const
{
  return mParent->mesh();
}

double MDAL::Dataset::time() const
{
  return mTime;
}

void MDAL::Dataset::setTime( double time )
{
  mTime = time;
}

bool MDAL::Dataset::isValid() const
{
  return mIsValid;
}

void MDAL::Dataset::setIsValid( bool isValid )
{
  mIsValid = isValid;
}

MDAL::DatasetGroup::DatasetGroup( const std::string &driverName,
                                  MDAL::Mesh *parent,
                                  const std::string &uri,
                                  const std::string &name )
  : mDriverName( driverName )
  , mParent( parent )
  , mUri( uri )
{
  assert( mParent );
  setName( name );
}

std::string MDAL::DatasetGroup::driverName() const
{
  return mDriverName;
}

MDAL::DatasetGroup::~DatasetGroup() = default;

MDAL::DatasetGroup::DatasetGroup( const std::string &driverName,
                                  MDAL::Mesh *parent,
                                  const std::string &uri )
  : mDriverName( driverName )
  , mParent( parent )
  , mUri( uri )
{
  assert( mParent );
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

std::string MDAL::DatasetGroup::uri() const
{
  return mUri;
}

MDAL::Statistics MDAL::DatasetGroup::statistics() const
{
  return mStatistics;
}

void MDAL::DatasetGroup::setStatistics( const Statistics &statistics )
{
  mStatistics = statistics;
}

std::string MDAL::DatasetGroup::referenceTime() const
{
  return mReferenceTime;
}

void MDAL::DatasetGroup::setReferenceTime( const std::string &referenceTime )
{
  mReferenceTime = referenceTime;
}

MDAL::Mesh *MDAL::DatasetGroup::mesh() const
{
  return mParent;
}

bool MDAL::DatasetGroup::isInEditMode() const
{
  return mInEditMode;
}

void MDAL::DatasetGroup::startEditing()
{
  mInEditMode = true;
}

void MDAL::DatasetGroup::stopEditing()
{
  mInEditMode = false;
}

bool MDAL::DatasetGroup::isOnVertices() const
{
  return mIsOnVertices;
}

void MDAL::DatasetGroup::setIsOnVertices( bool isOnVertices )
{
  // datasets are initialized (e.g. values array, active array) based
  // on this property. Do not allow to modify later on.
  assert( datasets.empty() );
  mIsOnVertices = isOnVertices;
}

bool MDAL::DatasetGroup::isScalar() const
{
  return mIsScalar;
}

void MDAL::DatasetGroup::setIsScalar( bool isScalar )
{
  // datasets are initialized (e.g. values array, active array) based
  // on this property. Do not allow to modify later on.
  assert( datasets.empty() );
  mIsScalar = isScalar;
}

MDAL::Mesh::Mesh(
  const std::string &driverName,
  size_t verticesCount,
  size_t facesCount,
  size_t faceVerticesMaximumCount,
  MDAL::BBox extent,
  const std::string &uri )
  : mDriverName( driverName )
  , mVerticesCount( verticesCount )
  , mFacesCount( facesCount )
  , mFaceVerticesMaximumCount( faceVerticesMaximumCount )
  , mExtent( extent )
  , mUri( uri )
{
}

std::string MDAL::Mesh::driverName() const
{
  return mDriverName;
}

MDAL::Mesh::~Mesh() = default;

std::shared_ptr<MDAL::DatasetGroup> MDAL::Mesh::group( const std::string &name )
{
  for ( auto grp : datasetGroups )
  {
    if ( grp->name() == name )
      return grp;
  }
  return std::shared_ptr<MDAL::DatasetGroup>();
}

void MDAL::Mesh::setSourceCrs( const std::string &str )
{
  mCrs = MDAL::trim( str );
}

void MDAL::Mesh::setSourceCrsFromWKT( const std::string &wkt )
{
  setSourceCrs( wkt );
}

void MDAL::Mesh::setSourceCrsFromEPSG( int code )
{
  setSourceCrs( std::string( "EPSG:" ) + std::to_string( code ) );
}

size_t MDAL::Mesh::verticesCount() const
{
  return mVerticesCount;
}

size_t MDAL::Mesh::facesCount() const
{
  return mFacesCount;
}

std::string MDAL::Mesh::uri() const
{
  return mUri;
}

MDAL::BBox MDAL::Mesh::extent() const
{
  return mExtent;
}

std::string MDAL::Mesh::crs() const
{
  return mCrs;
}

size_t MDAL::Mesh::faceVerticesMaximumCount() const
{
  return mFaceVerticesMaximumCount;
}

MDAL::MeshVertexIterator::~MeshVertexIterator() = default;

MDAL::MeshFaceIterator::~MeshFaceIterator() = default;
