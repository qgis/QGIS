/***************************************************************************
  qgspointlocator.cpp
  --------------------------------------
  Date                 : November 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointlocator.h"

#include "qgsgeometry.h"
#include "qgsvectorlayer.h"

#include <spatialindex/SpatialIndex.h>

#include <QLinkedListIterator>

using namespace SpatialIndex;



static SpatialIndex::Point point2point( const QgsPoint& point )
{
  double plow[2];
  plow[0] = point.x();
  plow[1] = point.y();
  return Point( plow, 2 );
}


static SpatialIndex::Region rect2region( const QgsRectangle& rect )
{
  double pLow[2], pHigh[2];
  pLow[0] = rect.xMinimum();
  pLow[1] = rect.yMinimum();
  pHigh[0] = rect.xMaximum();
  pHigh[1] = rect.yMaximum();
  return SpatialIndex::Region( pLow, pHigh, 2 );
}


// Ahh.... another magic number. Taken from QgsVectorLayer::snapToGeometry() call to closestSegmentWithContext().
// The default epsilon used for sqrDistToSegment (1e-8) is too high when working with lat/lon coordinates
// I still do not fully understand why the sqrDistToSegment() code uses epsilon and if the square distance
// is lower than epsilon it will have a special logic...
static const double POINT_LOC_EPSILON = 1e-12;

////////////////////////////////////////////////////////////////////////////


/** Helper class for bulk loading of R-trees. */
class QgsPointLocator_Stream : public IDataStream
{
  public:
    QgsPointLocator_Stream( const QLinkedList<RTree::Data*>& dataList ) : mDataList( dataList ), mIt( mDataList ) { }
    ~QgsPointLocator_Stream() { }

    virtual IData* getNext() override { return mIt.next(); }
    virtual bool hasNext() override { return mIt.hasNext(); }

    virtual uint32_t size() override { Q_ASSERT( 0 && "not available" ); return 0; }
    virtual void rewind() override { Q_ASSERT( 0 && "not available" ); }

  private:
    QLinkedList<RTree::Data*> mDataList;
    QLinkedListIterator<RTree::Data*> mIt;
};


////////////////////////////////////////////////////////////////////////////


/** Helper class used when traversing the index looking for vertices - builds a list of matches. */
class QgsPointLocator_VisitorNearestVertex : public IVisitor
{
  public:
    QgsPointLocator_VisitorNearestVertex( QgsPointLocator* pl, QgsPointLocator::Match& m, const QgsPoint& srcPoint, QgsPointLocator::MatchFilter* filter = 0 )
        : mLocator( pl ), mBest( m ), mSrcPoint( srcPoint ), mFilter( filter ) {}

    void visitNode( const INode& n ) override { Q_UNUSED( n ); }
    void visitData( std::vector<const IData*>& v ) override { Q_UNUSED( v ); }

    void visitData( const IData& d ) override
    {
      QgsFeatureId id = d.getIdentifier();
      QgsGeometry* geom = mLocator->mGeoms.value( id );
      int vertexIndex, beforeVertex, afterVertex;
      double sqrDist;
      QgsPoint pt = geom->closestVertex( mSrcPoint, vertexIndex, beforeVertex, afterVertex, sqrDist );

      QgsPointLocator::Match m( QgsPointLocator::Vertex, mLocator->mLayer, id, sqrt( sqrDist ), pt, vertexIndex );
      // in range queries the filter may reject some matches
      if ( mFilter && !mFilter->acceptMatch( m ) )
        return;

      if ( !mBest.isValid() || m.distance() < mBest.distance() )
        mBest = m;
    }

  private:
    QgsPointLocator* mLocator;
    QgsPointLocator::Match& mBest;
    QgsPoint mSrcPoint;
    QgsPointLocator::MatchFilter* mFilter;
};


////////////////////////////////////////////////////////////////////////////


/** Helper class used when traversing the index looking for edges - builds a list of matches. */
class QgsPointLocator_VisitorNearestEdge : public IVisitor
{
  public:
    QgsPointLocator_VisitorNearestEdge( QgsPointLocator* pl, QgsPointLocator::Match& m, const QgsPoint& srcPoint, QgsPointLocator::MatchFilter* filter = 0 )
        : mLocator( pl ), mBest( m ), mSrcPoint( srcPoint ), mFilter( filter ) {}

