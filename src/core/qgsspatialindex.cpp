/***************************************************************************
    qgsspatialindex.cpp  - wrapper class for spatial index library
    ----------------------
    begin                : December 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspatialindex.h"

#include "qgsgeometry.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsrectangle.h"
#include "qgslogger.h"
#include "qgsfeaturesource.h"
#include "qgsfeedback.h"
#include "qgsspatialindexutils.h"

#include <spatialindex/SpatialIndex.h>
#include <QMutex>
#include <QMutexLocker>

using namespace SpatialIndex;



/**
 * \ingroup core
 * \class QgisVisitor
 * \brief Custom visitor that adds found features to list.
 * \note not available in Python bindings
 */
class QgisVisitor : public SpatialIndex::IVisitor
{
  public:
    explicit QgisVisitor( QList<QgsFeatureId> &list )
      : mList( list ) {}

    void visitNode( const INode &n ) override
    { Q_UNUSED( n ) }

    void visitData( const IData &d ) override
    {
      mList.append( d.getIdentifier() );
    }

    void visitData( std::vector<const IData *> &v ) override
    { Q_UNUSED( v ) }

  private:
    QList<QgsFeatureId> &mList;
};

/**
 * \ingroup core
 * \class QgsSpatialIndexCopyVisitor
 * \note not available in Python bindings
 */
class QgsSpatialIndexCopyVisitor : public SpatialIndex::IVisitor
{
  public:
    explicit QgsSpatialIndexCopyVisitor( SpatialIndex::ISpatialIndex *newIndex )
      : mNewIndex( newIndex ) {}

    void visitNode( const INode &n ) override
    { Q_UNUSED( n ) }

    void visitData( const IData &d ) override
    {
      SpatialIndex::IShape *shape = nullptr;
      d.getShape( &shape );
      mNewIndex->insertData( 0, nullptr, *shape, d.getIdentifier() );
      delete shape;
    }

    void visitData( std::vector<const IData *> &v ) override
    { Q_UNUSED( v ) }

  private:
    SpatialIndex::ISpatialIndex *mNewIndex = nullptr;
};

///@cond PRIVATE
class QgsNearestNeighborComparator : public INearestNeighborComparator
{
  public:

    QgsNearestNeighborComparator( const QHash< QgsFeatureId, QgsGeometry > *geometries, const QgsPointXY &point, double maxDistance )
      : mGeometries( geometries )
      , mGeom( QgsGeometry::fromPointXY( point ) )
      , mMaxDistance( maxDistance )
    {
    }

    QgsNearestNeighborComparator( const QHash< QgsFeatureId, QgsGeometry > *geometries, const QgsGeometry &geometry, double maxDistance )
      : mGeometries( geometries )
      , mGeom( geometry )
      , mMaxDistance( maxDistance )
    {
    }

    const QHash< QgsFeatureId, QgsGeometry > *mGeometries = nullptr;
    QgsGeometry mGeom;
    double mMaxDistance = 0;
    QSet< QgsFeatureId > mFeaturesOutsideMaxDistance;

    double getMinimumDistance( const IShape &query, const IShape &entry ) override
    {
      return query.getMinimumDistance( entry );
    }

    double getMinimumDistance( const IShape &query, const IData &data ) override
    {
      // start with the default implementation, which gives distance to bounding box only
      IShape *pS;
      data.getShape( &pS );
      double dist = query.getMinimumDistance( *pS );
      delete pS;

      // if doing exact distance search, AND either no max distance specified OR the
      // distance to the bounding box is less than the max distance, calculate the exact
      // distance now.
      // (note: if bounding box is already greater than the distance away then max distance, there's no
      // point doing this more expensive calculation, since we can't possibly use this feature anyway!)
      if ( mGeometries && ( mMaxDistance <= 0.0 || dist <= mMaxDistance ) )
      {
        const QgsGeometry other = mGeometries->value( data.getIdentifier() );
        dist = other.distance( mGeom );
      }

      if ( mMaxDistance > 0 && dist > mMaxDistance )
      {
        // feature is outside of maximum distance threshold. Flag it,
        // but "trick" libspatialindex into considering it as just outside
        // our max distance region. This means if there's no other closer features (i.e.,
        // within our actual maximum search distance), the libspatialindex
        // nearest neighbor test will use this feature and not needlessly continue hunting
        // through the remaining more distant features in the index.
        // TODO: add proper API to libspatialindex to allow a maximum search distance in
        // nearest neighbor tests
        mFeaturesOutsideMaxDistance.insert( data.getIdentifier() );
        return mMaxDistance + 0.00000001;
      }
      return dist;
    }
};

