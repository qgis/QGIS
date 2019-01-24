/***************************************************************************
    qgsmeshspatialindex.cpp
    -----------------------
    begin                : January 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshspatialindex.h"

#include "qgsmeshdataprovider.h"
#include "qgsrectangle.h"
#include "qgslogger.h"
#include "qgsfeedback.h"

#include <spatialindex/SpatialIndex.h>
#include <QMutex>
#include <QMutexLocker>
#include <memory>

using namespace SpatialIndex;

///@cond PRIVATE

static Region faceToRegion( const QgsMesh &mesh, int id )
{
  const QgsMeshFace face = mesh.face( id );
  const QVector<QgsMeshVertex> &vertices = mesh.vertices;
  Q_ASSERT( face.size() > 0 );
  double xMinimum = vertices[face[0]].x();
  double yMinimum = vertices[face[0]].y();
  double xMaximum = vertices[face[0]].x();
  double yMaximum = vertices[face[0]].y();

  for ( int i = 1; i < face.size(); ++i )
  {
    xMinimum = std::min( vertices[face[i]].x(), xMinimum );
    yMinimum = std::min( vertices[face[i]].y(), yMinimum );
    xMaximum = std::max( vertices[face[i]].x(), xMaximum );
    yMaximum = std::max( vertices[face[i]].y(), yMaximum );
  }

  double pt1[2] = { xMinimum, yMinimum };
  double pt2[2] = { xMaximum, yMaximum };
  return SpatialIndex::Region( pt1, pt2, 2 );
}

static Region rectToRegion( const QgsRectangle &rect )
{
  double pt1[2] = { rect.xMinimum(), rect.yMinimum() };
  double pt2[2] = { rect.xMaximum(), rect.yMaximum() };
  return SpatialIndex::Region( pt1, pt2, 2 );
}

/**
 * \ingroup core
 * \class QgisMeshVisitor
 * \brief Custom visitor that adds found faces to list.
 * \note not available in Python bindings
 */
class QgisMeshVisitor : public SpatialIndex::IVisitor
{
  public:
    explicit QgisMeshVisitor( QList<int> &list )
      : mList( list ) {}

    void visitNode( const INode &n ) override
    { Q_UNUSED( n ); }

    void visitData( const IData &d ) override
    {
      mList.append( static_cast<int>( d.getIdentifier() ) );
    }

    void visitData( std::vector<const IData *> &v ) override
    { Q_UNUSED( v ); }

  private:
    QList<int> &mList;
};

/**
 * \ingroup core
 * \class QgsMeshSpatialIndexCopyVisitor
 * \note not available in Python bindings
 */
class QgsMeshSpatialIndexCopyVisitor : public SpatialIndex::IVisitor
{
  public:
    explicit QgsMeshSpatialIndexCopyVisitor( SpatialIndex::ISpatialIndex *newIndex )
      : mNewIndex( newIndex ) {}

    void visitNode( const INode &n ) override
    { Q_UNUSED( n ); }

    void visitData( const IData &d ) override
    {
      SpatialIndex::IShape *shape = nullptr;
      d.getShape( &shape );
      mNewIndex->insertData( 0, nullptr, *shape, d.getIdentifier() );
      delete shape;
    }

    void visitData( std::vector<const IData *> &v ) override
    { Q_UNUSED( v ); }

  private:
    SpatialIndex::ISpatialIndex *mNewIndex = nullptr;
};


/**
 * \ingroup core
 * \class QgsMeshFaceIteratorDataStream
 * \brief Utility class for bulk loading of R-trees. Not a part of public API.
 * \note not available in Python bindings
*/
class QgsMeshFaceIteratorDataStream : public IDataStream
{
  public:
    //! constructor - needs to load all data to a vector for later access when bulk loading
    explicit QgsMeshFaceIteratorDataStream( const QgsMesh &triangularMesh, QgsFeedback *feedback = nullptr )
      : mMesh( triangularMesh )
      , mFeedback( feedback )
    {
      readNextEntry();
    }

    ~QgsMeshFaceIteratorDataStream() override
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
    bool hasNext() override
    {
      return nullptr != mNextData;
    }

    //! returns the total number of entries available in the stream.
    uint32_t size() override
    {
      return static_cast<uint32_t>( mMesh.faceCount() );
    }

    //! sets the stream pointer to the first entry, if possible.
    void rewind() override
    {
      mIterator = 0;
    }

  protected:
    void readNextEntry()
    {
      SpatialIndex::Region r;
      const int faceCount = mMesh.faceCount();
      if ( mIterator < faceCount )
      {
        r = faceToRegion( mMesh, mIterator );
        mNextData = new RTree::Data(
          0,
          nullptr,
          r,
          mIterator );
        ++mIterator;
      }
    }

