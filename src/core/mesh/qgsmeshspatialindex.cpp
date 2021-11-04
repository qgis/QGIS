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
#include "qgsrectangle.h"
#include "qgslogger.h"
#include "qgsfeedback.h"

#include <spatialindex/SpatialIndex.h>
#include <QMutex>
#include <QMutexLocker>
#include <memory>

using namespace SpatialIndex;

///@cond PRIVATE

static Region faceToRegion( const QgsMesh &mesh, int id, bool &ok )
{
  const QgsMeshFace face = mesh.face( id );

  if ( face.isEmpty() )
  {
    ok = false;
    return Region();
  }

  const QVector<QgsMeshVertex> &vertices = mesh.vertices;

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

  ok = true;
  return SpatialIndex::Region( pt1, pt2, 2 );
}

static Region edgeToRegion( const QgsMesh &mesh, int id, bool &ok )
{
  const QgsMeshEdge edge = mesh.edge( id );
  const QgsMeshVertex firstVertex = mesh.vertices[edge.first];
  const QgsMeshVertex secondVertex = mesh.vertices[edge.second];
  const double xMinimum = std::min( firstVertex.x(), secondVertex.x() );
  const double yMinimum = std::min( firstVertex.y(), secondVertex.y() );
  const double xMaximum = std::max( firstVertex.x(), secondVertex.x() );
  const double yMaximum = std::max( firstVertex.y(), secondVertex.y() );
  double pt1[2] = { xMinimum, yMinimum };
  double pt2[2] = { xMaximum, yMaximum };
  ok = true;
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
 * \brief Custom visitor that adds found faces or edges to list.
 * \note not available in Python bindings
 */
class QgisMeshVisitor : public SpatialIndex::IVisitor
{
  public:
    explicit QgisMeshVisitor( QList<int> &list )
      : mList( list ) {}

    void visitNode( const INode &n ) override
    { Q_UNUSED( n ) }

    void visitData( const IData &d ) override
    {
      mList.append( static_cast<int>( d.getIdentifier() ) );
    }

    void visitData( std::vector<const IData *> &v ) override
    { Q_UNUSED( v ) }

  private:
    QList<int> &mList;
};

/**
 * \ingroup core
 * \class QgsMeshSpatialIndexCopyVisitor
 * \brief A copy visitor for populating a spatial index.
 * \note not available in Python bindings
 */
class QgsMeshSpatialIndexCopyVisitor : public SpatialIndex::IVisitor
{
  public:
    explicit QgsMeshSpatialIndexCopyVisitor( SpatialIndex::ISpatialIndex *newIndex )
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


/**
 * \ingroup core
 * \class QgsMeshFaceIteratorDataStream
 * \brief Utility class for bulk loading of R-trees. Not a part of public API.
 * \note not available in Python bindings
*/
class QgsMeshIteratorDataStream : public IDataStream
{
  public:
    //! constructor - needs to load all data to a vector for later access when bulk loading
    explicit QgsMeshIteratorDataStream( const QgsMesh &mesh,
                                        int featuresCount,
                                        std::function<Region( const QgsMesh &mesh, int id, bool &ok )> featureToRegionFunction,
                                        QgsFeedback *feedback = nullptr )
      : mMesh( mesh )
      , mFeaturesCount( featuresCount )
      , mFeatureToRegionFunction( featureToRegionFunction )
      , mFeedback( feedback )
    {
      readNextEntry();
    }

    ~QgsMeshIteratorDataStream() override
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
      return static_cast<uint32_t>( mFeaturesCount );
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
      while ( mIterator < mFeaturesCount )
      {
        bool ok = false;
        r = mFeatureToRegionFunction( mMesh, mIterator, ok );
        if ( ok )
        {
          mNextData = new RTree::Data( 0, nullptr, r, mIterator );
          ++mIterator;
          return;
        }
        else
        {
          ++mIterator;
          continue;
        }
      }
    }

  private:
    int mIterator = 0;
    const QgsMesh &mMesh;
    int mFeaturesCount = 0;
    std::function<Region( const QgsMesh &mesh, int id, bool &ok )> mFeatureToRegionFunction;
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
     * The optional \a feedback object can be used to allow cancellation of bulk face loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.
     */
    explicit QgsMeshSpatialIndexData( const QgsMesh &fi, QgsFeedback *feedback, QgsMesh::ElementType elementType )
    {
      switch ( elementType )
      {
        case QgsMesh::ElementType::Edge:
        {
          QgsMeshIteratorDataStream fids( fi, fi.edgeCount(), edgeToRegion, feedback );
          initTree( &fids );
        }
        break;
        case QgsMesh::ElementType::Face:
        {
          QgsMeshIteratorDataStream fids( fi, fi.faceCount(), faceToRegion, feedback );
          initTree( &fids );
        }
        break;
        default:
          // vertices are not supported
          Q_ASSERT( false );
          break;
      }
    }

    QgsMeshSpatialIndexData( const QgsMeshSpatialIndexData &other )
      : QSharedData( other )
    {
      const QMutexLocker locker( &other.mMutex );

      initTree();

      // copy R-tree data one by one (is there a faster way??)
      double low[]  = { std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest() };
      double high[] = { std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
      const SpatialIndex::Region query( low, high, 2 );
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
      const double fillFactor = 0.7;
      const unsigned int indexCapacity = 10;
      const unsigned int leafCapacity = 10;
      const unsigned int dimension = 2;
      const RTree::RTreeVariant variant = RTree::RV_RSTAR;

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

QgsMeshSpatialIndex::QgsMeshSpatialIndex( const QgsMesh &mesh, QgsFeedback *feedback, QgsMesh::ElementType elementType )
  : mElementType( elementType )
{
  d = new QgsMeshSpatialIndexData( mesh, feedback, elementType );
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

  const SpatialIndex::Region r = rectToRegion( rect );

  const QMutexLocker locker( &d->mMutex );
  d->mRTree->intersectsWithQuery( r, visitor );

  return list;
}

QList<int> QgsMeshSpatialIndex::nearestNeighbor( const QgsPointXY &point, int neighbors ) const
{
  QList<int> list;
  QgisMeshVisitor visitor( list );

  double pt[2] = { point.x(), point.y() };
  const Point p( pt, 2 );

  const QMutexLocker locker( &d->mMutex );
  d->mRTree->nearestNeighborQuery( static_cast<uint32_t>( neighbors ), p, visitor );

  return list;
}

QgsMesh::ElementType QgsMeshSpatialIndex::elementType() const
{
  return mElementType;
}

void QgsMeshSpatialIndex::addFace( int faceIndex, const QgsMesh &mesh )
{
  if ( mesh.face( faceIndex ).isEmpty() )
    return;

  bool ok = false;
  const SpatialIndex::Region r( faceToRegion( mesh, faceIndex, ok ) );
  if ( !ok )
    return;

  const QMutexLocker locker( &d.constData()->mMutex );

  try
  {
    d.constData()->mRTree->insertData( 0, nullptr, r, faceIndex );
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
}

void QgsMeshSpatialIndex::removeFace( int faceIndex, const QgsMesh &mesh )
{
  if ( mesh.face( faceIndex ).isEmpty() )
    return;
  const QMutexLocker locker( &d.constData()->mMutex );
  bool ok = false;
  d.constData()->mRTree->deleteData( faceToRegion( mesh, faceIndex, ok ), faceIndex );
}
