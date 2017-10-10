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
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgis_sip.h"

#include "poly2tri/poly2tri.h"

#include <QtDebug>
#include <QVector3D>
#include <algorithm>

static void make_quad( float x0, float y0, float x1, float y1, float zLow, float zHigh, QVector<float> &data, bool addNormals )
{
  float dx = x1 - x0;
  float dy = -( y1 - y0 );

  // perpendicular vector in plane to [x,y] is [-y,x]
  QVector3D vn( -dy, 0, dx );
  vn.normalize();

  // triangle 1
  data << x0 << zHigh << -y0;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  data << x1 << zHigh << -y1;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  data << x0 << zLow  << -y0;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();

  // triangle 2
  data << x0 << zLow  << -y0;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  data << x1 << zHigh << -y1;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  data << x1 << zLow  << -y1;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
}


QgsTessellator::QgsTessellator( double originX, double originY, bool addNormals )
  : mOriginX( originX )
  , mOriginY( originY )
  , mAddNormals( addNormals )
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
    float height = pt.z();
    // make a quad
    make_quad( x0, y0, x1, y1, height, height + extrusionHeight, data, addNormals );
    ptPrev = pt;
  }
}

static QVector3D _calculateNormal( const QgsCurve *curve, bool &hasValidZ )
{
  // Calculate the polygon's normal vector, based on Newell's method
  // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
  QgsVertexId::VertexType vt;
  QgsPoint pt1, pt2;
  QVector3D normal( 0, 0, 0 );

  // assume that Z coordinates are not present
  hasValidZ = false;

  for ( int i = 0; i < curve->numPoints() - 1; i++ )
  {
    curve->pointAt( i, pt1, vt );
    curve->pointAt( i + 1, pt2, vt );

    if ( qIsNaN( pt1.z() ) || qIsNaN( pt2.z() ) )
      continue;

    hasValidZ = true;

    normal.setX( normal.x() + ( pt1.y() - pt2.y() ) * ( pt1.z() + pt2.z() ) );
    normal.setY( normal.y() + ( pt1.z() - pt2.z() ) * ( pt1.x() + pt2.x() ) );
    normal.setZ( normal.z() + ( pt1.x() - pt2.x() ) * ( pt1.y() + pt2.y() ) );
  }

  if ( !hasValidZ )
    return QVector3D( 0, 0, 1 );

  normal.normalize();

  return normal;
}

