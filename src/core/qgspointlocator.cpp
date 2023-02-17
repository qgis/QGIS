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

#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgswkbptr.h"
#include "qgis.h"
#include "qgslogger.h"
#include "qgsrenderer.h"
#include "qgsapplication.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsexpressioncontextutils.h"
#include "qgslinestring.h"
#include "qgscurvepolygon.h"
#include "qgsrendercontext.h"
#include "qgspointlocatorinittask.h"
#include <spatialindex/SpatialIndex.h>

#include <QLinkedListIterator>
#include <QtConcurrent>

using namespace SpatialIndex;



static SpatialIndex::Point point2point( const QgsPointXY &point )
{
  double plow[2] = { point.x(), point.y() };
  return Point( plow, 2 );
}


static SpatialIndex::Region rect2region( const QgsRectangle &rect )
{
  double pLow[2] = { rect.xMinimum(), rect.yMinimum() };
  double pHigh[2] = { rect.xMaximum(), rect.yMaximum() };
  return SpatialIndex::Region( pLow, pHigh, 2 );
}


// Ahh.... another magic number. Taken from QgsVectorLayer::snapToGeometry() call to closestSegmentWithContext().
// The default epsilon used for sqrDistToSegment (1e-8) is too high when working with lat/lon coordinates
// I still do not fully understand why the sqrDistToSegment() code uses epsilon and if the square distance
// is lower than epsilon it will have a special logic...
static const double POINT_LOC_EPSILON = 1e-12;

////////////////////////////////////////////////////////////////////////////


/**
 * \ingroup core
 * \brief Helper class for bulk loading of R-trees.
 * \note not available in Python bindings
*/
class QgsPointLocator_Stream : public IDataStream
{
  public:
    explicit QgsPointLocator_Stream( const QLinkedList<RTree::Data *> &dataList )
      : mDataList( dataList )
      , mIt( mDataList )
    { }

    IData *getNext() override { return mIt.next(); }
    bool hasNext() override { return mIt.hasNext(); }

    uint32_t size() override { Q_ASSERT( false && "not available" ); return 0; }
    void rewind() override { Q_ASSERT( false && "not available" ); }

  private:
    QLinkedList<RTree::Data *> mDataList;
    QLinkedListIterator<RTree::Data *> mIt;
};


////////////////////////////////////////////////////////////////////////////


/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for vertices - builds a list of matches.
 * \note not available in Python bindings
*/
class QgsPointLocator_VisitorNearestVertex : public IVisitor
{
  public:
    QgsPointLocator_VisitorNearestVertex( QgsPointLocator *pl, QgsPointLocator::Match &m, const QgsPointXY &srcPoint, QgsPointLocator::MatchFilter *filter = nullptr )
      : mLocator( pl )
      , mBest( m )
      , mSrcPoint( srcPoint )
      , mFilter( filter )
    {}

    void visitNode( const INode &n ) override { Q_UNUSED( n ) }
    void visitData( std::vector<const IData *> &v ) override { Q_UNUSED( v ) }

    void visitData( const IData &d ) override
    {
      const QgsFeatureId id = d.getIdentifier();
      QgsGeometry *geom = mLocator->mGeoms.value( id );
      if ( !geom )
        return; // should not happen, but be safe
      int vertexIndex, beforeVertex, afterVertex;
      double sqrDist;

      const QgsPointXY pt = geom->closestVertex( mSrcPoint, vertexIndex, beforeVertex, afterVertex, sqrDist );
      if ( sqrDist < 0 )
        return;  // probably empty geometry

      const QgsPointLocator::Match m( QgsPointLocator::Vertex, mLocator->mLayer, id, std::sqrt( sqrDist ), pt, vertexIndex );
      // in range queries the filter may reject some matches
      if ( mFilter && !mFilter->acceptMatch( m ) )
        return;

      if ( !mBest.isValid() || m.distance() < mBest.distance() )
        mBest = m;
    }

  private:
    QgsPointLocator *mLocator = nullptr;
    QgsPointLocator::Match &mBest;
    QgsPointXY mSrcPoint;
    QgsPointLocator::MatchFilter *mFilter = nullptr;
};



/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for centroid - builds a list of matches.
 * \note not available in Python bindings
 * \since QGIS 3.12
*/
class QgsPointLocator_VisitorNearestCentroid : public IVisitor
{
  public:

    /**
     * \ingroup core
     * \brief Helper class used when traversing the index looking for centroid - builds a list of matches.
     * \note not available in Python bindings
     * \since QGIS 3.12
    */
    QgsPointLocator_VisitorNearestCentroid( QgsPointLocator *pl, QgsPointLocator::Match &m, const QgsPointXY &srcPoint, QgsPointLocator::MatchFilter *filter = nullptr )
      : mLocator( pl )
      , mBest( m )
      , mSrcPoint( srcPoint )
      , mFilter( filter )
    {}

    void visitNode( const INode &n ) override { Q_UNUSED( n ) }
    void visitData( std::vector<const IData *> &v ) override { Q_UNUSED( v ) }

    void visitData( const IData &d ) override
    {
      const QgsFeatureId id = d.getIdentifier();
      QgsGeometry *geom = mLocator->mGeoms.value( id );
      if ( !geom )
        return; // should not happen, but be safe

      const QgsPointXY pt = geom->centroid().asPoint();

      const QgsPointLocator::Match m( QgsPointLocator::Centroid, mLocator->mLayer, id, std::sqrt( mSrcPoint.sqrDist( pt ) ), pt, -1 );
      // in range queries the filter may reject some matches
      if ( mFilter && !mFilter->acceptMatch( m ) )
        return;

      if ( !mBest.isValid() || m.distance() < mBest.distance() )
        mBest = m;

    }

  private:
    QgsPointLocator *mLocator = nullptr;
    QgsPointLocator::Match &mBest;
    QgsPointXY mSrcPoint;
    QgsPointLocator::MatchFilter *mFilter = nullptr;
};

////////////////////////////////////////////////////////////////////////////

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for middle segment - builds a list of matches.
 * \note not available in Python bindings
 * \since QGIS 3.12
