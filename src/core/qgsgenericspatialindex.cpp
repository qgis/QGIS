/***************************************************************************
  qgsgenericspatialindex.h
  ------------------------
  Date                 : December 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgenericspatialindex.h"
#include "qgis.h"
#include "qgsspatialindexutils.h"
#include "qgslogger.h"

#include <memory>
#include <spatialindex/SpatialIndex.h>

std::unique_ptr< SpatialIndex::ISpatialIndex > createSpatialIndex( SpatialIndex::IStorageManager &storageManager )
{
  // R-Tree parameters
  constexpr double fillFactor = 0.7;
  constexpr unsigned long indexCapacity = 10;
  constexpr unsigned long leafCapacity = 10;
  constexpr unsigned long dimension = 2;
  constexpr SpatialIndex::RTree::RTreeVariant variant = SpatialIndex::RTree::RV_RSTAR;

  // create R-tree
  SpatialIndex::id_type indexId;
  return std::unique_ptr< SpatialIndex::ISpatialIndex >( SpatialIndex::RTree::createNewRTree( storageManager, fillFactor, indexCapacity,
         leafCapacity, dimension, variant, indexId ) );
}

///@cond PRIVATE
template <typename T>
class GenericIndexVisitor : public SpatialIndex::IVisitor
{
  public:
    explicit GenericIndexVisitor( const std::function< bool( const T *data )> &callback, const QHash< qint64, const T * > &data )
      : mCallback( callback )
      , mData( data )
    {}

    void visitNode( const SpatialIndex::INode &n ) override
    { Q_UNUSED( n ) }

    void visitData( const SpatialIndex::IData &d ) override
    {
      qint64 id = d.getIdentifier();
      const T *data = mData.value( id );
      mCallback( data );
    }

    void visitData( std::vector<const SpatialIndex::IData *> &v ) override
    { Q_UNUSED( v ) }

  private:
    const std::function< bool( const T *data )> &mCallback;
    QHash< qint64, const T * > mData;
};

///@endcond


template <class T>
QgsGenericSpatialIndex< T >::QgsGenericSpatialIndex()
  : mMutex( QMutex::Recursive )
{
  mStorageManager.reset( SpatialIndex::StorageManager::createNewMemoryStorageManager() );
  mRTree = createSpatialIndex( *mStorageManager );
}

template<typename T>
QgsGenericSpatialIndex<T>::~QgsGenericSpatialIndex() = default;

template<typename T>
bool QgsGenericSpatialIndex<T>::insertData( const T *data, const QgsRectangle &bounds )
{
  SpatialIndex::Region r( QgsSpatialIndexUtils::rectangleToRegion( bounds ) );

  QMutexLocker locker( &mMutex );

  qint64 id = mNextId++;
  mIdToData.insert( id, data );
  mDataToId.insert( data, id );
  try
  {
    mRTree->insertData( 0, nullptr, r, static_cast< qint64 >( id ) );
    return true;
  }
  catch ( Tools::Exception &e )
  {
    Q_UNUSED( e )
    QgsDebugMsg( QStringLiteral( "Tools::Exception caught: " ).arg( e.what().c_str() ) );
  }
  catch ( const std::exception &e )
  {
    Q_UNUSED( e )
    QgsDebugMsg( QStringLiteral( "std::exception caught: " ).arg( e.what() ) );
  }
  catch ( ... )
  {
    QgsDebugMsg( QStringLiteral( "unknown spatial index exception caught" ) );
  }

  return false;
}

template<typename T>
bool QgsGenericSpatialIndex<T>::deleteData( const T *data, const QgsRectangle &bounds )
{
  SpatialIndex::Region r = QgsSpatialIndexUtils::rectangleToRegion( bounds );

  QMutexLocker locker( &mMutex );

  const qint64 id = mDataToId.value( data, 0 );
  if ( id == 0 )
    return false;

  // TODO: handle exceptions
  bool res = mRTree->deleteData( r, id );
  mDataToId.remove( data );
  mIdToData.remove( id );
  return res;
}

template<typename T>
bool QgsGenericSpatialIndex<T>::intersects( const QgsRectangle &rectangle, const std::function<bool ( const T * )> &callback ) const
{
  GenericIndexVisitor<T> visitor( callback, mIdToData );
  SpatialIndex::Region r = QgsSpatialIndexUtils::rectangleToRegion( rectangle );

  QMutexLocker locker( &mMutex );
  mRTree->intersectsWithQuery( r, visitor );
  return true;
}

namespace pal
{
  class FeaturePart;
  class LabelPosition;
}

template class QgsGenericSpatialIndex<pal::FeaturePart>;
template class QgsGenericSpatialIndex<pal::LabelPosition>;

class QgsLabelPosition;
template class QgsGenericSpatialIndex<QgsLabelPosition>;