/**
 * \ingroup core
 * \class QgsFeatureIteratorDataStream
 * \brief Utility class for bulk loading of R-trees. Not a part of public API.
 * \note not available in Python bindings
*/
class QgsFeatureIteratorDataStream : public IDataStream
{
  public:
    //! constructor - needs to load all data to a vector for later access when bulk loading
    explicit QgsFeatureIteratorDataStream( const QgsFeatureIterator &fi, QgsFeedback *feedback = nullptr, QgsSpatialIndex::Flags flags = QgsSpatialIndex::Flags(),
                                           const std::function< bool( const QgsFeature & ) > *callback = nullptr )
      : mFi( fi )
      , mFeedback( feedback )
      , mFlags( flags )
      , mCallback( callback )
    {
      readNextEntry();
    }

    ~QgsFeatureIteratorDataStream() override
    {
      delete mNextData;
    }

    //! returns a pointer to the next entry in the stream or 0 at the end of the stream.
    IData *getNext() override
    {
      if ( mFeedback && mFeedback->isCanceled() )
        return nullptr;

      RTree::Data *ret = mNextData;
      mNextData = nullptr;
      readNextEntry();
      return ret;
    }

    //! returns true if there are more items in the stream.
    bool hasNext() override { return nullptr != mNextData; }

    //! returns the total number of entries available in the stream.
    uint32_t size() override { Q_ASSERT( false && "not available" ); return 0; }

    //! sets the stream pointer to the first entry, if possible.
    void rewind() override { Q_ASSERT( false && "not available" ); }

    QHash< QgsFeatureId, QgsGeometry > geometries;

  protected:
    void readNextEntry()
    {
      QgsFeature f;
      SpatialIndex::Region r;
      QgsFeatureId id;
      while ( mFi.nextFeature( f ) )
      {
        if ( mCallback )
        {
          const bool res = ( *mCallback )( f );
          if ( !res )
          {
            mNextData = nullptr;
            return;
          }
        }
        if ( QgsSpatialIndex::featureInfo( f, r, id ) )
        {
          mNextData = new RTree::Data( 0, nullptr, r, id );
          if ( mFlags & QgsSpatialIndex::FlagStoreFeatureGeometries )
            geometries.insert( f.id(), f.geometry() );
          return;
        }
      }
    }

  private:
    QgsFeatureIterator mFi;
    RTree::Data *mNextData = nullptr;
    QgsFeedback *mFeedback = nullptr;
    QgsSpatialIndex::Flags mFlags = QgsSpatialIndex::Flags();
    const std::function< bool( const QgsFeature & ) > *mCallback = nullptr;

};


/**
 * \ingroup core
 * \class QgsSpatialIndexData
 * \brief Data of spatial index that may be implicitly shared
 * \note not available in Python bindings
*/
class QgsSpatialIndexData : public QSharedData
{
  public:
    QgsSpatialIndexData( QgsSpatialIndex::Flags flags )
      : mFlags( flags )
    {
      initTree();
    }

    QgsSpatialIndex::Flags mFlags = QgsSpatialIndex::Flags();

    QHash< QgsFeatureId, QgsGeometry > mGeometries;

    /**
     * Constructor for QgsSpatialIndexData which bulk loads features from the specified feature iterator
     * \a fi.
     *
     * The optional \a feedback object can be used to allow cancellation of bulk feature loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.
     */
    explicit QgsSpatialIndexData( const QgsFeatureIterator &fi, QgsFeedback *feedback = nullptr, QgsSpatialIndex::Flags flags = QgsSpatialIndex::Flags(),
                                  const std::function< bool( const QgsFeature & ) > *callback = nullptr )
      : mFlags( flags )
    {
      QgsFeatureIteratorDataStream fids( fi, feedback, mFlags, callback );
      initTree( &fids );
      if ( flags & QgsSpatialIndex::FlagStoreFeatureGeometries )
        mGeometries = fids.geometries;
    }

