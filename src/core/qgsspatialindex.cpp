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

#include "SpatialIndex.h"

using namespace SpatialIndex;



/** \ingroup core
 * \class QgisVisitor
 * \brief Custom visitor that adds found features to list.
 * \note not available in Python bindings
 */
class QgisVisitor : public SpatialIndex::IVisitor
{
  public:
    explicit QgisVisitor( QList<QgsFeatureId> & list )
        : mList( list ) {}

    void visitNode( const INode& n ) override
      { Q_UNUSED( n ); }

    void visitData( const IData& d ) override
    {
      mList.append( d.getIdentifier() );
    }

    void visitData( std::vector<const IData*>& v ) override
      { Q_UNUSED( v ); }

  private:
    QList<QgsFeatureId>& mList;
};

/** \ingroup core
 * \class QgsSpatialIndexCopyVisitor
 * \note not available in Python bindings
 */
class QgsSpatialIndexCopyVisitor : public SpatialIndex::IVisitor
{
  public:
    explicit QgsSpatialIndexCopyVisitor( SpatialIndex::ISpatialIndex* newIndex )
        : mNewIndex( newIndex ) {}

    void visitNode( const INode& n ) override
      { Q_UNUSED( n ); }

    void visitData( const IData& d ) override
    {
      SpatialIndex::IShape* shape;
      d.getShape( &shape );
      mNewIndex->insertData( 0, nullptr, *shape, d.getIdentifier() );
      delete shape;
    }

    void visitData( std::vector<const IData*>& v ) override
      { Q_UNUSED( v ); }

  private:
    SpatialIndex::ISpatialIndex* mNewIndex;
};


/** \ingroup core
 * \class QgsFeatureIteratorDataStream
 * \brief Utility class for bulk loading of R-trees. Not a part of public API.
 * \note not available in Python bindings
*/
class QgsFeatureIteratorDataStream : public IDataStream
{
  public:
    //! constructor - needs to load all data to a vector for later access when bulk loading
    explicit QgsFeatureIteratorDataStream( const QgsFeatureIterator& fi )
        : mFi( fi )
        , mNextData( nullptr )
    {
      readNextEntry();
    }

    ~QgsFeatureIteratorDataStream()
    {
      delete mNextData;
    }

    //! returns a pointer to the next entry in the stream or 0 at the end of the stream.
    virtual IData* getNext() override
    {
      RTree::Data* ret = mNextData;
      mNextData = nullptr;
      readNextEntry();
      return ret;
    }

    //! returns true if there are more items in the stream.
    virtual bool hasNext() override { return nullptr != mNextData; }

    //! returns the total number of entries available in the stream.
    virtual uint32_t size() override { Q_ASSERT( 0 && "not available" ); return 0; }

    //! sets the stream pointer to the first entry, if possible.
    virtual void rewind() override { Q_ASSERT( 0 && "not available" ); }

  protected:
    void readNextEntry()
    {
      QgsFeature f;
      SpatialIndex::Region r;
      QgsFeatureId id;
      while ( mFi.nextFeature( f ) )
      {
        if ( QgsSpatialIndex::featureInfo( f, r, id ) )
        {
          mNextData = new RTree::Data( 0, nullptr, r, id );
          return;
        }
      }
    }

  private:
    QgsFeatureIterator mFi;
    RTree::Data* mNextData;
};


/** \ingroup core
 *  \class QgsSpatialIndexData
 * \brief Data of spatial index that may be implicitly shared
 * \note not available in Python bindings
*/
class QgsSpatialIndexData : public QSharedData
{
  public:
    QgsSpatialIndexData()
    {
      initTree();
    }

    explicit QgsSpatialIndexData( const QgsFeatureIterator& fi )
    {
      QgsFeatureIteratorDataStream fids( fi );
      initTree( &fids );
    }

    QgsSpatialIndexData( const QgsSpatialIndexData& other )
        : QSharedData( other )
    {
      initTree();

      // copy R-tree data one by one (is there a faster way??)
      double low[]  = { DBL_MIN, DBL_MIN };
      double high[] = { DBL_MAX, DBL_MAX };
      SpatialIndex::Region query( low, high, 2 );
      QgsSpatialIndexCopyVisitor visitor( mRTree );
      other.mRTree->intersectsWithQuery( query, visitor );
    }

    ~QgsSpatialIndexData()
    {
      delete mRTree;
      delete mStorage;
    }

