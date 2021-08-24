/***************************************************************************
  qgsgeometrysnappersinglesource.cpp
  ---------------------
  Date                 : May 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrysnappersinglesource.h"

#include "qgsfeatureiterator.h"
#include "qgsfeaturesink.h"
#include "qgsfeaturesource.h"
#include "qgsfeedback.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsspatialindex.h"

//! record about vertex coordinates and index of anchor to which it is snapped
struct AnchorPoint
{
  //! coordinates of the point
  double x, y;

  /**
   * Anchor information:
   *  0+ - index of anchor to which this point should be snapped
   * -1  - initial value (undefined)
   * -2  - this point is an anchor, i.e. do not snap this point (snap others to this point)
   */
  int anchor;
};


//! record about anchor being along a segment
struct AnchorAlongSegment
{
  int anchor;    //!< Index of the anchor point
  double along;  //!< Distance of the anchor point along the segment
};


static void buildSnapIndex( QgsFeatureIterator &fi, QgsSpatialIndex &index, QVector<AnchorPoint> &pnts, QgsFeedback *feedback, long long &count, long long totalCount )
{
  QgsFeature f;
  int pntId = 0;

  while ( fi.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    const QgsGeometry g = f.geometry();

    for ( auto it = g.vertices_begin(); it != g.vertices_end(); ++it )
    {
      const QgsPoint pt = *it;
      const QgsRectangle rect( pt.x(), pt.y(), pt.x(), pt.y() );

      const QList<QgsFeatureId> ids = index.intersects( rect );
      if ( ids.isEmpty() )
      {
        // add to tree and to structure
        index.addFeature( pntId, pt.boundingBox() );

        AnchorPoint xp;
        xp.x = pt.x();
        xp.y = pt.y();
        xp.anchor = -1;
        pnts.append( xp );
        pntId++;
      }
    }

    ++count;
    feedback->setProgress( 100. * count / totalCount );
  }
}


static void assignAnchors( QgsSpatialIndex &index, QVector<AnchorPoint> &pnts, double thresh )
{
  const double thresh2 = thresh * thresh;
  int nanchors = 0, ntosnap = 0;
  for ( int point = 0; point < pnts.count(); ++point )
  {
    if ( pnts[point].anchor >= 0 )
      continue;

    pnts[point].anchor = -2; // make it anchor
    nanchors++;

    // Find points in threshold
    double x = pnts[point].x, y = pnts[point].y;
    const QgsRectangle rect( x - thresh, y - thresh, x + thresh, y + thresh );

    const QList<QgsFeatureId> ids = index.intersects( rect );
    for ( const QgsFeatureId pointb : ids )
    {
      if ( pointb == point )
        continue;

      const double dx = pnts[pointb].x - pnts[point].x;
      const double dy = pnts[pointb].y - pnts[point].y;
      const double dist2 = dx * dx + dy * dy;
      if ( dist2 > thresh2 )
        continue;   // outside threshold

      if ( pnts[pointb].anchor == -1 )
      {
        // doesn't have an anchor yet
        pnts[pointb].anchor = point;
        ntosnap++;
      }
      else if ( pnts[pointb].anchor >= 0 )
      {
        // check distance to previously assigned anchor
        const double dx2 = pnts[pnts[pointb].anchor].x - pnts[pointb].x;
        const double dy2 = pnts[pnts[pointb].anchor].y - pnts[pointb].y;
        const double dist2_a = dx2 * dx2 + dy2 * dy2;
        if ( dist2 < dist2_a )
          pnts[pointb].anchor = point;   // replace old anchor
      }
    }
  }
}


static bool snapPoint( QgsPoint *pt, QgsSpatialIndex &index, QVector<AnchorPoint> &pnts )
{
  // Find point ( should always find one point )
  QList<QgsFeatureId> fids = index.intersects( QgsRectangle( pt->x(), pt->y(), pt->x(), pt->y() ) );
  Q_ASSERT( fids.count() == 1 );

  const int spoint = fids[0];
  const int anchor = pnts[spoint].anchor;

  if ( anchor >= 0 )
  {
    // to be snapped
    pt->setX( pnts[anchor].x );
    pt->setY( pnts[anchor].y );
    return true;
  }

  return false;
}