void QgsTessellator::addPolygon( const QgsPolygonV2 &polygon, float extrusionHeight )
{
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
    QList<QgsGeometry> geomRings;
    geomRings << QgsGeometry( polygon.exteriorRing()->clone() );
    for ( int i = 0; i < polygon.numInteriorRings(); ++i )
      geomRings << QgsGeometry( polygon.interiorRing( i )->clone() );

    for ( int i = 0; i < geomRings.count(); ++i )
      for ( int j = i + 1; j < geomRings.count(); ++j )
      {
        if ( geomRings[i].intersects( geomRings[j] ) )
        {
          // skip the polygon - it would cause a crash inside poly2tri library
          qDebug() << "polygon rings intersect each other - skipping";
          return;
        }
      }
  }

  const QgsCurve *exterior = polygon.exteriorRing();

  QList< std::vector<p2t::Point *> > polylinesToDelete;
  QHash<p2t::Point *, float> z;

  std::vector<p2t::Point *> polyline;
  polyline.reserve( exterior->numPoints() );

  QgsVertexId::VertexType vt;
  QgsPoint pt;

  bool hasValidZ;
  const QVector3D pNormal = _calculateNormal( exterior, hasValidZ );
  const int pCount = exterior->numPoints();

  // Polygon is a triangle
  if ( pCount == 4 )
  {
    QgsPoint pt;
    for ( int i = 0; i < 3; i++ )
    {
      exterior->pointAt( i, pt, vt );
      mData << pt.x() - mOriginX << pt.z() << - pt.y() + mOriginY;
      if ( mAddNormals )
        mData << pNormal.x() << pNormal.z() << - pNormal.y();
    }
  }
  else
  {

    if ( !qgsDoubleNear( pNormal.length(), 1, 0.001 ) )
    {
      return;
    }

    const QgsPoint ptFirst( exterior->startPoint() );
    QVector3D pOrigin( ptFirst.x(), ptFirst.y(), qIsNaN( ptFirst.z() ) ? 0 : ptFirst.z() );
    QVector3D pXVector;
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
    const QVector3D pYVector = QVector3D::normal( pNormal, pXVector );

    for ( int i = 0; i < pCount - 1; ++i )
    {
      exterior->pointAt( i, pt, vt );
      QVector3D tempPt( pt.x(), pt.y(), ( qIsNaN( pt.z() ) ? 0 : pt.z() ) );
      const float x = QVector3D::dotProduct( tempPt - pOrigin, pXVector );
      const float y = QVector3D::dotProduct( tempPt - pOrigin, pYVector );

      const bool found = std::find_if( polyline.begin(), polyline.end(), [x, y]( p2t::Point *&p ) { return *p == p2t::Point( x, y ); } ) != polyline.end();

      if ( found )
      {
        continue;
      }

      p2t::Point *pt2 = new p2t::Point( x, y );
      polyline.push_back( pt2 );

      z[pt2] = qIsNaN( pt.z() ) ? 0 : pt.z();
    }
    polylinesToDelete << polyline;

    // TODO: robustness (no nearly duplicate points, invalid geometries ...)

    if ( polyline.size() == 3 && polygon.numInteriorRings() == 0 )
    {
      for ( std::vector<p2t::Point *>::iterator it = polyline.begin(); it != polyline.end(); it++ )
      {
        p2t::Point *p = *it;
        const double zPt = z[p];
        QVector3D nPoint = pOrigin + pXVector * p->x + pYVector * p->y;
        const double fx = nPoint.x() - mOriginX;
        const double fy = nPoint.y() - mOriginY;
        const double fz = extrusionHeight + ( qIsNaN( zPt ) ? 0 : zPt );
        mData << fx << fz << -fy;
        if ( mAddNormals )
          mData << pNormal.x() << pNormal.z() << - pNormal.y();
      }
    }
    else if ( polyline.size() >= 3 )
    {
      p2t::CDT *cdt = new p2t::CDT( polyline );

      // polygon holes
      for ( int i = 0; i < polygon.numInteriorRings(); ++i )
      {
        std::vector<p2t::Point *> holePolyline;
        holePolyline.reserve( exterior->numPoints() );
        const QgsCurve *hole = polygon.interiorRing( i );
        for ( int j = 0; j < hole->numPoints() - 1; ++j )
        {
          hole->pointAt( j, pt, vt );
          QVector3D tempPt( pt.x(), pt.y(), ( qIsNaN( pt.z() ) ? 0 : pt.z() ) );

          const float x = QVector3D::dotProduct( tempPt - pOrigin, pXVector );
          const float y = QVector3D::dotProduct( tempPt - pOrigin, pYVector );

          const bool found = std::find_if( polyline.begin(), polyline.end(), [x, y]( p2t::Point *&p ) { return *p == p2t::Point( x, y ); } ) != polyline.end();

          if ( found )
          {
            continue;
          }

          p2t::Point *pt2 = new p2t::Point( x, y );
          holePolyline.push_back( pt2 );

          z[pt2] = qIsNaN( pt.z() ) ? 0 : pt.z();
        }
        cdt->AddHole( holePolyline );
        polylinesToDelete << holePolyline;
      }
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
            float zPt = z[p];
            QVector3D nPoint = pOrigin + pXVector * p->x + pYVector * p->y;
            float fx = nPoint.x() - mOriginX;
            float fy = nPoint.y() - mOriginY;
            float fz = extrusionHeight + ( qIsNaN( zPt ) ? 0 : zPt );
            mData << fx << fz << -fy;
            if ( mAddNormals )
              mData << pNormal.x() << pNormal.z() << - pNormal.y();
          }
        }
      }
      catch ( ... )
      {
        qDebug() << "Triangulation failed. Skipping polygon...";
      }

      delete cdt;
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
