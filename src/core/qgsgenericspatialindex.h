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

#ifndef QGSGENERICSPATIALINDEX_H
#define QGSGENERICSPATIALINDEX_H

#include "qgis_core.h"
#include "qgsspatialindexutils.h"
#include "qgslogger.h"

#include <memory>
#include <QMutex>
#include <QString>

#define SIP_NO_FILE

#include <functional>
#include <spatialindex/SpatialIndex.h>

class QgsRectangle;

/**
 * \ingroup core
 * \class QgsGenericSpatialIndex
 *
 * \brief A generic rtree spatial index based on a libspatialindex backend.
 *
 * \note Not available in Python bindings.
 * \since QGIS 3.12
 */
template <typename T>
class QgsGenericSpatialIndex
{
  public:

    /**
     * Constructor for QgsGenericSpatialIndex.
     */
    QgsGenericSpatialIndex()
    {
      mStorageManager.reset( SpatialIndex::StorageManager::createNewMemoryStorageManager() );
      mRTree = createSpatialIndex( *mStorageManager );
    }

    /**
     * Inserts new \a data into the spatial index, with the specified \a bounds.
     *
     * Ownership of \a data is not transferred, and it is the caller's responsibility to ensure that
     * it exists for the lifetime of the spatial index.
     */
    bool insert( T *data, const QgsRectangle &bounds )
    {
      const SpatialIndex::Region r( QgsSpatialIndexUtils::rectangleToRegion( bounds ) );

      const QMutexLocker locker( &mMutex );

      const qint64 id = mNextId++;
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

    /**
     * Removes existing \a data from the spatial index, with the specified \a bounds.
     *
     * \a data is not deleted, and it is the caller's responsibility to ensure that
     * it is appropriately cleaned up.
     */
    bool remove( T *data, const QgsRectangle &bounds )
    {
      const SpatialIndex::Region r = QgsSpatialIndexUtils::rectangleToRegion( bounds );

      const QMutexLocker locker( &mMutex );

      const qint64 id = mDataToId.value( data, 0 );
      if ( id == 0 )
        return false;

      // TODO: handle exceptions
      const bool res = mRTree->deleteData( r, id );
      mDataToId.remove( data );
      mIdToData.remove( id );
      return res;
    }

    /**
     * Performs an intersection check against the index, for data intersecting the specified \a bounds.
     *
     * The \a callback function will be called once for each matching data object encountered.
     */
    bool intersects( const QgsRectangle &bounds, const std::function< bool( T *data )> &callback ) const
    {
      GenericIndexVisitor<T> visitor( callback, mIdToData );
      const SpatialIndex::Region r = QgsSpatialIndexUtils::rectangleToRegion( bounds );

      const QMutexLocker locker( &mMutex );
      mRTree->intersectsWithQuery( r, visitor );
      return true;
    }

    /**
     * Returns TRUE if the index contains no items.
     */
    bool isEmpty( ) const
    {
      const QMutexLocker locker( &mMutex );
      return mIdToData.isEmpty();
    }

  private:

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

    std::unique_ptr< SpatialIndex::IStorageManager > mStorageManager;
    std::unique_ptr< SpatialIndex::ISpatialIndex > mRTree;

    mutable QMutex mMutex;

    qint64 mNextId = 1;
    QHash< qint64, T * > mIdToData;
    QHash< T *, qint64 > mDataToId;

    template <typename TT>
    class GenericIndexVisitor : public SpatialIndex::IVisitor
    {
      public:
        explicit GenericIndexVisitor( const std::function< bool( TT *data )> &callback, const QHash< qint64, TT * > &data )
          : mCallback( callback )
          , mData( data )
        {}

        void visitNode( const SpatialIndex::INode &n ) override
        { Q_UNUSED( n ) }

        void visitData( const SpatialIndex::IData &d ) override
        {
          const qint64 id = d.getIdentifier();
          T *data = mData.value( id );
          mCallback( data );
        }

        void visitData( std::vector<const SpatialIndex::IData *> &v ) override
        { Q_UNUSED( v ) }

      private:
        const std::function< bool( TT *data )> &mCallback;
        QHash< qint64, TT * > mData;
    };

};

#endif // QGSGENERICSPATIALINDEX_H