    QgsSpatialIndexData( const QgsSpatialIndexData &other )
      : QSharedData( other )
      , mFlags( other.mFlags )
      , mGeometries( other.mGeometries )
    {
      const QMutexLocker locker( &other.mMutex );

      initTree();

      // copy R-tree data one by one (is there a faster way??)
      double low[]  = { std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest() };
      double high[] = { std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
      const SpatialIndex::Region query( low, high, 2 );
      QgsSpatialIndexCopyVisitor visitor( mRTree );
      other.mRTree->intersectsWithQuery( query, visitor );
    }

    ~QgsSpatialIndexData()
    {
      delete mRTree;
      delete mStorage;
    }

    QgsSpatialIndexData &operator=( const QgsSpatialIndexData &rh ) = delete;

    void initTree( IDataStream *inputStream = nullptr )
    {
      // for now only memory manager
      mStorage = StorageManager::createNewMemoryStorageManager();

      // R-Tree parameters
      const double fillFactor = 0.7;
      const unsigned long indexCapacity = 10;
      const unsigned long leafCapacity = 10;
      const unsigned long dimension = 2;
      const RTree::RTreeVariant variant = RTree::RV_RSTAR;

      // create R-tree
      SpatialIndex::id_type indexId;

      if ( inputStream && inputStream->hasNext() )
        mRTree = RTree::createAndBulkLoadNewRTree( RTree::BLM_STR, *inputStream, *mStorage, fillFactor, indexCapacity,
                 leafCapacity, dimension, variant, indexId );
      else
        mRTree = RTree::createNewRTree( *mStorage, fillFactor, indexCapacity,
                                        leafCapacity, dimension, variant, indexId );
    }

    //! Storage manager
    SpatialIndex::IStorageManager *mStorage = nullptr;

    //! R-tree containing spatial index
    SpatialIndex::ISpatialIndex *mRTree = nullptr;

    mutable QRecursiveMutex mMutex;

};

///@endcond

// -------------------------------------------------------------------------


QgsSpatialIndex::QgsSpatialIndex( QgsSpatialIndex::Flags flags )
{
  d = new QgsSpatialIndexData( flags );
}

QgsSpatialIndex::QgsSpatialIndex( const QgsFeatureIterator &fi, QgsFeedback *feedback, QgsSpatialIndex::Flags flags )
{
  d = new QgsSpatialIndexData( fi, feedback, flags );
}

///@cond PRIVATE // else throws a doxygen warning?
QgsSpatialIndex::QgsSpatialIndex( const QgsFeatureIterator &fi, const std::function< bool( const QgsFeature & )> &callback, QgsSpatialIndex::Flags flags )
{
  d = new QgsSpatialIndexData( fi, nullptr, flags, &callback );
}
///@endcond

QgsSpatialIndex::QgsSpatialIndex( const QgsFeatureSource &source, QgsFeedback *feedback, QgsSpatialIndex::Flags flags )
{
  d = new QgsSpatialIndexData( source.getFeatures( QgsFeatureRequest().setNoAttributes() ), feedback, flags );
}

QgsSpatialIndex::QgsSpatialIndex( const QgsSpatialIndex &other ) //NOLINT
  : d( other.d )
{
}

QgsSpatialIndex:: ~QgsSpatialIndex() //NOLINT
{
}

QgsSpatialIndex &QgsSpatialIndex::operator=( const QgsSpatialIndex &other )
{
  if ( this != &other )
    d = other.d;
  return *this;
}

bool QgsSpatialIndex::featureInfo( const QgsFeature &f, SpatialIndex::Region &r, QgsFeatureId &id )
{
  QgsRectangle rect;
  if ( !featureInfo( f, rect, id ) )
    return false;

  r = QgsSpatialIndexUtils::rectangleToRegion( rect );
  return true;
}

bool QgsSpatialIndex::featureInfo( const QgsFeature &f, QgsRectangle &rect, QgsFeatureId &id )
{
  if ( !f.hasGeometry() )
    return false;

  id = f.id();
  rect = f.geometry().boundingBox();

  if ( !rect.isFinite() )
    return false;

  return true;
}

bool QgsSpatialIndex::addFeature( QgsFeature &feature, QgsFeatureSink::Flags )
{
  QgsRectangle rect;
  QgsFeatureId id;
  if ( !featureInfo( feature, rect, id ) )
    return false;

  if ( addFeature( id, rect ) )
  {
    if ( d->mFlags & QgsSpatialIndex::FlagStoreFeatureGeometries )
    {
      const QMutexLocker locker( &d->mMutex );
      d->mGeometries.insert( feature.id(), feature.geometry() );
    }
    return true;
  }
  return false;
}

bool QgsSpatialIndex::addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags )
{
  QgsFeatureList::iterator fIt = features.begin();
  bool result = true;
  for ( ; fIt != features.end(); ++fIt )
  {
    result = result && addFeature( *fIt, flags );
  }
  return result;
}