    void visitNode( const INode& n ) override { Q_UNUSED( n ); }
    void visitData( std::vector<const IData*>& v ) override { Q_UNUSED( v ); }

    void visitData( const IData& d ) override
    {
      QgsFeatureId id = d.getIdentifier();
      QgsGeometry* geom = mLocator->mGeoms.value( id );
      QgsPoint pt;
      int afterVertex;
      double sqrDist = geom->closestSegmentWithContext( mSrcPoint, pt, afterVertex, 0, POINT_LOC_EPSILON );
      if ( sqrDist < 0 )
        return;

      QgsPoint edgePoints[2];
      edgePoints[0] = geom->vertexAt( afterVertex - 1 );
      edgePoints[1] = geom->vertexAt( afterVertex );
      QgsPointLocator::Match m( QgsPointLocator::Edge, mLocator->mLayer, id, sqrt( sqrDist ), pt, afterVertex - 1, edgePoints );
      // in range queries the filter may reject some matches
      if ( mFilter && !mFilter->acceptMatch( m ) )
        return;

      if ( !mBest.isValid() || m.distance() < mBest.distance() )
        mBest = m;
    }

  private:
    QgsPointLocator* mLocator;
    QgsPointLocator::Match& mBest;
    QgsPoint mSrcPoint;
    QgsPointLocator::MatchFilter* mFilter;
};


////////////////////////////////////////////////////////////////////////////


/** Helper class used when traversing the index with areas - builds a list of matches. */
class QgsPointLocator_VisitorArea : public IVisitor
{
  public:
    //! constructor
    QgsPointLocator_VisitorArea( QgsPointLocator* pl, const QgsPoint& origPt, QgsPointLocator::MatchList& list )
        : mLocator( pl ), mList( list ), mGeomPt( QgsGeometry::fromPoint( origPt ) ) {}

    ~QgsPointLocator_VisitorArea() { delete mGeomPt; }

    void visitNode( const INode& n ) override { Q_UNUSED( n ); }
    void visitData( std::vector<const IData*>& v ) override { Q_UNUSED( v ); }

    void visitData( const IData& d ) override
    {
      QgsFeatureId id = d.getIdentifier();
      QgsGeometry* g = mLocator->mGeoms.value( id );
      if ( g->intersects( mGeomPt ) )
        mList << QgsPointLocator::Match( QgsPointLocator::Area, mLocator->mLayer, id, 0, QgsPoint() );
    }
  private:
    QgsPointLocator* mLocator;
    QgsPointLocator::MatchList& mList;
    QgsGeometry* mGeomPt;
};


////////////////////////////////////////////////////////////////////////////

// code adapted from
// http://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
struct _CohenSutherland
{
  _CohenSutherland( const QgsRectangle& rect ) : mRect( rect ) {}

  typedef int OutCode;

  static const int INSIDE = 0; // 0000
  static const int LEFT = 1;   // 0001
  static const int RIGHT = 2;  // 0010
  static const int BOTTOM = 4; // 0100
  static const int TOP = 8;    // 1000

  QgsRectangle mRect;

  OutCode computeOutCode( double x, double y )
  {
    OutCode code = INSIDE;  // initialised as being inside of clip window
    if ( x < mRect.xMinimum() )         // to the left of clip window
      code |= LEFT;
    else if ( x > mRect.xMaximum() )    // to the right of clip window
      code |= RIGHT;
    if ( y < mRect.yMinimum() )         // below the clip window
      code |= BOTTOM;
    else if ( y > mRect.yMaximum() )    // above the clip window
      code |= TOP;
    return code;
  }