*/
class QgsPointLocator_VisitorNearestMiddleOfSegment: public IVisitor
{
  public:

    /**
     * \ingroup core
     * \brief Helper class used when traversing the index looking for middle segment - builds a list of matches.
     * \note not available in Python bindings
     * \since QGIS 3.12
    */
    QgsPointLocator_VisitorNearestMiddleOfSegment( QgsPointLocator *pl, QgsPointLocator::Match &m, const QgsPointXY &srcPoint, QgsPointLocator::MatchFilter *filter = nullptr )
      : mLocator( pl )
      , mBest( m )
      , mSrcPoint( srcPoint )
      , mFilter( filter )
    {}

    void visitNode( const INode &n ) override { Q_UNUSED( n ) }
    void visitData( std::vector<const IData *> &v ) override { Q_UNUSED( v ) }

    void visitData( const IData &d ) override
    {
      const QgsFeatureId id = d.getIdentifier();
      QgsGeometry *geom = mLocator->mGeoms.value( id );
      if ( !geom )
        return; // should not happen, but be safe

      QgsPointXY pt;
      int afterVertex;
      const double sqrDist = geom->closestSegmentWithContext( mSrcPoint, pt, afterVertex, nullptr, POINT_LOC_EPSILON );
      if ( sqrDist < 0 )
        return;

      QgsPointXY edgePoints[2];
      edgePoints[0] = geom->vertexAt( afterVertex - 1 );
      edgePoints[1] = geom->vertexAt( afterVertex );
      pt = QgsPointXY( ( edgePoints[0].x() + edgePoints[1].x() ) / 2.0, ( edgePoints[0].y() + edgePoints[1].y() ) / 2.0 );

      const QgsPointLocator::Match m( QgsPointLocator::MiddleOfSegment, mLocator->mLayer, id, std::sqrt( mSrcPoint.sqrDist( pt ) ), pt, afterVertex - 1 );
      // in range queries the filter may reject some matches
      if ( mFilter && !mFilter->acceptMatch( m ) )
        return;

      if ( !mBest.isValid() || m.distance() < mBest.distance() )
        mBest = m;

    }

  private:
    QgsPointLocator *mLocator = nullptr;
    QgsPointLocator::Match &mBest;
    QgsPointXY mSrcPoint;
    QgsPointLocator::MatchFilter *mFilter = nullptr;
};

////////////////////////////////////////////////////////////////////////////

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for line endpoints (start or end vertex) - builds a list of matches.
 * \note not available in Python bindings
 * \since QGIS 3.20
*/
class QgsPointLocator_VisitorNearestLineEndpoint : public IVisitor
{
  public:

    /**
     * \ingroup core
     * \brief Helper class used when traversing the index looking for line endpoints (start or end vertex) - builds a list of matches.
    */
    QgsPointLocator_VisitorNearestLineEndpoint( QgsPointLocator *pl, QgsPointLocator::Match &m, const QgsPointXY &srcPoint, QgsPointLocator::MatchFilter *filter = nullptr )
      : mLocator( pl )
      , mBest( m )
      , mSrcPoint( srcPoint )
      , mFilter( filter )
    {}

    void visitNode( const INode &n ) override { Q_UNUSED( n ) }
    void visitData( std::vector<const IData *> &v ) override { Q_UNUSED( v ) }

    void visitData( const IData &d ) override
    {
      const QgsFeatureId id = d.getIdentifier();
      const QgsGeometry *geom = mLocator->mGeoms.value( id );
      if ( !geom )
        return; // should not happen, but be safe

      QgsPointXY bestPoint;
      int bestVertexNumber = -1;
      auto replaceIfBetter = [this, &bestPoint, &bestVertexNumber]( const QgsPoint & candidate, int vertexNumber )
      {
        if ( bestPoint.isEmpty() || candidate.distanceSquared( mSrcPoint.x(), mSrcPoint.y() ) < bestPoint.sqrDist( mSrcPoint ) )
        {
          bestPoint = QgsPointXY( candidate );
          bestVertexNumber = vertexNumber;
        }
      };

      switch ( QgsWkbTypes::geometryType( geom->wkbType() ) )
      {
        case QgsWkbTypes::PointGeometry:
        case QgsWkbTypes::UnknownGeometry:
        case QgsWkbTypes::NullGeometry:
          return;

        case QgsWkbTypes::LineGeometry:
        {
          int partStartVertexNum = 0;
          for ( auto partIt = geom->const_parts_begin(); partIt != geom->const_parts_end(); ++partIt )
          {
            if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( *partIt ) )
            {
              replaceIfBetter( curve->startPoint(), partStartVertexNum );
              replaceIfBetter( curve->endPoint(), partStartVertexNum + curve->numPoints() - 1 );
              partStartVertexNum += curve->numPoints();
            }
          }
          break;
        }

        case QgsWkbTypes::PolygonGeometry:
        {
          int partStartVertexNum = 0;
          for ( auto partIt = geom->const_parts_begin(); partIt != geom->const_parts_end(); ++partIt )
          {
            if ( const QgsCurvePolygon *polygon = qgsgeometry_cast< const QgsCurvePolygon * >( *partIt ) )
            {
              if ( polygon->exteriorRing() )
              {
                replaceIfBetter( polygon->exteriorRing()->startPoint(), partStartVertexNum );
                partStartVertexNum += polygon->exteriorRing()->numPoints();
              }
              for ( int i = 0; i < polygon->numInteriorRings(); ++i )
              {
                const QgsCurve *ring = polygon->interiorRing( i );
                replaceIfBetter( ring->startPoint(), partStartVertexNum );
                partStartVertexNum += ring->numPoints();
              }
            }
          }
          break;
        }
      }

      const QgsPointLocator::Match m( QgsPointLocator::LineEndpoint, mLocator->mLayer, id, std::sqrt( mSrcPoint.sqrDist( bestPoint ) ), bestPoint, bestVertexNumber );
      // in range queries the filter may reject some matches
      if ( mFilter && !mFilter->acceptMatch( m ) )
        return;

