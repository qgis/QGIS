/***************************************************************************
                        qgsabstractgeometryv2.cpp
  -------------------------------------------------------------------
Date                 : 04 Sept 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsabstractgeometryv2.h"
#include "qgswkbptr.h"
#include "qgsgeos.h"
#include "qgsmaptopixel.h"
#include "qgspointv2.h"

#include <limits>
#include <QTransform>

QgsAbstractGeometryV2::QgsAbstractGeometryV2(): mWkbType( QgsWkbTypes::Unknown )
{
}

QgsAbstractGeometryV2::~QgsAbstractGeometryV2()
{
}

QgsAbstractGeometryV2::QgsAbstractGeometryV2( const QgsAbstractGeometryV2& geom )
{
  mWkbType = geom.mWkbType;
}

QgsAbstractGeometryV2& QgsAbstractGeometryV2::operator=( const QgsAbstractGeometryV2 & geom )
{
  if ( &geom != this )
  {
    clear();
    mWkbType = geom.mWkbType;
  }
  return *this;
}

bool QgsAbstractGeometryV2::is3D() const
{
  return QgsWkbTypes::hasZ( mWkbType );
}

bool QgsAbstractGeometryV2::isMeasure() const
{
  return QgsWkbTypes::hasM( mWkbType );
}

#if 0
void QgsAbstractGeometryV2::clip( const QgsRectangle& rect )
{
  // TODO
  // - Implementation
  // - API doc in header

  // Don't insert Q_UNUSED, so we have a warning that reminds us of this TODO
}
#endif

void QgsAbstractGeometryV2::setZMTypeFromSubGeometry( const QgsAbstractGeometryV2* subgeom, QgsWkbTypes::Type baseGeomType )
{
  if ( !subgeom )
  {
    return;
  }

  //special handling for 25d types:
  if ( baseGeomType == QgsWkbTypes::LineString &&
       ( subgeom->wkbType() == QgsWkbTypes::Point25D || subgeom->wkbType() == QgsWkbTypes::LineString25D ) )
  {
    mWkbType = QgsWkbTypes::LineString25D;
    return;
  }
  else if ( baseGeomType == QgsWkbTypes::Polygon &&
            ( subgeom->wkbType() == QgsWkbTypes::Point25D || subgeom->wkbType() == QgsWkbTypes::LineString25D ) )
  {
    mWkbType = QgsWkbTypes::Polygon25D;
    return;
  }

  bool hasZ = subgeom->is3D();
  bool hasM = subgeom->isMeasure();

  if ( hasZ && hasM )
  {
    mWkbType = QgsWkbTypes::addM( QgsWkbTypes::addZ( baseGeomType ) );
  }
  else if ( hasZ )
  {
    mWkbType = QgsWkbTypes::addZ( baseGeomType );
  }
  else if ( hasM )
  {
    mWkbType =  QgsWkbTypes::addM( baseGeomType );
  }
  else
  {
    mWkbType = baseGeomType;
  }
}

QgsRectangle QgsAbstractGeometryV2::calculateBoundingBox() const
{
  double xmin = std::numeric_limits<double>::max();
  double ymin = std::numeric_limits<double>::max();
  double xmax = -std::numeric_limits<double>::max();
  double ymax = -std::numeric_limits<double>::max();

  QgsVertexId id;
  QgsPointV2 vertex;
  double x, y;
  while ( nextVertex( id, vertex ) )
  {
    x = vertex.x();
    y = vertex.y();
    if ( x < xmin )
      xmin = x;
    if ( x > xmax )
      xmax = x;
    if ( y < ymin )
      ymin = y;
    if ( y > ymax )
      ymax = y;
  }

  return QgsRectangle( xmin, ymin, xmax, ymax );
}

int QgsAbstractGeometryV2::nCoordinates() const
{
  int nCoords = 0;

  Q_FOREACH ( const QgsRingSequenceV2 &r, coordinateSequence() )
  {
    Q_FOREACH ( const QgsPointSequenceV2 &p, r )
    {
      nCoords += p.size();
    }
  }

  return nCoords;
}

QString QgsAbstractGeometryV2::wktTypeStr() const
{
  QString wkt = geometryType();
  if ( is3D() )
    wkt += 'Z';
  if ( isMeasure() )
    wkt += 'M';
  return wkt;
}

QgsPointV2 QgsAbstractGeometryV2::centroid() const
{
  // http://en.wikipedia.org/wiki/Centroid#Centroid_of_polygon
  // Pick the first ring of first part for the moment

  int n = vertexCount( 0, 0 );
  if ( n == 1 )
  {
    return vertexAt( QgsVertexId( 0, 0, 0 ) );
  }

  double A = 0.;
  double Cx = 0.;
  double Cy = 0.;
  QgsPointV2 v0 = vertexAt( QgsVertexId( 0, 0, 0 ) );
  int i = 0, j = 1;
  if ( vertexAt( QgsVertexId( 0, 0, 0 ) ) != vertexAt( QgsVertexId( 0, 0, n - 1 ) ) )
  {
    i = n - 1;
    j = 0;
  }
  for ( ; j < n; i = j++ )
  {
    QgsPointV2 vi = vertexAt( QgsVertexId( 0, 0, i ) );
    QgsPointV2 vj = vertexAt( QgsVertexId( 0, 0, j ) );
    vi.rx() -= v0.x();
    vi.ry() -= v0.y();
    vj.rx() -= v0.x();
    vj.ry() -= v0.y();
    double d = vi.x() * vj.y() - vj.x() * vi.y();
    A += d;
    Cx += ( vi.x() + vj.x() ) * d;
    Cy += ( vi.y() + vj.y() ) * d;
  }

  if ( A < 1E-12 )
  {
    Cx = Cy = 0.;
    for ( int i = 0; i < n - 1; ++i )
    {
      QgsPointV2 vi = vertexAt( QgsVertexId( 0, 0, i ) );
      Cx += vi.x();
      Cy += vi.y();
    }
    return QgsPointV2( Cx / ( n - 1 ), Cy / ( n - 1 ) );
  }
  else
  {
    return QgsPointV2( v0.x() + Cx / ( 3. * A ), v0.y() + Cy / ( 3. * A ) );
  }
}

bool QgsAbstractGeometryV2::convertTo( QgsWkbTypes::Type type )
{
  if ( type == mWkbType )
    return true;

  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::flatType( mWkbType ) )
    return false;

  bool needZ = QgsWkbTypes::hasZ( type );
  bool needM = QgsWkbTypes::hasM( type );
  if ( !needZ )
  {
    dropZValue();
  }
  else if ( !is3D() )
  {
    addZValue();
  }

  if ( !needM )
  {
    dropMValue();
  }
  else if ( !isMeasure() )
  {
    addMValue();
  }

  return true;
}

bool QgsAbstractGeometryV2::isEmpty() const
{
  QgsVertexId vId;
  QgsPointV2 vertex;
  return !nextVertex( vId, vertex );
}


QgsAbstractGeometryV2* QgsAbstractGeometryV2::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  Q_UNUSED( tolerance );
  Q_UNUSED( toleranceType );
  return clone();
}