  private:
    int mIterator = 0;
    const QgsMesh &mMesh;
    RTree::Data *mNextData = nullptr;
    QgsFeedback *mFeedback = nullptr;
};


/**
 * \ingroup core
 * \class QgsSpatialIndexData
 * \brief Data of spatial index that may be implicitly shared
 * \note not available in Python bindings
*/
class QgsMeshSpatialIndexData : public QSharedData
{
  public:
    QgsMeshSpatialIndexData()
    {
      initTree();
    }

    /**
     * Constructor for QgsSpatialIndexData which bulk loads faces from the specified mesh
     * \a fi.
     *
     * The optional \a feedback object can be used to allow cancelation of bulk face loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.
     */
    explicit QgsMeshSpatialIndexData( const QgsMesh &fi, QgsFeedback *feedback = nullptr )
    {
      QgsMeshFaceIteratorDataStream fids( fi, feedback );
      initTree( &fids );
    }

    QgsMeshSpatialIndexData( const QgsMeshSpatialIndexData &other )
      : QSharedData( other )
    {
      QMutexLocker locker( &other.mMutex );

      initTree();

      // copy R-tree data one by one (is there a faster way??)
      double low[]  = { std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest() };
      double high[] = { std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
      SpatialIndex::Region query( low, high, 2 );
      QgsMeshSpatialIndexCopyVisitor visitor( mRTree.get() );
      other.mRTree->intersectsWithQuery( query, visitor );
    }

    ~QgsMeshSpatialIndexData() = default;

    QgsMeshSpatialIndexData &operator=( const QgsMeshSpatialIndexData &rh ) = delete;

    void initTree( IDataStream *inputStream = nullptr )
    {
      // for now only memory manager
      mStorage.reset( StorageManager::createNewMemoryStorageManager() );

      // R-Tree parameters
      double fillFactor = 0.7;
      unsigned int indexCapacity = 10;
      unsigned int leafCapacity = 10;
      unsigned int dimension = 2;
      RTree::RTreeVariant variant = RTree::RV_RSTAR;

      // create R-tree
      SpatialIndex::id_type indexId;

      if ( inputStream && inputStream->hasNext() )
        mRTree.reset(
          RTree::createAndBulkLoadNewRTree(
            RTree::BLM_STR,
            *inputStream,
            *mStorage, fillFactor,
            indexCapacity,
            leafCapacity,
            dimension,
            variant,
            indexId )
        );
      else
        mRTree.reset(
          RTree::createNewRTree(
            *mStorage,
            fillFactor,
            indexCapacity,
            leafCapacity,
            dimension,
            variant,
            indexId )
        );
    }

    //! Storage manager
    std::unique_ptr<SpatialIndex::IStorageManager> mStorage;

    //! R-tree containing spatial index
    std::unique_ptr<SpatialIndex::ISpatialIndex> mRTree;

    mutable QMutex mMutex;

};

///@endcond

QgsMeshSpatialIndex::QgsMeshSpatialIndex()
{
  d = new QgsMeshSpatialIndexData;
}

QgsMeshSpatialIndex::QgsMeshSpatialIndex( const QgsMesh &triangularMesh, QgsFeedback *feedback )
{
  d = new QgsMeshSpatialIndexData( triangularMesh, feedback );
}

QgsMeshSpatialIndex::QgsMeshSpatialIndex( const QgsMeshSpatialIndex &other ) //NOLINT
  : d( other.d )
{
}

QgsMeshSpatialIndex:: ~QgsMeshSpatialIndex() = default; //NOLINT

QgsMeshSpatialIndex &QgsMeshSpatialIndex::operator=( const QgsMeshSpatialIndex &other )
{
  if ( this != &other )
    d = other.d;
  return *this;
}

QList<int> QgsMeshSpatialIndex::intersects( const QgsRectangle &rect ) const
{
  QList<int> list;
  QgisMeshVisitor visitor( list );

  SpatialIndex::Region r = rectToRegion( rect );

  QMutexLocker locker( &d->mMutex );
  d->mRTree->intersectsWithQuery( r, visitor );

  return list;
}

QList<int> QgsMeshSpatialIndex::nearestNeighbor( const QgsPointXY &point, int neighbors ) const
{
  QList<int> list;
  QgisMeshVisitor visitor( list );

  double pt[2] = { point.x(), point.y() };
  Point p( pt, 2 );

  QMutexLocker locker( &d->mMutex );
  d->mRTree->nearestNeighborQuery( static_cast<uint32_t>( neighbors ), p, visitor );

  return list;
}