      if ( !mBest.isValid() || m.distance() < mBest.distance() )
        mBest = m;
    }

  private:
    QgsPointLocator *mLocator = nullptr;
    QgsPointLocator::Match &mBest;
    QgsPointXY mSrcPoint;
    QgsPointLocator::MatchFilter *mFilter = nullptr;
};


////////////////////////////////////////////////////////////////////////////


/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for edges - builds a list of matches.
 * \note not available in Python bindings
*/
class QgsPointLocator_VisitorNearestEdge : public IVisitor
{
  public:
    QgsPointLocator_VisitorNearestEdge( QgsPointLocator *pl, QgsPointLocator::Match &m, const QgsPointXY &srcPoint, QgsPointLocator::MatchFilter *filter = nullptr )
      : mLocator( pl )
      , mBest( m )
      , mSrcPoint( srcPoint )
      , mFilter( filter )
    {}

    void visitNode( const INode &n ) override { Q_UNUSED( n ) }
    void visitData( std::vector<const IData *> &v ) override { Q_UNUSED( v ) }

    void visitData( const IData &d ) override
    {
      const QgsFeatureId id = d.getIdentifier();
      QgsGeometry *geom = mLocator->mGeoms.value( id );
      if ( !geom )
        return; // should not happen, but be safe

      QgsPointXY pt;
      int afterVertex;
      const double sqrDist = geom->closestSegmentWithContext( mSrcPoint, pt, afterVertex, nullptr, POINT_LOC_EPSILON );
      if ( sqrDist < 0 )
        return;

      QgsPointXY edgePoints[2];
      edgePoints[0] = geom->vertexAt( afterVertex - 1 );
      edgePoints[1] = geom->vertexAt( afterVertex );
      const QgsPointLocator::Match m( QgsPointLocator::Edge, mLocator->mLayer, id, std::sqrt( sqrDist ), pt, afterVertex - 1, edgePoints );
      // in range queries the filter may reject some matches
      if ( mFilter && !mFilter->acceptMatch( m ) )
        return;

      if ( !mBest.isValid() || m.distance() < mBest.distance() )
        mBest = m;
    }

  private:
    QgsPointLocator *mLocator = nullptr;
    QgsPointLocator::Match &mBest;
    QgsPointXY mSrcPoint;
    QgsPointLocator::MatchFilter *mFilter = nullptr;
};


////////////////////////////////////////////////////////////////////////////

/**
 * \ingroup core
 * \brief Helper class used when traversing the index with areas - builds a list of matches.
 * \note not available in Python bindings
*/
class QgsPointLocator_VisitorArea : public IVisitor
{
  public:
    //! constructor
    QgsPointLocator_VisitorArea( QgsPointLocator *pl, const QgsPointXY &origPt, QgsPointLocator::MatchList &list )
      : mLocator( pl )
      , mList( list )
      , mGeomPt( QgsGeometry::fromPointXY( origPt ) )
    {}

    void visitNode( const INode &n ) override { Q_UNUSED( n ) }
    void visitData( std::vector<const IData *> &v ) override { Q_UNUSED( v ) }

    void visitData( const IData &d ) override
    {
      const QgsFeatureId id = d.getIdentifier();
      QgsGeometry *g = mLocator->mGeoms.value( id );
      if ( !g )
        return; // should not happen, but be safe

      if ( g->intersects( mGeomPt ) )
        mList << QgsPointLocator::Match( QgsPointLocator::Area, mLocator->mLayer, id, 0, mGeomPt.asPoint() );
    }
  private:
    QgsPointLocator *mLocator = nullptr;
    QgsPointLocator::MatchList &mList;
    QgsGeometry mGeomPt;
};


////////////////////////////////////////////////////////////////////////////

// code adapted from
// http://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
struct _CohenSutherland
{
  explicit _CohenSutherland( const QgsRectangle &rect ) : mRect( rect ) {}

  typedef int OutCode;

  static const int INSIDE = 0; // 0000
  static const int LEFT = 1;   // 0001
  static const int RIGHT = 2;  // 0010
  static const int BOTTOM = 4; // 0100
  static const int TOP = 8;    // 1000

  QgsRectangle mRect;