bool QgsSpatialIndex::insertFeature( const QgsFeature &f )
{
  QgsFeature feature( f );
  return addFeature( feature );
}

bool QgsSpatialIndex::insertFeature( QgsFeatureId id, const QgsRectangle &bounds )
{
  return addFeature( id, bounds );
}

bool QgsSpatialIndex::addFeature( QgsFeatureId id, const QgsRectangle &bounds )
{
  const SpatialIndex::Region r( QgsSpatialIndexUtils::rectangleToRegion( bounds ) );

  const QMutexLocker locker( &d->mMutex );

  // TODO: handle possible exceptions correctly
  try
  {
    d->mRTree->insertData( 0, nullptr, r, FID_TO_NUMBER( id ) );
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

bool QgsSpatialIndex::deleteFeature( const QgsFeature &f )
{
  SpatialIndex::Region r;
  QgsFeatureId id;
  if ( !featureInfo( f, r, id ) )
    return false;

  const QMutexLocker locker( &d->mMutex );
  // TODO: handle exceptions
  if ( d->mFlags & QgsSpatialIndex::FlagStoreFeatureGeometries )
    d->mGeometries.remove( f.id() );
  return d->mRTree->deleteData( r, FID_TO_NUMBER( id ) );
}

QList<QgsFeatureId> QgsSpatialIndex::intersects( const QgsRectangle &rect ) const
{
  QList<QgsFeatureId> list;
  QgisVisitor visitor( list );

  const SpatialIndex::Region r = QgsSpatialIndexUtils::rectangleToRegion( rect );

  const QMutexLocker locker( &d->mMutex );
  d->mRTree->intersectsWithQuery( r, visitor );

  return list;
}

QList<QgsFeatureId> QgsSpatialIndex::nearestNeighbor( const QgsPointXY &point, const int neighbors, const double maxDistance ) const
{
  QList<QgsFeatureId> list;
  QgisVisitor visitor( list );

  double pt[2] = { point.x(), point.y() };
  const Point p( pt, 2 );

  const QMutexLocker locker( &d->mMutex );
  QgsNearestNeighborComparator nnc( ( d->mFlags & QgsSpatialIndex::FlagStoreFeatureGeometries ) ? &d->mGeometries : nullptr,
                                    point, maxDistance );
  d->mRTree->nearestNeighborQuery( neighbors, p, visitor, nnc );

  if ( maxDistance > 0 )
  {
    // trim features outside of max distance
    list.erase( std::remove_if( list.begin(), list.end(),
                                [&nnc]( QgsFeatureId id )
    {
      return nnc.mFeaturesOutsideMaxDistance.contains( id );
    } ), list.end() );
  }

  return list;
}

QList<QgsFeatureId> QgsSpatialIndex::nearestNeighbor( const QgsGeometry &geometry, int neighbors, double maxDistance ) const
{
  QList<QgsFeatureId> list;
  QgisVisitor visitor( list );

  const SpatialIndex::Region r = QgsSpatialIndexUtils::rectangleToRegion( geometry.boundingBox() );

  const QMutexLocker locker( &d->mMutex );
  QgsNearestNeighborComparator nnc( ( d->mFlags & QgsSpatialIndex::FlagStoreFeatureGeometries ) ? &d->mGeometries : nullptr,
                                    geometry, maxDistance );
  d->mRTree->nearestNeighborQuery( neighbors, r, visitor, nnc );

  if ( maxDistance > 0 )
  {
    // trim features outside of max distance
    list.erase( std::remove_if( list.begin(), list.end(),
                                [&nnc]( QgsFeatureId id )
    {
      return nnc.mFeaturesOutsideMaxDistance.contains( id );
    } ), list.end() );
  }

  return list;
}

QgsGeometry QgsSpatialIndex::geometry( QgsFeatureId id ) const
{
  const QMutexLocker locker( &d->mMutex );
  return d->mGeometries.value( id );
}

QAtomicInt QgsSpatialIndex::refs() const
{
  return d->ref;
}