  bool isSegmentInRect( double x0, double y0, double x1, double y1 )
  {
    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    OutCode outcode0 = computeOutCode( x0, y0 );
    OutCode outcode1 = computeOutCode( x1, y1 );
    bool accept = false;

    while ( true )
    {
      if ( !( outcode0 | outcode1 ) )
      {
        // Bitwise OR is 0. Trivially accept and get out of loop
        accept = true;
        break;
      }
      else if ( outcode0 & outcode1 )
      {
        // Bitwise AND is not 0. Trivially reject and get out of loop
        break;
      }
      else
      {
        // failed both tests, so calculate the line segment to clip
        // from an outside point to an intersection with clip edge
        double x, y;

        // At least one endpoint is outside the clip rectangle; pick it.
        OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

        // Now find the intersection point;
        // use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
        if ( outcodeOut & TOP )
        { // point is above the clip rectangle
          x = x0 + ( x1 - x0 ) * ( mRect.yMaximum() - y0 ) / ( y1 - y0 );
          y = mRect.yMaximum();
        }
        else if ( outcodeOut & BOTTOM )
        { // point is below the clip rectangle
          x = x0 + ( x1 - x0 ) * ( mRect.yMinimum() - y0 ) / ( y1 - y0 );
          y = mRect.yMinimum();
        }
        else if ( outcodeOut & RIGHT )
        {  // point is to the right of clip rectangle
          y = y0 + ( y1 - y0 ) * ( mRect.xMaximum() - x0 ) / ( x1 - x0 );
          x = mRect.xMaximum();
        }
        else if ( outcodeOut & LEFT )
        {   // point is to the left of clip rectangle
          y = y0 + ( y1 - y0 ) * ( mRect.xMinimum() - x0 ) / ( x1 - x0 );
          x = mRect.xMinimum();
        }

        // Now we move outside point to intersection point to clip
        // and get ready for next pass.
        if ( outcodeOut == outcode0 )
        {
          x0 = x;
          y0 = y;
          outcode0 = computeOutCode( x0, y0 );
        }
        else
        {
          x1 = x;
          y1 = y;
          outcode1 = computeOutCode( x1, y1 );
        }
      }
    }
    return accept;
  }
};


static QgsPointLocator::MatchList _geometrySegmentsInRect( QgsGeometry* geom, const QgsRectangle& rect, QgsVectorLayer* vl, QgsFeatureId fid )
{
  // this code is stupidly based on QgsGeometry::closestSegmentWithContext
  // we need iterator for segments...

  QgsPointLocator::MatchList lst;
  unsigned char* wkb = const_cast<unsigned char*>( geom->asWkb() ); // we're not changing wkb, just need non-const for QgsWkbPtr
  if ( !wkb )
    return lst;

  _CohenSutherland cs( rect );

  QgsWkbPtr wkbPtr( wkb + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  bool hasZValue = false;
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
    {
      // Points have no lines
      return lst;
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      int nPoints;
      wkbPtr >> nPoints;

      double prevx = 0.0, prevy = 0.0;
      for ( int index = 0; index < nPoints; ++index )
      {
        double thisx, thisy;
        wkbPtr >> thisx >> thisy;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        if ( index > 0 )
        {
          if ( cs.isSegmentInRect( prevx, prevy, thisx, thisy ) )
          {
            QgsPoint edgePoints[2];
            edgePoints[0].set( prevx, prevy );
            edgePoints[1].set( thisx, thisy );
            lst << QgsPointLocator::Match( QgsPointLocator::Edge, vl, fid, 0, QgsPoint(), index - 1, edgePoints );
          }
        }

        prevx = thisx;
        prevy = thisy;
      }
      break;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;
      for ( int linenr = 0, pointIndex = 0; linenr < nLines; ++linenr )
      {
        wkbPtr += 1 + sizeof( int );
        int nPoints;
        wkbPtr >> nPoints;

        double prevx = 0.0, prevy = 0.0;
        for ( int pointnr = 0; pointnr < nPoints; ++pointnr )
        {
          double thisx, thisy;
          wkbPtr >> thisx >> thisy;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          if ( pointnr > 0 )
          {
            if ( cs.isSegmentInRect( prevx, prevy, thisx, thisy ) )
            {
              QgsPoint edgePoints[2];
              edgePoints[0].set( prevx, prevy );
              edgePoints[1].set( thisx, thisy );
              lst << QgsPointLocator::Match( QgsPointLocator::Edge, vl, fid, 0, QgsPoint(), pointIndex - 1, edgePoints );
            }
          }

          prevx = thisx;
          prevy = thisy;
          ++pointIndex;
        }
      }
      break;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nRings;
      wkbPtr >> nRings;

      for ( int ringnr = 0, pointIndex = 0; ringnr < nRings; ++ringnr )//loop over rings
      {
        int nPoints;
        wkbPtr >> nPoints;

        double prevx = 0.0, prevy = 0.0;
        for ( int pointnr = 0; pointnr < nPoints; ++pointnr )//loop over points in a ring
        {
          double thisx, thisy;
          wkbPtr >> thisx >> thisy;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          if ( pointnr > 0 )
          {
            if ( cs.isSegmentInRect( prevx, prevy, thisx, thisy ) )
            {
              QgsPoint edgePoints[2];
              edgePoints[0].set( prevx, prevy );
              edgePoints[1].set( thisx, thisy );
              lst << QgsPointLocator::Match( QgsPointLocator::Edge, vl, fid, 0, QgsPoint(), pointIndex - 1, edgePoints );
            }
          }

          prevx = thisx;
          prevy = thisy;
          ++pointIndex;
        }
      }
      break;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolygons;
      wkbPtr >> nPolygons;
      for ( int polynr = 0, pointIndex = 0; polynr < nPolygons; ++polynr )
      {
        wkbPtr += 1 + sizeof( int );
        int nRings;
        wkbPtr >> nRings;
        for ( int ringnr = 0; ringnr < nRings; ++ringnr )
        {
          int nPoints;
          wkbPtr >> nPoints;

          double prevx = 0.0, prevy = 0.0;
          for ( int pointnr = 0; pointnr < nPoints; ++pointnr )
          {
            double thisx, thisy;
            wkbPtr >> thisx >> thisy;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            if ( pointnr > 0 )
            {
              if ( cs.isSegmentInRect( prevx, prevy, thisx, thisy ) )
              {
                QgsPoint edgePoints[2];
                edgePoints[0].set( prevx, prevy );
                edgePoints[1].set( thisx, thisy );
                lst << QgsPointLocator::Match( QgsPointLocator::Edge, vl, fid, 0, QgsPoint(), pointIndex - 1, edgePoints );
              }
            }

            prevx = thisx;
            prevy = thisy;
            ++pointIndex;
          }
        }
      }
      break;
    }

    case QGis::WKBUnknown:
    default:
      return lst;
      break;
  } // switch (wkbType)

  return lst;
}