  OutCode computeOutCode( double x, double y )
  {
    OutCode code = INSIDE;  // initialized as being inside of clip window
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
        const OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

        // Now find the intersection point;
        // use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
        if ( outcodeOut & TOP )
        {
          // point is above the clip rectangle
          x = x0 + ( x1 - x0 ) * ( mRect.yMaximum() - y0 ) / ( y1 - y0 );
          y = mRect.yMaximum();
        }
        else if ( outcodeOut & BOTTOM )
        {
          // point is below the clip rectangle
          x = x0 + ( x1 - x0 ) * ( mRect.yMinimum() - y0 ) / ( y1 - y0 );
          y = mRect.yMinimum();
        }
        else if ( outcodeOut & RIGHT )
        {
          // point is to the right of clip rectangle
          y = y0 + ( y1 - y0 ) * ( mRect.xMaximum() - x0 ) / ( x1 - x0 );
          x = mRect.xMaximum();
        }
        else if ( outcodeOut & LEFT )
        {
          // point is to the left of clip rectangle
          y = y0 + ( y1 - y0 ) * ( mRect.xMinimum() - x0 ) / ( x1 - x0 );
          x = mRect.xMinimum();
        }
        else
          break;

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


static QgsPointLocator::MatchList _geometrySegmentsInRect( QgsGeometry *geom, const QgsRectangle &rect, QgsVectorLayer *vl, QgsFeatureId fid )
{
  // this code is stupidly based on QgsGeometry::closestSegmentWithContext
  // we need iterator for segments...

  QgsPointLocator::MatchList lst;

  // geom is converted to a MultiCurve
  QgsGeometry straightGeom = geom->convertToType( QgsWkbTypes::LineGeometry, true );
  // and convert to straight segemnt / converts curve to linestring
  straightGeom.convertToStraightSegment();

  // so, you must have multilinestring
  //
  // Special case: Intersections cannot be done on an empty linestring like
  // QgsGeometry(QgsLineString()) or QgsGeometry::fromWkt("LINESTRING EMPTY")
  if ( straightGeom.isEmpty() || ( ( straightGeom.type() != QgsWkbTypes::LineGeometry ) && ( !straightGeom.isMultipart() ) ) )
    return lst;

  _CohenSutherland cs( rect );

  int pointIndex = 0;
  for ( auto part = straightGeom.const_parts_begin(); part != straightGeom.const_parts_end(); ++part )
  {
    // Checking for invalid linestrings
    // A linestring should/(must?) have at least two points.
    QgsCurve *curve = qgsgeometry_cast<QgsCurve *>( *part );
    Q_ASSERT( !curve->hasCurvedSegments() );
    if ( curve->numPoints() < 2 )
      continue;

    QgsAbstractGeometry::vertex_iterator it = ( *part )->vertices_begin();
    QgsPointXY prevPoint( *it );
    it++;
    while ( it != ( *part )->vertices_end() )
    {
      const QgsPointXY thisPoint( *it );
      if ( cs.isSegmentInRect( prevPoint.x(), prevPoint.y(), thisPoint.x(), thisPoint.y() ) )
      {
        QgsPointXY edgePoints[2];
        edgePoints[0] = prevPoint;
        edgePoints[1] = thisPoint;
        lst << QgsPointLocator::Match( QgsPointLocator::Edge, vl, fid, 0, QgsPointXY(), pointIndex - 1, edgePoints );
      }
      prevPoint = QgsPointXY( *it );
      it++;
      pointIndex += 1;

    }
  }
  return lst;
}

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for edges - builds a list of matches.
 * \note not available in Python bindings
*/
class QgsPointLocator_VisitorEdgesInRect : public IVisitor
{
  public:
    QgsPointLocator_VisitorEdgesInRect( QgsPointLocator *pl, QgsPointLocator::MatchList &lst, const QgsRectangle &srcRect, QgsPointLocator::MatchFilter *filter = nullptr )
      : mLocator( pl )
      , mList( lst )
      , mSrcRect( srcRect )
      , mFilter( filter )
    {}

    void visitNode( const INode &n ) override { Q_UNUSED( n ) }
    void visitData( std::vector<const IData *> &v ) override { Q_UNUSED( v ) }

    void visitData( const IData &d ) override
    {
      const QgsFeatureId id = d.getIdentifier();
      QgsGeometry *geom = mLocator->mGeoms.value( id );
      if ( !geom )
        return; // should not happen, but be safe

      const auto segmentsInRect {_geometrySegmentsInRect( geom, mSrcRect, mLocator->mLayer, id )};
      for ( const QgsPointLocator::Match &m : segmentsInRect )
      {
        // in range queries the filter may reject some matches
        if ( mFilter && !mFilter->acceptMatch( m ) )
          continue;

        mList << m;
      }
    }

  private:
    QgsPointLocator *mLocator = nullptr;
    QgsPointLocator::MatchList &mList;
    QgsRectangle mSrcRect;
    QgsPointLocator::MatchFilter *mFilter = nullptr;
};

////////////////////////////////////////////////////////////////////////////

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for vertices - builds a list of matches.
 * \note not available in Python bindings
 * \since QGIS 3.6
*/
class QgsPointLocator_VisitorVerticesInRect : public IVisitor
{
  public:
    //! Constructs the visitor
    QgsPointLocator_VisitorVerticesInRect( QgsPointLocator *pl, QgsPointLocator::MatchList &lst, const QgsRectangle &srcRect, QgsPointLocator::MatchFilter *filter = nullptr )
      : mLocator( pl )
      , mList( lst )
      , mSrcRect( srcRect )
      , mFilter( filter )
    {}

    void visitNode( const INode &n ) override { Q_UNUSED( n ) }
    void visitData( std::vector<const IData *> &v ) override { Q_UNUSED( v ) }

    void visitData( const IData &d ) override
    {
      const QgsFeatureId id = d.getIdentifier();
      const QgsGeometry *geom = mLocator->mGeoms.value( id );
      if ( !geom )
        return; // should not happen, but be safe

      for ( QgsAbstractGeometry::vertex_iterator it = geom->vertices_begin(); it != geom->vertices_end(); ++it )
      {
        if ( mSrcRect.contains( *it ) )
        {
          const QgsPointLocator::Match m( QgsPointLocator::Vertex, mLocator->mLayer, id, 0, *it, geom->vertexNrFromVertexId( it.vertexId() ) );

          // in range queries the filter may reject some matches
          if ( mFilter && !mFilter->acceptMatch( m ) )
            continue;

          mList << m;
        }
      }
    }

  private:
    QgsPointLocator *mLocator = nullptr;
    QgsPointLocator::MatchList &mList;
    QgsRectangle mSrcRect;
    QgsPointLocator::MatchFilter *mFilter = nullptr;
};

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for centroid - builds a list of matches.
 * \note not available in Python bindings
 * \since QGIS 3.10
*/
class QgsPointLocator_VisitorCentroidsInRect : public IVisitor
{
  public:
    //! Constructs the visitor
    QgsPointLocator_VisitorCentroidsInRect( QgsPointLocator *pl, QgsPointLocator::MatchList &lst, const QgsRectangle &srcRect, QgsPointLocator::MatchFilter *filter = nullptr )
      : mLocator( pl )
      , mList( lst )
      , mSrcRect( srcRect )
      , mFilter( filter )
    {}

    void visitNode( const INode &n ) override { Q_UNUSED( n ); }
    void visitData( std::vector<const IData *> &v ) override { Q_UNUSED( v ); }

    void visitData( const IData &d ) override
    {
      const QgsFeatureId id = d.getIdentifier();
      const QgsGeometry *geom = mLocator->mGeoms.value( id );
      if ( !geom )
        return; // should not happen, but be safe

      const QgsPointXY centroid = geom->centroid().asPoint();
      if ( mSrcRect.contains( centroid ) )
      {
        const QgsPointLocator::Match m( QgsPointLocator::Centroid, mLocator->mLayer, id, 0, centroid, -1 );

        // in range queries the filter may reject some matches
        if ( !( mFilter && !mFilter->acceptMatch( m ) ) )
          mList << m;
      }
    }

  private:
    QgsPointLocator *mLocator = nullptr;
    QgsPointLocator::MatchList &mList;
    QgsRectangle mSrcRect;
    QgsPointLocator::MatchFilter *mFilter = nullptr;
};

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for middle segment - builds a list of matches.
 * \note not available in Python bindings
 * \since QGIS 3.10
*/
class QgsPointLocator_VisitorMiddlesInRect : public IVisitor
{
  public:
    //! Constructs the visitor
    QgsPointLocator_VisitorMiddlesInRect( QgsPointLocator *pl, QgsPointLocator::MatchList &lst, const QgsRectangle &srcRect, QgsPointLocator::MatchFilter *filter = nullptr )
      : mLocator( pl )
      , mList( lst )
      , mSrcRect( srcRect )
      , mFilter( filter )
    {}

    void visitNode( const INode &n ) override { Q_UNUSED( n ); }
    void visitData( std::vector<const IData *> &v ) override { Q_UNUSED( v ); }

    void visitData( const IData &d ) override
    {
      const QgsFeatureId id = d.getIdentifier();
      const QgsGeometry *geom = mLocator->mGeoms.value( id );
      if ( !geom )
        return; // should not happen, but be safe

      for ( QgsAbstractGeometry::const_part_iterator itPart = geom->const_parts_begin() ; itPart != geom->const_parts_end() ; ++itPart )
      {
        QgsAbstractGeometry::vertex_iterator it = ( *itPart )->vertices_begin();
        QgsAbstractGeometry::vertex_iterator itPrevious = ( *itPart )->vertices_begin();
        it++;
        for ( ; it != geom->vertices_end(); ++it, ++itPrevious )
        {
          const QgsPointXY pt( ( ( *itPrevious ).x() + ( *it ).x() ) / 2.0, ( ( *itPrevious ).y() + ( *it ).y() ) / 2.0 );
          if ( mSrcRect.contains( pt ) )
          {
            const QgsPointLocator::Match m( QgsPointLocator::MiddleOfSegment, mLocator->mLayer, id, 0, pt, geom->vertexNrFromVertexId( it.vertexId() ) );

            // in range queries the filter may reject some matches
            if ( mFilter && !mFilter->acceptMatch( m ) )
              continue;

            mList << m;
          }
        }
      }
    }

  private:
    QgsPointLocator *mLocator = nullptr;
    QgsPointLocator::MatchList &mList;
    QgsRectangle mSrcRect;
    QgsPointLocator::MatchFilter *mFilter = nullptr;
};

////////////////////////////////////////////////////////////////////////////
#include <QStack>

/**
 * \ingroup core
 * \brief Helper class to dump the R-index nodes and their content
 * \note not available in Python bindings
*/
class QgsPointLocator_DumpTree : public SpatialIndex::IQueryStrategy
{
  private:
    QStack<id_type> ids;

  public:

    void getNextEntry( const IEntry &entry, id_type &nextEntry, bool &hasNext ) override
    {
      const INode *n = dynamic_cast<const INode *>( &entry );
      if ( !n )
        return;

      QgsDebugMsgLevel( QStringLiteral( "NODE: %1" ).arg( n->getIdentifier() ), 4 );
      if ( n->getLevel() > 0 )
      {
        // inner nodes
        for ( uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++ )
        {
          QgsDebugMsgLevel( QStringLiteral( "- CH: %1" ).arg( n->getChildIdentifier( cChild ) ), 4 );
          ids.push( n->getChildIdentifier( cChild ) );
        }
      }
      else
      {
        // leaves
        for ( uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++ )
        {
          QgsDebugMsgLevel( QStringLiteral( "- L: %1" ).arg( n->getChildIdentifier( cChild ) ), 4 );
        }
      }

      if ( ! ids.empty() )
      {
        nextEntry = ids.back();
        ids.pop();
        hasNext = true;
      }
      else
        hasNext = false;
    }
};

////////////////////////////////////////////////////////////////////////////


QgsPointLocator::QgsPointLocator( QgsVectorLayer *layer, const QgsCoordinateReferenceSystem &destCRS, const QgsCoordinateTransformContext &transformContext, const QgsRectangle *extent )
  : mLayer( layer )
{
  if ( destCRS.isValid() )
  {
    mTransform = QgsCoordinateTransform( layer->crs(), destCRS, transformContext );
  }

  setExtent( extent );

  mStorage.reset( StorageManager::createNewMemoryStorageManager() );

  connect( mLayer, &QgsVectorLayer::featureAdded, this, &QgsPointLocator::onFeatureAdded );
  connect( mLayer, &QgsVectorLayer::featureDeleted, this, &QgsPointLocator::onFeatureDeleted );
  connect( mLayer, &QgsVectorLayer::geometryChanged, this, &QgsPointLocator::onGeometryChanged );
  connect( mLayer, &QgsVectorLayer::attributeValueChanged, this, &QgsPointLocator::onAttributeValueChanged );
  connect( mLayer, &QgsVectorLayer::dataChanged, this, &QgsPointLocator::destroyIndex );
}


QgsPointLocator::~QgsPointLocator()
{
  // don't delete a locator if there is an indexing task running on it
  mIsDestroying = true;
  if ( mIsIndexing )
    waitForIndexingFinished();

  destroyIndex();
}

QgsCoordinateReferenceSystem QgsPointLocator::destinationCrs() const
{
  return mTransform.isValid() ? mTransform.destinationCrs() : QgsCoordinateReferenceSystem();
}

void QgsPointLocator::setExtent( const QgsRectangle *extent )
{
  if ( mIsIndexing )
    // already indexing, return!
    return;

  mExtent.reset( extent ? new QgsRectangle( *extent ) : nullptr );

  destroyIndex();
}

void QgsPointLocator::setRenderContext( const QgsRenderContext *context )
{
  if ( mIsIndexing )
    // already indexing, return!
    return;

  disconnect( mLayer, &QgsVectorLayer::styleChanged, this, &QgsPointLocator::destroyIndex );

  destroyIndex();
  mContext.reset( nullptr );

  if ( context )
  {
    mContext = std::unique_ptr<QgsRenderContext>( new QgsRenderContext( *context ) );
    connect( mLayer, &QgsVectorLayer::styleChanged, this, &QgsPointLocator::destroyIndex );
  }

}

void QgsPointLocator::onInitTaskFinished()
{
  Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "QgsPointLocator::onInitTaskFinished", "was not called on main thread" );

  // Check that we don't call this method twice, when calling waitForFinished
  // for instance (because of taskCompleted signal)
  if ( !mIsIndexing )
    return;

  if ( mIsDestroying )
    return;

  mIsIndexing = false;
  mRenderer.reset();
  mSource.reset();

  // treat added and deleted feature while indexing
  for ( const QgsFeatureId fid : std::as_const( mAddedFeatures ) )
    onFeatureAdded( fid );
  mAddedFeatures.clear();

  for ( const QgsFeatureId fid : std::as_const( mDeletedFeatures ) )
    onFeatureDeleted( fid );
  mDeletedFeatures.clear();

  emit initFinished( mInitTask->isBuildOK() );
}

bool QgsPointLocator::init( int maxFeaturesToIndex, bool relaxed )
{
  const QgsWkbTypes::GeometryType geomType = mLayer->geometryType();
  if ( geomType == QgsWkbTypes::NullGeometry // nothing to index
       || hasIndex()
       || mIsIndexing ) // already indexing, return!
    return true;

  if ( !mLayer->dataProvider()
       || !mLayer->dataProvider()->isValid() )
    return false;

  mSource.reset( new QgsVectorLayerFeatureSource( mLayer ) );

  if ( mContext )
  {
    mRenderer.reset( mLayer->renderer() ? mLayer->renderer()->clone() : nullptr );
    mContext->expressionContext() << QgsExpressionContextUtils::layerScope( mLayer );
  }

  mIsIndexing = true;

  if ( relaxed )
  {
    mInitTask = new QgsPointLocatorInitTask( this );
    connect( mInitTask, &QgsPointLocatorInitTask::taskTerminated, this, &QgsPointLocator::onInitTaskFinished );
    connect( mInitTask, &QgsPointLocatorInitTask::taskCompleted, this, &QgsPointLocator::onInitTaskFinished );
    QgsApplication::taskManager()->addTask( mInitTask );
    return true;
  }
  else
  {
    const bool ok = rebuildIndex( maxFeaturesToIndex );
    mIsIndexing = false;
    emit initFinished( ok );
    return ok;
  }
}

void QgsPointLocator::waitForIndexingFinished()
{
  disconnect( mInitTask, &QgsPointLocatorInitTask::taskTerminated, this, &QgsPointLocator::onInitTaskFinished );
  disconnect( mInitTask, &QgsPointLocatorInitTask::taskCompleted, this, &QgsPointLocator::onInitTaskFinished );
  mInitTask->waitForFinished();

  if ( !mIsDestroying )
    onInitTaskFinished();
}

bool QgsPointLocator::hasIndex() const
{
  return mIsIndexing || mRTree || mIsEmptyLayer;
}

bool QgsPointLocator::prepare( bool relaxed )
{
  if ( mIsIndexing )
  {
    if ( relaxed )
      return false;
    else
      waitForIndexingFinished();
  }

  if ( !mRTree )
  {
    init( -1, relaxed );
    if ( ( relaxed && mIsIndexing ) || !mRTree ) // relaxed mode and currently indexing or still invalid?
      return false;
  }

  return true;
}

bool QgsPointLocator::rebuildIndex( int maxFeaturesToIndex )
{
  QElapsedTimer t;
  t.start();

  QgsDebugMsgLevel( QStringLiteral( "RebuildIndex start : %1" ).arg( mSource->id() ), 2 );

  destroyIndex();

  QLinkedList<RTree::Data *> dataList;
  QgsFeature f;

  QgsFeatureRequest request;
  request.setNoAttributes();

  if ( mExtent )
  {
    QgsRectangle rect = *mExtent;
    if ( !mTransform.isShortCircuited() )
    {
      QgsCoordinateTransform rectTransform = mTransform;
      rectTransform.setBallparkTransformsAreAppropriate( true );
      try
      {
        rect = rectTransform.transformBoundingBox( rect, Qgis::TransformDirection::Reverse );
      }
      catch ( const QgsException &e )
      {
        Q_UNUSED( e )
        // See https://github.com/qgis/QGIS/issues/20749
        QgsDebugMsg( QStringLiteral( "could not transform bounding box to map, skipping the snap filter (%1)" ).arg( e.what() ) );
      }
    }
    request.setFilterRect( rect );
  }

  bool filter = false;
  QgsRenderContext *ctx = nullptr;
  if ( mContext )
  {
    ctx = mContext.get();
    if ( mRenderer )
    {
      // setup scale for scale dependent visibility (rule based)
      mRenderer->startRender( *ctx, mSource->fields() );
      filter = mRenderer->capabilities() & QgsFeatureRenderer::Filter;
      request.setSubsetOfAttributes( mRenderer->usedAttributes( *ctx ), mSource->fields() );
    }
  }

  QgsFeatureIterator fi = mSource->getFeatures( request );
  int indexedCount = 0;

  while ( fi.nextFeature( f ) )
  {
    if ( !f.hasGeometry() )
      continue;

    if ( filter && ctx && mRenderer )
    {
      ctx->expressionContext().setFeature( f );
      if ( !mRenderer->willRenderFeature( f, *ctx ) )
      {
        continue;
      }
    }

    if ( mTransform.isValid() )
    {
      try
      {
        QgsGeometry transformedGeometry = f.geometry();
        transformedGeometry.transform( mTransform );
        f.setGeometry( transformedGeometry );
      }
      catch ( const QgsException &e )
      {
        Q_UNUSED( e )
        // See https://github.com/qgis/QGIS/issues/20749
        QgsDebugMsg( QStringLiteral( "could not transform geometry to map, skipping the snap for it (%1)" ).arg( e.what() ) );
        continue;
      }
    }

    const QgsRectangle bbox = f.geometry().boundingBox();
    if ( bbox.isFinite() )
    {
      SpatialIndex::Region r( rect2region( bbox ) );
      dataList << new RTree::Data( 0, nullptr, r, f.id() );

      auto it = mGeoms.find( f.id() );
      if ( it != mGeoms.end() )
      {
        delete *it;
        *it = new QgsGeometry( f.geometry() );
      }
      else
      {
        mGeoms[f.id()] = new QgsGeometry( f.geometry() );
      }
      ++indexedCount;
    }

    if ( maxFeaturesToIndex != -1 && indexedCount > maxFeaturesToIndex )
    {
      qDeleteAll( dataList );
      destroyIndex();
      return false;
    }
  }

  // R-Tree parameters
  const double fillFactor = 0.7;
  const unsigned long indexCapacity = 10;
  const unsigned long leafCapacity = 10;
  const unsigned long dimension = 2;
  const RTree::RTreeVariant variant = RTree::RV_RSTAR;
  SpatialIndex::id_type indexId;

  if ( dataList.isEmpty() )
  {
    mIsEmptyLayer = true;
    return true; // no features
  }

  QgsPointLocator_Stream stream( dataList );
  mRTree.reset( RTree::createAndBulkLoadNewRTree( RTree::BLM_STR, stream, *mStorage, fillFactor, indexCapacity,
                leafCapacity, dimension, variant, indexId ) );

  if ( ctx && mRenderer )
  {
    mRenderer->stopRender( *ctx );
  }

  QgsDebugMsgLevel( QStringLiteral( "RebuildIndex end : %1 ms (%2)" ).arg( t.elapsed() ).arg( mSource->id() ), 2 );

  return true;
}


void QgsPointLocator::destroyIndex()
{
  mRTree.reset();

  mIsEmptyLayer = false;

  qDeleteAll( mGeoms );

  mGeoms.clear();
}

void QgsPointLocator::onFeatureAdded( QgsFeatureId fid )
{
  if ( mIsIndexing )
  {
    // will modify index once current indexing is finished
    mAddedFeatures << fid;
    return;
  }

  if ( !mRTree )
  {
    if ( mIsEmptyLayer )
    {
      // layer is not empty any more, let's build the index
      mIsEmptyLayer = false;
      init();
    }
    return; // nothing to do if we are not initialized yet
  }

  QgsFeature f;
  if ( mLayer->getFeatures( mContext ? QgsFeatureRequest( fid ) : QgsFeatureRequest( fid ).setNoAttributes() ).nextFeature( f ) )
  {
    if ( !f.hasGeometry() )
      return;

    if ( mContext )
    {
      std::unique_ptr< QgsFeatureRenderer > renderer( mLayer->renderer() ? mLayer->renderer()->clone() : nullptr );
      QgsRenderContext *ctx = nullptr;

      mContext->expressionContext() << QgsExpressionContextUtils::layerScope( mLayer );
      ctx = mContext.get();
      if ( renderer && ctx )
      {
        bool pass = false;
        renderer->startRender( *ctx, mLayer->fields() );

        ctx->expressionContext().setFeature( f );
        if ( !renderer->willRenderFeature( f, *ctx ) )
        {
          pass = true;
        }

        renderer->stopRender( *ctx );
        if ( pass )
          return;
      }
    }

    if ( mTransform.isValid() )
    {
      try
      {
        QgsGeometry transformedGeom = f.geometry();
        transformedGeom.transform( mTransform );
        f.setGeometry( transformedGeom );
      }
      catch ( const QgsException &e )
      {
        Q_UNUSED( e )
        // See https://github.com/qgis/QGIS/issues/20749
        QgsDebugMsg( QStringLiteral( "could not transform geometry to map, skipping the snap for it (%1)" ).arg( e.what() ) );
        return;
      }
    }

    const QgsRectangle bbox = f.geometry().boundingBox();
    if ( bbox.isFinite() )
    {
      const SpatialIndex::Region r( rect2region( bbox ) );
      mRTree->insertData( 0, nullptr, r, f.id() );

      auto it = mGeoms.find( f.id() );
      if ( it != mGeoms.end() )
      {
        delete *it;
        *it = new QgsGeometry( f.geometry() );
      }
      else
      {
        mGeoms[fid] = new QgsGeometry( f.geometry() );
      }
    }
  }
}

void QgsPointLocator::onFeatureDeleted( QgsFeatureId fid )
{
  if ( mIsIndexing )
  {
    if ( mAddedFeatures.contains( fid ) )
    {
      mAddedFeatures.remove( fid );
    }
    else
    {
      // will modify index once current indexing is finished
      mDeletedFeatures << fid;
    }
    return;
  }

  if ( !mRTree )
    return; // nothing to do if we are not initialized yet

  auto it = mGeoms.find( fid );
  if ( it != mGeoms.end() )
  {
    mRTree->deleteData( rect2region( ( *it )->boundingBox() ), fid );
    delete *it;
    mGeoms.erase( it );
  }

}

void QgsPointLocator::onGeometryChanged( QgsFeatureId fid, const QgsGeometry &geom )
{
  Q_UNUSED( geom )
  onFeatureDeleted( fid );
  onFeatureAdded( fid );
}

void QgsPointLocator::onAttributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value )
{
  Q_UNUSED( idx )
  Q_UNUSED( value )
  if ( mContext )
  {
    onFeatureDeleted( fid );
    onFeatureAdded( fid );
  }
}


