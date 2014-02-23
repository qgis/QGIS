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
#include "qgsrectangle.h"
#include "qgslogger.h"

#include "SpatialIndex.h"

using namespace SpatialIndex;



// custom visitor that adds found features to list
class QgisVisitor : public SpatialIndex::IVisitor
{
  public:
    QgisVisitor( QList<QgsFeatureId> & list )
        : mList( list ) {}

    void visitNode( const INode& n )
    { Q_UNUSED( n ); }

    void visitData( const IData& d )
    {
      mList.append( d.getIdentifier() );
    }

    void visitData( std::vector<const IData*>& v )
    { Q_UNUSED( v ); }

  private:
    QList<QgsFeatureId>& mList;
};

class QgsSpatialIndexCopyVisitor : public SpatialIndex::IVisitor
{
  public:
    QgsSpatialIndexCopyVisitor( SpatialIndex::ISpatialIndex* newIndex )
        : mNewIndex( newIndex ) {}

    void visitNode( const INode& n )
    { Q_UNUSED( n ); }

    void visitData( const IData& d )
    {
      SpatialIndex::IShape* shape;
      d.getShape( &shape );
      mNewIndex->insertData( 0, 0, *shape, d.getIdentifier() );
      delete shape;
    }

    void visitData( std::vector<const IData*>& v )
    { Q_UNUSED( v ); }

  private:
    SpatialIndex::ISpatialIndex* mNewIndex;
};


/** Data of spatial index that may be implicitly shared */
class QgsSpatialIndexData : public QSharedData
{
  public:
    QgsSpatialIndexData()
    {
      initTree();
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

    void initTree()
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
      mRTree = RTree::createNewRTree( *mStorage, fillFactor, indexCapacity,
                                      leafCapacity, dimension, variant, indexId );
    }

    /** storage manager */
    SpatialIndex::IStorageManager* mStorage;

    /** R-tree containing spatial index */
    SpatialIndex::ISpatialIndex* mRTree;
};

// -------------------------------------------------------------------------


QgsSpatialIndex::QgsSpatialIndex()
{
  d = new QgsSpatialIndexData;
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

Region QgsSpatialIndex::rectToRegion( QgsRectangle rect )
{
  double pt1[2], pt2[2];
  pt1[0] = rect.xMinimum();
  pt1[1] = rect.yMinimum();
  pt2[0] = rect.xMaximum();
  pt2[1] = rect.yMaximum();
  return Region( pt1, pt2, 2 );
}

bool QgsSpatialIndex::featureInfo( const QgsFeature& f, SpatialIndex::Region& r, QgsFeatureId &id )
{
  QgsGeometry *g = f.geometry();
  if ( !g )
    return false;

  id = f.id();
  r = rectToRegion( g->boundingBox() );
  return true;
}


bool QgsSpatialIndex::insertFeature( const QgsFeature& f )
{
  Region r;
  QgsFeatureId id;
  if ( !featureInfo( f, r, id ) )
    return false;

  // TODO: handle possible exceptions correctly
  try
  {
    d->mRTree->insertData( 0, 0, r, FID_TO_NUMBER( id ) );
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
  Region r;
  QgsFeatureId id;
  if ( !featureInfo( f, r, id ) )
    return false;

  // TODO: handle exceptions
  return d->mRTree->deleteData( r, FID_TO_NUMBER( id ) );
}

QList<QgsFeatureId> QgsSpatialIndex::intersects( QgsRectangle rect ) const
{
  QList<QgsFeatureId> list;
  QgisVisitor visitor( list );

  Region r = rectToRegion( rect );

  d->mRTree->intersectsWithQuery( r, visitor );

  return list;
}

QList<QgsFeatureId> QgsSpatialIndex::nearestNeighbor( QgsPoint point, int neighbors ) const
{
  QList<QgsFeatureId> list;
  QgisVisitor visitor( list );

  double pt[2];
  pt[0] = point.x();
  pt[1] = point.y();
  Point p( pt, 2 );

  d->mRTree->nearestNeighborQuery( neighbors, p, visitor );

  return list;
}

int QgsSpatialIndex::refs() const
{
  return d->ref;
}