/** Helper class used when traversing the index looking for edges - builds a list of matches. */
class QgsPointLocator_VisitorEdgesInRect : public IVisitor
{
  public:
    QgsPointLocator_VisitorEdgesInRect( QgsPointLocator* pl, QgsPointLocator::MatchList& lst, const QgsRectangle& srcRect, QgsPointLocator::MatchFilter* filter = 0 )
        : mLocator( pl ), mList( lst ), mSrcRect( srcRect ), mFilter( filter ) {}

    void visitNode( const INode& n ) override { Q_UNUSED( n ); }
    void visitData( std::vector<const IData*>& v ) override { Q_UNUSED( v ); }

    void visitData( const IData& d ) override
    {
      QgsFeatureId id = d.getIdentifier();
      QgsGeometry* geom = mLocator->mGeoms.value( id );

      foreach ( const QgsPointLocator::Match& m, _geometrySegmentsInRect( geom, mSrcRect, mLocator->mLayer, id ) )
      {
        // in range queries the filter may reject some matches
        if ( mFilter && !mFilter->acceptMatch( m ) )
          continue;

        mList << m;
      }
    }

  private:
    QgsPointLocator* mLocator;
    QgsPointLocator::MatchList& mList;
    QgsRectangle mSrcRect;
    QgsPointLocator::MatchFilter* mFilter;
};



////////////////////////////////////////////////////////////////////////////
#include <QStack>

/** Helper class to dump the R-index nodes and their content */
class QgsPointLocator_DumpTree : public SpatialIndex::IQueryStrategy
{
  private:
    QStack<id_type> ids;

  public:

    void getNextEntry( const IEntry& entry, id_type& nextEntry, bool& hasNext ) override
    {
      const INode* n = dynamic_cast<const INode*>( &entry );
      if ( !n )
        return;

      qDebug( "NODE: %ld", n->getIdentifier() );
      if ( n->getLevel() > 0 )
      {
        // inner nodes
        for ( uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++ )
        {
          qDebug( "- CH: %ld", n->getChildIdentifier( cChild ) );
          ids.push( n->getChildIdentifier( cChild ) );
        }
      }
      else
      {
        // leaves
        for ( uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++ )
        {
          qDebug( "- L: %ld", n->getChildIdentifier( cChild ) );
        }
      }

      if ( ! ids.empty() )
      {
        nextEntry = ids.back(); ids.pop();
        hasNext = true;
      }
      else
        hasNext = false;
    }
};

////////////////////////////////////////////////////////////////////////////


QgsPointLocator::QgsPointLocator( QgsVectorLayer* layer, const QgsCoordinateReferenceSystem* destCRS, const QgsRectangle* extent )
    : mStorage( 0 )
    , mRTree( 0 )
    , mIsEmptyLayer( false )
    , mTransform( 0 )
    , mLayer( layer )
    , mExtent( 0 )
{
  if ( destCRS )
  {
    mTransform = new QgsCoordinateTransform( layer->crs(), *destCRS );
  }

  if ( extent )
  {
    mExtent = new QgsRectangle( *extent );
  }

  mStorage = StorageManager::createNewMemoryStorageManager();

  connect( mLayer, SIGNAL( featureAdded( QgsFeatureId ) ), this, SLOT( onFeatureAdded( QgsFeatureId ) ) );
  connect( mLayer, SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( onFeatureDeleted( QgsFeatureId ) ) );
  connect( mLayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry& ) ), this, SLOT( onGeometryChanged( QgsFeatureId, QgsGeometry& ) ) );
}


QgsPointLocator::~QgsPointLocator()
{
  destroyIndex();
  delete mStorage;
  delete mTransform;
  delete mExtent;
}


bool QgsPointLocator::init( int maxFeaturesToIndex )
{
  return hasIndex() ? true : rebuildIndex( maxFeaturesToIndex );
}

bool QgsPointLocator::hasIndex() const
{
  return mRTree != 0 || mIsEmptyLayer;
}



bool QgsPointLocator::rebuildIndex( int maxFeaturesToIndex )
{
  destroyIndex();

  QLinkedList<RTree::Data*> dataList;
  QgsFeature f;
  QGis::GeometryType geomType = mLayer->geometryType();
  if ( geomType == QGis::NoGeometry )
    return true; // nothing to index

  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() );
  if ( mExtent )
  {
    QgsRectangle rect = *mExtent;
    if ( mTransform )
      rect = mTransform->transformBoundingBox( rect, QgsCoordinateTransform::ReverseTransform );
    request.setFilterRect( rect );
  }
  QgsFeatureIterator fi = mLayer->getFeatures( request );
  int indexedCount = 0;
  while ( fi.nextFeature( f ) )
  {
    if ( !f.geometry() )
      continue;

    if ( mTransform )
      f.geometry()->transform( *mTransform );

    SpatialIndex::Region r( rect2region( f.geometry()->boundingBox() ) );
    dataList << new RTree::Data( 0, 0, r, f.id() );
    mGeoms[f.id()] = new QgsGeometry( *f.geometry() );
    ++indexedCount;

    if ( maxFeaturesToIndex != -1 && indexedCount > maxFeaturesToIndex )
    {
      qDeleteAll( dataList );
      destroyIndex();
      return false;
    }
  }

  // R-Tree parameters
  double fillFactor = 0.7;
  unsigned long indexCapacity = 10;
  unsigned long leafCapacity = 10;
  unsigned long dimension = 2;
  RTree::RTreeVariant variant = RTree::RV_RSTAR;
  SpatialIndex::id_type indexId;

  if ( dataList.isEmpty() )
  {
    mIsEmptyLayer = true;
    return true; // no features
  }

  QgsPointLocator_Stream stream( dataList );
  mRTree = RTree::createAndBulkLoadNewRTree( RTree::BLM_STR, stream, *mStorage, fillFactor, indexCapacity,
           leafCapacity, dimension, variant, indexId );
  return true;
}