QgsPointLocator::Match QgsPointLocator::nearestVertex( const QgsPointXY &point, double tolerance, MatchFilter *filter, bool relaxed )
{
  if ( !prepare( relaxed ) )
    return Match();

  Match m;
  QgsPointLocator_VisitorNearestVertex visitor( this, m, point, filter );
  const QgsRectangle rect( point.x() - tolerance, point.y() - tolerance, point.x() + tolerance, point.y() + tolerance );
  mRTree->intersectsWithQuery( rect2region( rect ), visitor );
  if ( m.isValid() && m.distance() > tolerance )
    return Match(); // make sure that only match strictly within the tolerance is returned
  return m;
}

QgsPointLocator::Match QgsPointLocator::nearestCentroid( const QgsPointXY &point, double tolerance, MatchFilter *filter, bool relaxed )
{
  if ( !prepare( relaxed ) )
    return Match();

  Match m;
  QgsPointLocator_VisitorNearestCentroid visitor( this, m, point, filter );

  const QgsRectangle rect( point.x() - tolerance, point.y() - tolerance, point.x() + tolerance, point.y() + tolerance );
  mRTree->intersectsWithQuery( rect2region( rect ), visitor );
  if ( m.isValid() && m.distance() > tolerance )
    return Match(); // make sure that only match strictly within the tolerance is returned
  return m;
}