static bool snapLineString( QgsLineString *linestring, QgsSpatialIndex &index, QVector<AnchorPoint> &pnts, double thresh )
{
  QVector<QgsPoint> newPoints;
  QVector<int> anchors;  // indexes of anchors for vertices
  const double thresh2 = thresh * thresh;
  double minDistX, minDistY;   // coordinates of the closest point on the segment line
  bool changed = false;

  // snap vertices
  for ( int v = 0; v < linestring->numPoints(); v++ )
  {
    const double x = linestring->xAt( v );
    const double y = linestring->yAt( v );
    const QgsRectangle rect( x, y, x, y );

    // Find point ( should always find one point )
    QList<QgsFeatureId> fids = index.intersects( rect );
    Q_ASSERT( fids.count() == 1 );

    const int spoint = fids.first();
    const int anchor = pnts[spoint].anchor;
    if ( anchor >= 0 )
    {
      // to be snapped
      linestring->setXAt( v, pnts[anchor].x );
      linestring->setYAt( v, pnts[anchor].y );
      anchors.append( anchor ); // point on new location
      changed = true;
    }
    else
    {
      anchors.append( spoint ); // old point
    }
  }

  // Snap all segments to anchors in threshold
  for ( int v = 0; v < linestring->numPoints() - 1; v++ )
  {
    const double x1 = linestring->xAt( v );
    const double x2 = linestring->xAt( v + 1 );
    const double y1 = linestring->yAt( v );
    const double y2 = linestring->yAt( v + 1 );

    newPoints << linestring->pointN( v );

    // Box
    double xmin = x1, xmax = x2, ymin = y1, ymax = y2;
    if ( xmin > xmax )
      std::swap( xmin, xmax );
    if ( ymin > ymax )
      std::swap( ymin, ymax );

    const QgsRectangle rect( xmin - thresh, ymin - thresh, xmax + thresh, ymax + thresh );

    // Find points
    const QList<QgsFeatureId> fids = index.intersects( rect );

    QVector<AnchorAlongSegment> newVerticesAlongSegment;

    // Snap to anchor in threshold different from end points
    for ( const QgsFeatureId fid : fids )
    {
      const int spoint = fid;

      if ( spoint == anchors[v] || spoint == anchors[v + 1] )
        continue; // end point
      if ( pnts[spoint].anchor >= 0 )
        continue; // point is not anchor

      // Check the distance
      const double dist2 = QgsGeometryUtils::sqrDistToLine( pnts[spoint].x, pnts[spoint].y, x1, y1, x2, y2, minDistX, minDistY, 0 );
      // skip points that are behind segment's endpoints or extremely close to them
      double dx1 = minDistX - x1, dx2 = minDistX - x2;
      double dy1 = minDistY - y1, dy2 = minDistY - y2;
      const bool isOnSegment = !qgsDoubleNear( dx1 * dx1 + dy1 * dy1, 0 ) && !qgsDoubleNear( dx2 * dx2 + dy2 * dy2, 0 );
      if ( isOnSegment && dist2 <= thresh2 )
      {
        // an anchor is in the threshold
        AnchorAlongSegment item;
        item.anchor = spoint;
        item.along = QgsPointXY( x1, y1 ).distance( minDistX, minDistY );
        newVerticesAlongSegment << item;
      }
    }

    if ( !newVerticesAlongSegment.isEmpty() )
    {
      // sort by distance along the segment
      std::sort( newVerticesAlongSegment.begin(), newVerticesAlongSegment.end(), []( AnchorAlongSegment p1, AnchorAlongSegment p2 )
      {
        return p1.along < p2.along;
      } );

      // insert new vertices
      for ( int i = 0; i < newVerticesAlongSegment.count(); i++ )
      {
        const int anchor = newVerticesAlongSegment[i].anchor;
        newPoints << QgsPoint( pnts[anchor].x, pnts[anchor].y, 0 );
      }
      changed = true;
    }
  }

  // append end point
  newPoints << linestring->pointN( linestring->numPoints() - 1 );

  // replace linestring's points
  if ( changed )
    linestring->setPoints( newPoints );

  return changed;
}


static bool snapGeometry( QgsAbstractGeometry *g, QgsSpatialIndex &index, QVector<AnchorPoint> &pnts, double thresh )
{
  bool changed = false;
  if ( QgsLineString *linestring = qgsgeometry_cast<QgsLineString *>( g ) )
  {
    changed |= snapLineString( linestring, index, pnts, thresh );
  }
  else if ( QgsPolygon *polygon = qgsgeometry_cast<QgsPolygon *>( g ) )
  {
    if ( QgsLineString *exteriorRing = qgsgeometry_cast<QgsLineString *>( polygon->exteriorRing() ) )
      changed |= snapLineString( exteriorRing, index, pnts, thresh );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      if ( QgsLineString *interiorRing = qgsgeometry_cast<QgsLineString *>( polygon->interiorRing( i ) ) )
        changed |= snapLineString( interiorRing, index, pnts, thresh );
    }
  }
  else if ( QgsGeometryCollection *collection = qgsgeometry_cast<QgsGeometryCollection *>( g ) )
  {
    for ( int i = 0; i < collection->numGeometries(); ++i )
      changed |= snapGeometry( collection->geometryN( i ), index, pnts, thresh );
  }
  else if ( QgsPoint *pt = qgsgeometry_cast<QgsPoint *>( g ) )
  {
    changed |= snapPoint( pt, index, pnts );
  }

  return changed;
}


int QgsGeometrySnapperSingleSource::run( const QgsFeatureSource &source, QgsFeatureSink &sink, double thresh, QgsFeedback *feedback )
{
  // the logic here comes from GRASS implementation of Vect_snap_lines_list()

  long long count = 0;
  const long long totalCount = source.featureCount() * 2;

  // step 1: record all point locations in a spatial index + extra data structure to keep
  // reference to which other point they have been snapped to (in the next phase).

  QgsSpatialIndex index;
  QVector<AnchorPoint> pnts;
  QgsFeatureRequest request;
  request.setNoAttributes();
  QgsFeatureIterator fi = source.getFeatures( request );
  buildSnapIndex( fi, index, pnts, feedback, count, totalCount );

  if ( feedback->isCanceled() )
    return 0;

  // step 2: go through all registered points and if not yet marked mark it as anchor and
  // assign this anchor to all not yet marked points in threshold

  assignAnchors( index, pnts, thresh );

  // step 3: alignment of vertices and segments to the anchors
  // Go through all lines and:
  //   1) for all vertices: if not anchor snap it to its anchor
  //   2) for all segments: snap it to all anchors in threshold (except anchors of vertices of course)

  int modified = 0;
  QgsFeature f;
  fi = source.getFeatures();
  while ( fi.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    QgsGeometry geom = f.geometry();
    if ( snapGeometry( geom.get(), index, pnts, thresh ) )
    {
      f.setGeometry( geom );
      ++modified;
    }

    sink.addFeature( f, QgsFeatureSink::FastInsert );

    ++count;
    feedback->setProgress( 100. * count / totalCount );
  }

  return modified;
}