void QgsPointLocator::destroyIndex()
{
  delete mRTree;
  mRTree = 0;

  mIsEmptyLayer = false;

  foreach ( QgsGeometry* g, mGeoms )
    delete g;
  mGeoms.clear();
}

void QgsPointLocator::onFeatureAdded( QgsFeatureId fid )
{
  if ( !mRTree )
  {
    if ( mIsEmptyLayer )
      rebuildIndex(); // first feature - let's built the index
    return; // nothing to do if we are not initialized yet
  }

  QgsFeature f;
  if ( mLayer->getFeatures( QgsFeatureRequest( fid ) ).nextFeature( f ) )
  {
    if ( !f.geometry() )
      return;

    if ( mTransform )
      f.geometry()->transform( *mTransform );

    SpatialIndex::Region r( rect2region( f.geometry()->boundingBox() ) );
    mRTree->insertData( 0, 0, r, f.id() );
    mGeoms[fid] = new QgsGeometry( *f.geometry() );
  }
}

void QgsPointLocator::onFeatureDeleted( QgsFeatureId fid )
{
  if ( !mRTree )
    return; // nothing to do if we are not initialized yet

  if ( mGeoms.contains( fid ) )
  {
    mRTree->deleteData( rect2region( mGeoms[fid]->boundingBox() ), fid );
    mGeoms.remove( fid );
  }
}

void QgsPointLocator::onGeometryChanged( QgsFeatureId fid, QgsGeometry& geom )
{
  Q_UNUSED( geom );
  onFeatureDeleted( fid );
  onFeatureAdded( fid );
}


QgsPointLocator::Match QgsPointLocator::nearestVertex( const QgsPoint& point, double tolerance, MatchFilter* filter )
{
  if ( !mRTree )
  {
    init();
    if ( !mRTree ) // still invalid?
      return Match();
  }

  Match m;
  QgsPointLocator_VisitorNearestVertex visitor( this, m, point, filter );
  QgsRectangle rect( point.x() - tolerance, point.y() - tolerance, point.x() + tolerance, point.y() + tolerance );
  mRTree->intersectsWithQuery( rect2region( rect ), visitor );
  if ( m.isValid() && m.distance() > tolerance )
    return Match(); // // make sure that only match strictly within the tolerance is returned
  return m;
}

QgsPointLocator::Match QgsPointLocator::nearestEdge( const QgsPoint& point, double tolerance, MatchFilter* filter )
{
  if ( !mRTree )
  {
    init();
    if ( !mRTree ) // still invalid?
      return Match();
  }

  Match m;
  QgsPointLocator_VisitorNearestEdge visitor( this, m, point, filter );
  QgsRectangle rect( point.x() - tolerance, point.y() - tolerance, point.x() + tolerance, point.y() + tolerance );
  mRTree->intersectsWithQuery( rect2region( rect ), visitor );
  if ( m.isValid() && m.distance() > tolerance )
    return Match(); // // make sure that only match strictly within the tolerance is returned
  return m;
}

QgsPointLocator::MatchList QgsPointLocator::edgesInRect( const QgsRectangle& rect, QgsPointLocator::MatchFilter* filter )
{
  if ( !mRTree )
  {
    init();
    if ( !mRTree ) // still invalid?
      return MatchList();
  }

  MatchList lst;
  QgsPointLocator_VisitorEdgesInRect visitor( this, lst, rect, filter );
  mRTree->intersectsWithQuery( rect2region( rect ), visitor );

  return lst;
}

QgsPointLocator::MatchList QgsPointLocator::edgesInRect( const QgsPoint& point, double tolerance, QgsPointLocator::MatchFilter* filter )
{
  QgsRectangle rect( point.x() - tolerance, point.y() - tolerance, point.x() + tolerance, point.y() + tolerance );
  return edgesInRect( rect, filter );
}


QgsPointLocator::MatchList QgsPointLocator::pointInPolygon( const QgsPoint& point )
{
  if ( !mRTree )
  {
    init();
    if ( !mRTree ) // still invalid?
      return MatchList();
  }

  MatchList lst;
  QgsPointLocator_VisitorArea visitor( this, point, lst );
  mRTree->intersectsWithQuery( point2point( point ), visitor );
  return lst;
}