QgsPointLocator::Match QgsPointLocator::nearestMiddleOfSegment( const QgsPointXY &point, double tolerance, MatchFilter *filter, bool relaxed )
{
  if ( !prepare( relaxed ) )
    return Match();

  Match m;
  QgsPointLocator_VisitorNearestMiddleOfSegment visitor( this, m, point, filter );

  const QgsRectangle rect( point.x() - tolerance, point.y() - tolerance, point.x() + tolerance, point.y() + tolerance );
  mRTree->intersectsWithQuery( rect2region( rect ), visitor );
  if ( m.isValid() && m.distance() > tolerance )
    return Match(); // make sure that only match strictly within the tolerance is returned
  return m;
}

QgsPointLocator::Match QgsPointLocator::nearestLineEndpoints( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter, bool relaxed )
{
  if ( !prepare( relaxed ) )
    return Match();

  Match m;
  QgsPointLocator_VisitorNearestLineEndpoint visitor( this, m, point, filter );

  const QgsRectangle rect( point.x() - tolerance, point.y() - tolerance, point.x() + tolerance, point.y() + tolerance );
  mRTree->intersectsWithQuery( rect2region( rect ), visitor );
  if ( m.isValid() && m.distance() > tolerance )
    return Match(); // make sure that only match strictly within the tolerance is returned
  return m;
}

