/***************************************************************************
  qgstessellator.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstessellator.h"

#include "qgscurve.h"
#include "qgsgeometry.h"
#include "qgsmessagelog.h"
#include "qgsmultipolygon.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgstriangle.h"
#include "qgis_sip.h"

#include "poly2tri.h"

#include <QtDebug>
#include <QMatrix4x4>
#include <QVector3D>
#include <algorithm>


static void make_quad( float x0, float y0, float z0, float x1, float y1, float z1, float height, QVector<float> &data, bool addNormals )
{
  float dx = x1 - x0;
  float dy = -( y1 - y0 );

  // perpendicular vector in plane to [x,y] is [-y,x]
  QVector3D vn( -dy, 0, dx );
  vn = -vn;
  vn.normalize();

  // triangle 1
  data << x0 << z0 + height << -y0;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  data << x1 << z1 + height << -y1;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  data << x0 << z0 << -y0;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();

  // triangle 2
  data << x0 << z0 << -y0;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  data << x1 << z1 + height << -y1;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  data << x1 << z1 << -y1;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
}


QgsTessellator::QgsTessellator( double originX, double originY, bool addNormals, bool invertNormals, bool addBackFaces )
  : mOriginX( originX )
  , mOriginY( originY )
  , mAddNormals( addNormals )
  , mInvertNormals( invertNormals )
  , mAddBackFaces( addBackFaces )
{
  mStride = 3 * sizeof( float );
  if ( addNormals )
    mStride += 3 * sizeof( float );
}


static bool _isRingCounterClockWise( const QgsCurve &ring )
{
  double a = 0;
  int count = ring.numPoints();
  QgsVertexId::VertexType vt;
  QgsPoint pt, ptPrev;
  ring.pointAt( 0, ptPrev, vt );
  for ( int i = 1; i < count + 1; ++i )
  {
    ring.pointAt( i % count, pt, vt );
    a += ptPrev.x() * pt.y() - ptPrev.y() * pt.x();
    ptPrev = pt;
  }
  return a > 0; // clockwise if a is negative
}

static void _makeWalls( const QgsCurve &ring, bool ccw, float extrusionHeight, QVector<float> &data, bool addNormals, double originX, double originY )
{
  // we need to find out orientation of the ring so that the triangles we generate
  // face the right direction
  // (for exterior we want clockwise order, for holes we want counter-clockwise order)
  bool is_counter_clockwise = _isRingCounterClockWise( ring );

  QgsVertexId::VertexType vt;
  QgsPoint pt;

  QgsPoint ptPrev;
  ring.pointAt( is_counter_clockwise == ccw ? 0 : ring.numPoints() - 1, ptPrev, vt );
  for ( int i = 1; i < ring.numPoints(); ++i )
  {
    ring.pointAt( is_counter_clockwise == ccw ? i : ring.numPoints() - i - 1, pt, vt );
    float x0 = ptPrev.x() - originX, y0 = ptPrev.y() - originY;
    float x1 = pt.x() - originX, y1 = pt.y() - originY;
    float z0 = std::isnan( ptPrev.z() ) ? 0 : ptPrev.z();
    float z1 = std::isnan( pt.z() ) ? 0 : pt.z();

    // make a quad
    make_quad( x0, y0, z0, x1, y1, z1, extrusionHeight, data, addNormals );
    ptPrev = pt;
  }
}

static QVector3D _calculateNormal( const QgsCurve *curve, double originX, double originY, bool invertNormal )
{
  QgsVertexId::VertexType vt;
  QgsPoint pt1, pt2;

  // if it is just plain 2D curve there is no need to calculate anything
  // because it will be a flat horizontally oriented patch
  if ( !QgsWkbTypes::hasZ( curve->wkbType() ) )
    return QVector3D( 0, 0, 1 );

  // often we have 3D coordinates, but Z is the same for all vertices
  // so in order to save calculation and avoid possible issues with order of vertices
  // (the calculation below may decide that a polygon faces downwards)
  bool sameZ = true;
  curve->pointAt( 0, pt1, vt );
  for ( int i = 1; i < curve->numPoints(); i++ )
  {
    curve->pointAt( i, pt2, vt );
    if ( pt1.z() != pt2.z() )
    {
      sameZ = false;
      break;
    }
  }
  if ( sameZ )
    return QVector3D( 0, 0, 1 );

  // Calculate the polygon's normal vector, based on Newell's method
  // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
  //
  // Order of vertices is important here as it determines the front/back face of the polygon

  double nx = 0, ny = 0, nz = 0;
  for ( int i = 0; i < curve->numPoints() - 1; i++ )
  {
    curve->pointAt( i, pt1, vt );
    curve->pointAt( i + 1, pt2, vt );

    // shift points by the tessellator's origin - this does not affect normal calculation and it may save us from losing some precision
    pt1.setX( pt1.x() - originX );
    pt1.setY( pt1.y() - originY );
    pt2.setX( pt2.x() - originX );
    pt2.setY( pt2.y() - originY );

    if ( std::isnan( pt1.z() ) || std::isnan( pt2.z() ) )
      continue;

    nx += ( pt1.y() - pt2.y() ) * ( pt1.z() + pt2.z() );
    ny += ( pt1.z() - pt2.z() ) * ( pt1.x() + pt2.x() );
    nz += ( pt1.x() - pt2.x() ) * ( pt1.y() + pt2.y() );
  }

  QVector3D normal( nx, ny, nz );
  if ( invertNormal )
    normal = -normal;
  normal.normalize();
  return normal;
}


static void _normalVectorToXYVectors( const QVector3D &pNormal, QVector3D &pXVector, QVector3D &pYVector )
{
  // Here we define the two perpendicular vectors that define the local
  // 2D space on the plane. They will act as axis for which we will
  // calculate the projection coordinates of a 3D point to the plane.
  if ( pNormal.z() > 0.001 || pNormal.z() < -0.001 )
  {
    pXVector = QVector3D( 1, 0, -pNormal.x() / pNormal.z() );
  }
  else if ( pNormal.y() > 0.001 || pNormal.y() < -0.001 )
  {
    pXVector = QVector3D( 1, -pNormal.x() / pNormal.y(), 0 );
  }
  else
  {
    pXVector = QVector3D( -pNormal.y() / pNormal.x(), 1, 0 );
  }
  pXVector.normalize();
  pYVector = QVector3D::normal( pNormal, pXVector );
}


static void _ringToPoly2tri( const QgsCurve *ring, std::vector<p2t::Point *> &polyline, QHash<p2t::Point *, float> &zHash )
{
  QgsVertexId::VertexType vt;
  QgsPoint pt;

  const int pCount = ring->numPoints();

  polyline.reserve( pCount );

  for ( int i = 0; i < pCount - 1; ++i )
  {
    ring->pointAt( i, pt, vt );
    const float x = pt.x();
    const float y = pt.y();
    const float z = pt.z();

    const bool found = std::find_if( polyline.begin(), polyline.end(), [x, y]( p2t::Point *&p ) { return *p == p2t::Point( x, y ); } ) != polyline.end();

    if ( found )
    {
      continue;
    }

    p2t::Point *pt2 = new p2t::Point( x, y );
    polyline.push_back( pt2 );
    zHash[pt2] = z;
  }
}


inline double _round_coord( double x )
{
  const double exp = 1e10;   // round to 10 decimal digits
  return round( x * exp ) / exp;
}


static QgsCurve *_transform_ring_to_new_base( const QgsCurve &curve, const QgsPoint &pt0, const QMatrix4x4 *toNewBase )
{
  int count = curve.numPoints();
  QVector<QgsPoint> pts;
  pts.reserve( count );
  QgsVertexId::VertexType vt;
  for ( int i = 0; i < count; ++i )
  {
    QgsPoint pt;
    curve.pointAt( i, pt, vt );
    QgsPoint pt2( QgsWkbTypes::PointZ, pt.x() - pt0.x(), pt.y() - pt0.y(), std::isnan( pt.z() ) ? 0 : pt.z() - pt0.z() );
    QVector4D v( pt2.x(), pt2.y(), pt2.z(), 0 );
    if ( toNewBase )
      v = toNewBase->map( v );

    // we also round coordinates before passing them to poly2tri triangulation in order to fix possible numerical
    // stability issues. We had crashes with nearly collinear points where one of the points was off by a tiny bit (e.g. by 1e-20).
    // See TestQgsTessellator::testIssue17745().
    //
    // A hint for a similar issue: https://github.com/greenm01/poly2tri/issues/99
    //
    //    The collinear tests uses epsilon 1e-12. Seems rounding to 12 places you still
    //    can get problems with this test when points are pretty much on a straight line.
    //    I suggest you round to 10 decimals for stability and you can live with that
    //    precision.

    pts << QgsPoint( QgsWkbTypes::PointZ, _round_coord( v.x() ), _round_coord( v.y() ), _round_coord( v.z() ) );
  }
  return new QgsLineString( pts );
}


static QgsPolygon *_transform_polygon_to_new_base( const QgsPolygon &polygon, const QgsPoint &pt0, const QMatrix4x4 *toNewBase )
{
  QgsPolygon *p = new QgsPolygon;
  p->setExteriorRing( _transform_ring_to_new_base( *polygon.exteriorRing(), pt0, toNewBase ) );
  for ( int i = 0; i < polygon.numInteriorRings(); ++i )
    p->addInteriorRing( _transform_ring_to_new_base( *polygon.interiorRing( i ), pt0, toNewBase ) );
  return p;
}

static bool _check_intersecting_rings( const QgsPolygon &polygon )
{
  QList<QgsGeometry> geomRings;
  geomRings << QgsGeometry( polygon.exteriorRing()->clone() );
  for ( int i = 0; i < polygon.numInteriorRings(); ++i )
    geomRings << QgsGeometry( polygon.interiorRing( i )->clone() );

  // we need to make sure that the polygon has no rings with self-intersection: that may
  // crash the tessellator. The original geometry maybe have been valid and the self-intersection
  // was introduced when transforming to a new base (in a rare case when all points are not in the same plane)

  for ( int i = 0; i < geomRings.count(); ++i )
  {
    if ( !geomRings[i].isSimple() )
      return false;
  }

  // At this point we assume that input polygons are valid according to the OGC definition.
  // This means e.g. no duplicate points, polygons are simple (no butterfly shaped polygon with self-intersection),
  // internal rings are inside exterior rings, rings do not cross each other, no dangles.

  // There is however an issue with polygons where rings touch:
  //  +---+
  //  |   |
  //  | +-+-+
  //  | | | |
  //  | +-+ |
  //  |     |
  //  +-----+
  // This is a valid polygon with one exterior and one interior ring that touch at one point,
  // but poly2tri library does not allow interior rings touch each other or exterior ring.
  // TODO: Handle the situation better - rather than just detecting the problem, try to fix
  // it by converting touching rings into one ring.

  if ( polygon.numInteriorRings() > 0 )
  {
    for ( int i = 0; i < geomRings.count(); ++i )
      for ( int j = i + 1; j < geomRings.count(); ++j )
      {
        if ( geomRings[i].intersects( geomRings[j] ) )
          return false;
      }
  }
  return true;
}


double _minimum_distance_between_coordinates( const QgsPolygon &polygon )
{
  double min_d = 1e20;
  auto it = polygon.vertices_begin();

  if ( it == polygon.vertices_end() )
    return min_d;

  QgsPoint p0 = *it;
  ++it;
  for ( ; it != polygon.vertices_end(); ++it )
  {
    QgsPoint p1 = *it;
    double d = p0.distance( p1 );
    if ( d < min_d )
      min_d = d;
    p0 = p1;
  }
  return min_d;
}


void QgsTessellator::addPolygon( const QgsPolygon &polygon, float extrusionHeight )
{
  const QgsCurve *exterior = polygon.exteriorRing();

  const QVector3D pNormal = _calculateNormal( exterior, mOriginX, mOriginY, mInvertNormals );
  const int pCount = exterior->numPoints();

  if ( pCount == 4 && polygon.numInteriorRings() == 0 )
  {
    // polygon is a triangle - write vertices to the output data array without triangulation
    QgsPoint pt;
    QgsVertexId::VertexType vt;
    for ( int i = 0; i < 3; i++ )
    {
      exterior->pointAt( i, pt, vt );
      mData << pt.x() - mOriginX << pt.z() << - pt.y() + mOriginY;
      if ( mAddNormals )
        mData << pNormal.x() << pNormal.z() << - pNormal.y();
    }

    if ( mAddBackFaces )
    {
      // the same triangle with reversed order of coordinates and inverted normal
      for ( int i = 2; i >= 0; i-- )
      {
        exterior->pointAt( i, pt, vt );
        mData << pt.x() - mOriginX << pt.z() << - pt.y() + mOriginY;
        if ( mAddNormals )
          mData << -pNormal.x() << -pNormal.z() << pNormal.y();
      }
    }
  }
  else
  {
    if ( !qgsDoubleNear( pNormal.length(), 1, 0.001 ) )
      return;  // this should not happen - pNormal should be normalized to unit length

    std::unique_ptr<QMatrix4x4> toNewBase, toOldBase;
    if ( pNormal != QVector3D( 0, 0, 1 ) )
    {
      // this is not a horizontal plane - need to reproject the polygon to a new base so that
      // we can do the triangulation in a plane

      QVector3D pXVector, pYVector;
      _normalVectorToXYVectors( pNormal, pXVector, pYVector );

      // so now we have three orthogonal unit vectors defining new base
      // let's build transform matrix. We actually need just a 3x3 matrix,
      // but Qt does not have good support for it, so using 4x4 matrix instead.
      toNewBase.reset( new QMatrix4x4(
                         pXVector.x(), pXVector.y(), pXVector.z(), 0,
                         pYVector.x(), pYVector.y(), pYVector.z(), 0,
                         pNormal.x(), pNormal.y(), pNormal.z(), 0,
                         0, 0, 0, 0 ) );

      // our 3x3 matrix is orthogonal, so for inverse we only need to transpose it
      toOldBase.reset( new QMatrix4x4( toNewBase->transposed() ) );
    }

    const QgsPoint ptStart( exterior->startPoint() );
    const QgsPoint pt0( QgsWkbTypes::PointZ, ptStart.x(), ptStart.y(), std::isnan( ptStart.z() ) ? 0 : ptStart.z() );

    // subtract ptFirst from geometry for better numerical stability in triangulation
    // and apply new 3D vector base if the polygon is not horizontal
    std::unique_ptr<QgsPolygon> polygonNew( _transform_polygon_to_new_base( polygon, pt0, toNewBase.get() ) );

    if ( _minimum_distance_between_coordinates( *polygonNew ) < 0.001 )
    {
      // when the distances between coordinates of input points are very small,
      // the triangulation likes to crash on numerical errors - when the distances are ~ 1e-5
      // Assuming that the coordinates should be in a projected CRS, we should be able
      // to simplify geometries that may cause problems and avoid possible crashes
      QgsGeometry polygonSimplified = QgsGeometry( polygonNew->clone() ).simplify( 0.001 );
      const QgsPolygon *polygonSimplifiedData = qgsgeometry_cast<const QgsPolygon *>( polygonSimplified.constGet() );
      if ( _minimum_distance_between_coordinates( *polygonSimplifiedData ) < 0.001 )
      {
        // Failed to fix that. It could be a really tiny geometry... or maybe they gave us
        // geometry in unprojected lat/lon coordinates
        QgsMessageLog::logMessage( QObject::tr( "geometry's coordinates are too close to each other and simplification failed - skipping" ), QObject::tr( "3D" ) );
        return;
      }
      else
      {
        polygonNew.reset( polygonSimplifiedData->clone() );
      }
    }

    if ( !_check_intersecting_rings( *polygonNew ) )
    {
      // skip the polygon - it would cause a crash inside poly2tri library
      QgsMessageLog::logMessage( QObject::tr( "polygon rings self-intersect or intersect each other - skipping" ), QObject::tr( "3D" ) );
      return;
    }

    QList< std::vector<p2t::Point *> > polylinesToDelete;
    QHash<p2t::Point *, float> z;

    // polygon exterior
    std::vector<p2t::Point *> polyline;
    _ringToPoly2tri( polygonNew->exteriorRing(), polyline, z );
    polylinesToDelete << polyline;

    std::unique_ptr<p2t::CDT> cdt( new p2t::CDT( polyline ) );

    // polygon holes
    for ( int i = 0; i < polygonNew->numInteriorRings(); ++i )
    {
      std::vector<p2t::Point *> holePolyline;
      const QgsCurve *hole = polygonNew->interiorRing( i );

      _ringToPoly2tri( hole, holePolyline, z );

      cdt->AddHole( holePolyline );
      polylinesToDelete << holePolyline;
    }

    // run triangulation and write vertices to the output data array
    try
    {
      cdt->Triangulate();

      std::vector<p2t::Triangle *> triangles = cdt->GetTriangles();

      for ( size_t i = 0; i < triangles.size(); ++i )
      {
        p2t::Triangle *t = triangles[i];
        for ( int j = 0; j < 3; ++j )
        {
          p2t::Point *p = t->GetPoint( j );
          QVector4D pt( p->x, p->y, z[p], 0 );
          if ( toOldBase )
            pt = *toOldBase * pt;
          const double fx = pt.x() - mOriginX + pt0.x();
          const double fy = pt.y() - mOriginY + pt0.y();
          const double fz = pt.z() + extrusionHeight + pt0.z();
          mData << fx << fz << -fy;
          if ( mAddNormals )
            mData << pNormal.x() << pNormal.z() << - pNormal.y();
        }

        if ( mAddBackFaces )
        {
          // the same triangle with reversed order of coordinates and inverted normal
          for ( int j = 2; j >= 0; --j )
          {
            p2t::Point *p = t->GetPoint( j );
            QVector4D pt( p->x, p->y, z[p], 0 );
            if ( toOldBase )
              pt = *toOldBase * pt;
            const double fx = pt.x() - mOriginX + pt0.x();
            const double fy = pt.y() - mOriginY + pt0.y();
            const double fz = pt.z() + extrusionHeight + pt0.z();
            mData << fx << fz << -fy;
            if ( mAddNormals )
              mData << -pNormal.x() << -pNormal.z() << pNormal.y();
          }
        }
      }
    }
    catch ( ... )
    {
      QgsMessageLog::logMessage( QObject::tr( "Triangulation failed. Skipping polygon…" ), QObject::tr( "3D" ) );
    }

    for ( int i = 0; i < polylinesToDelete.count(); ++i )
      qDeleteAll( polylinesToDelete[i] );
  }

  // add walls if extrusion is enabled
  if ( extrusionHeight != 0 )
  {
    _makeWalls( *exterior, false, extrusionHeight, mData, mAddNormals, mOriginX, mOriginY );

    for ( int i = 0; i < polygon.numInteriorRings(); ++i )
      _makeWalls( *polygon.interiorRing( i ), true, extrusionHeight, mData, mAddNormals, mOriginX, mOriginY );
  }
}

QgsPoint getPointFromData( QVector< float >::const_iterator &it )
{
  // tessellator geometry is x, z, -y
  double x = *it;
  ++it;
  double z = *it;
  ++it;
  double y = -( *it );
  ++it;
  return QgsPoint( x, y, z );
}

int QgsTessellator::dataVerticesCount() const
{
  return mData.size() / ( mAddNormals ? 6 : 3 );
}

std::unique_ptr<QgsMultiPolygon> QgsTessellator::asMultiPolygon() const
{
  std::unique_ptr< QgsMultiPolygon > mp = qgis::make_unique< QgsMultiPolygon >();
  const QVector<float> data = mData;
  for ( auto it = data.constBegin(); it != data.constEnd(); )
  {
    QgsPoint p1 = getPointFromData( it );
    QgsPoint p2 = getPointFromData( it );
    QgsPoint p3 = getPointFromData( it );
    mp->addGeometry( new QgsTriangle( p1, p2, p3 ) );
  }
  return mp;
}