    void initTree( IDataStream* inputStream = nullptr )
    {
      // for now only memory manager
      mStorage = StorageManager::createNewMemoryStorageManager();

      // R-Tree parameters
      double fillFactor = 0.7;
      unsigned long indexCapacity = 10;
      unsigned long leafCapacity = 10;
      unsigned long dimension = 2;
      RTree::RTreeVariant variant = RTree::RV_RSTAR;

      // create R-tree
      SpatialIndex::id_type indexId;

      if ( inputStream )
        mRTree = RTree::createAndBulkLoadNewRTree( RTree::BLM_STR, *inputStream, *mStorage, fillFactor, indexCapacity,
                 leafCapacity, dimension, variant, indexId );
      else
        mRTree = RTree::createNewRTree( *mStorage, fillFactor, indexCapacity,
                                        leafCapacity, dimension, variant, indexId );
    }

    /** Storage manager */
    SpatialIndex::IStorageManager* mStorage;

    /** R-tree containing spatial index */
    SpatialIndex::ISpatialIndex* mRTree;

  private:

    QgsSpatialIndexData& operator=( const QgsSpatialIndexData& rh );
};

// -------------------------------------------------------------------------


QgsSpatialIndex::QgsSpatialIndex()
{
  d = new QgsSpatialIndexData;
}

QgsSpatialIndex::QgsSpatialIndex( const QgsFeatureIterator& fi )
{
  d = new QgsSpatialIndexData( fi );
}

QgsSpatialIndex::QgsSpatialIndex( const QgsSpatialIndex& other )
    : d( other.d )
{
}

QgsSpatialIndex:: ~QgsSpatialIndex()
{
}

QgsSpatialIndex& QgsSpatialIndex::operator=( const QgsSpatialIndex & other )
{
  if ( this != &other )
    d = other.d;
  return *this;
}

SpatialIndex::Region QgsSpatialIndex::rectToRegion( const QgsRectangle& rect )
{
  double pt1[2] = { rect.xMinimum(), rect.yMinimum() },
                  pt2[2] = { rect.xMaximum(), rect.yMaximum() };
  return SpatialIndex::Region( pt1, pt2, 2 );
}

bool QgsSpatialIndex::featureInfo( const QgsFeature& f, SpatialIndex::Region& r, QgsFeatureId &id )
{
  if ( !f.constGeometry() )
    return false;

  QgsGeometry g( *f.constGeometry() );

  id = f.id();
  r = rectToRegion( g.boundingBox() );
  return true;
}


bool QgsSpatialIndex::insertFeature( const QgsFeature& f )
{
  SpatialIndex::Region r;
  QgsFeatureId id;
  if ( !featureInfo( f, r, id ) )
    return false;

  // TODO: handle possible exceptions correctly
  try
  {
    d->mRTree->insertData( 0, nullptr, r, FID_TO_NUMBER( id ) );
    return true;
  }
  catch ( Tools::Exception &e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( QString( "Tools::Exception caught: " ).arg( e.what().c_str() ) );
  }
  catch ( const std::exception &e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( QString( "std::exception caught: " ).arg( e.what() ) );
  }
  catch ( ... )
  {
    QgsDebugMsg( "unknown spatial index exception caught" );
  }

  return false;
}

bool QgsSpatialIndex::deleteFeature( const QgsFeature& f )
{
  SpatialIndex::Region r;
  QgsFeatureId id;
  if ( !featureInfo( f, r, id ) )
    return false;

  // TODO: handle exceptions
  return d->mRTree->deleteData( r, FID_TO_NUMBER( id ) );
}

QList<QgsFeatureId> QgsSpatialIndex::intersects( const QgsRectangle& rect ) const
{
  QList<QgsFeatureId> list;
  QgisVisitor visitor( list );

  SpatialIndex::Region r = rectToRegion( rect );

  d->mRTree->intersectsWithQuery( r, visitor );

  return list;
}

QList<QgsFeatureId> QgsSpatialIndex::nearestNeighbor( const QgsPoint& point, int neighbors ) const
{
  QList<QgsFeatureId> list;
  QgisVisitor visitor( list );

  double pt[2] = { point.x(), point.y() };
  Point p( pt, 2 );

  d->mRTree->nearestNeighborQuery( neighbors, p, visitor );

  return list;
}

QAtomicInt QgsSpatialIndex::refs() const
{
  return d->ref;
}