QgsPointLocator::Match QgsPointLocator::nearestEdge( const QgsPointXY &point, double tolerance, MatchFilter *filter, bool relaxed )
{
  if ( !prepare( relaxed ) )
    return Match();

  const QgsWkbTypes::GeometryType geomType = mLayer->geometryType();
  if ( geomType == QgsWkbTypes::PointGeometry )
    return Match();

  Match m;
  QgsPointLocator_VisitorNearestEdge visitor( this, m, point, filter );
  const QgsRectangle rect( point.x() - tolerance, point.y() - tolerance, point.x() + tolerance, point.y() + tolerance );
  mRTree->intersectsWithQuery( rect2region( rect ), visitor );
  if ( m.isValid() && m.distance() > tolerance )
    return Match(); // make sure that only match strictly within the tolerance is returned
  return m;
}

QgsPointLocator::Match QgsPointLocator::nearestArea( const QgsPointXY &point, double tolerance, MatchFilter *filter, bool relaxed )
{
  if ( !prepare( relaxed ) )
    return Match();

  const MatchList mlist = pointInPolygon( point );
  if ( !mlist.isEmpty() && mlist.at( 0 ).isValid() )
  {
    return mlist.at( 0 );
  }

  if ( tolerance == 0 )
  {
    return Match();
  }

  // discard point and line layers to keep only polygons
  const QgsWkbTypes::GeometryType geomType = mLayer->geometryType();
  if ( geomType == QgsWkbTypes::PointGeometry || geomType == QgsWkbTypes::LineGeometry )
    return Match();

  // use edges for adding tolerance
  const Match m = nearestEdge( point, tolerance, filter );
  if ( m.isValid() )
    return Match( Area, m.layer(), m.featureId(), m.distance(), m.point() );
  else
    return Match();
}


QgsPointLocator::MatchList QgsPointLocator::edgesInRect( const QgsRectangle &rect, QgsPointLocator::MatchFilter *filter, bool relaxed )
{
  if ( !prepare( relaxed ) )
    return MatchList();

  const QgsWkbTypes::GeometryType geomType = mLayer->geometryType();
  if ( geomType == QgsWkbTypes::PointGeometry )
    return MatchList();

  MatchList lst;
  QgsPointLocator_VisitorEdgesInRect visitor( this, lst, rect, filter );
  mRTree->intersectsWithQuery( rect2region( rect ), visitor );

  return lst;
}

QgsPointLocator::MatchList QgsPointLocator::edgesInRect( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter, bool relaxed )
{
  const QgsRectangle rect( point.x() - tolerance, point.y() - tolerance, point.x() + tolerance, point.y() + tolerance );
  return edgesInRect( rect, filter, relaxed );
}

QgsPointLocator::MatchList QgsPointLocator::verticesInRect( const QgsRectangle &rect, QgsPointLocator::MatchFilter *filter, bool relaxed )
{
  if ( !prepare( relaxed ) )
    return MatchList();

  MatchList lst;
  QgsPointLocator_VisitorVerticesInRect visitor( this, lst, rect, filter );
  mRTree->intersectsWithQuery( rect2region( rect ), visitor );

  return lst;
}

QgsPointLocator::MatchList QgsPointLocator::verticesInRect( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter, bool relaxed )
{
  const QgsRectangle rect( point.x() - tolerance, point.y() - tolerance, point.x() + tolerance, point.y() + tolerance );
  return verticesInRect( rect, filter, relaxed );
}

QgsPointLocator::MatchList QgsPointLocator::pointInPolygon( const QgsPointXY &point, bool relaxed )
{
  if ( !prepare( relaxed ) )
    return MatchList();

  const QgsWkbTypes::GeometryType geomType = mLayer->geometryType();
  if ( geomType == QgsWkbTypes::PointGeometry || geomType == QgsWkbTypes::LineGeometry )
    return MatchList();

  MatchList lst;
  QgsPointLocator_VisitorArea visitor( this, point, lst );
  mRTree->intersectsWithQuery( point2point( point ), visitor );
  return lst;
}
