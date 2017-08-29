/***************************************************************************
                         qgscurve.cpp
                         --------------
    begin                : November 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscurve.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgsmultipoint.h"

QgsCurve::QgsCurve(): QgsAbstractGeometry()
{}

bool QgsCurve::isClosed() const
{
  if ( numPoints() == 0 )
    return false;

  //don't consider M-coordinates when testing closedness
  QgsPoint start = startPoint();
  QgsPoint end = endPoint();

  bool closed = qgsDoubleNear( start.x(), end.x(), 1E-8 ) &&
                qgsDoubleNear( start.y(), end.y(), 1E-8 );
  if ( is3D() && closed )
    closed &= qgsDoubleNear( start.z(), end.z(), 1E-8 ) || ( std::isnan( start.z() ) && std::isnan( end.z() ) );
  return closed;
}

bool QgsCurve::isRing() const
{
  return ( isClosed() && numPoints() >= 4 );
}

QgsCoordinateSequence QgsCurve::coordinateSequence() const
{
  if ( !mCoordinateSequence.isEmpty() )
    return mCoordinateSequence;

  mCoordinateSequence.append( QgsRingSequence() );
  mCoordinateSequence.back().append( QgsPointSequence() );
  points( mCoordinateSequence.back().back() );

  return mCoordinateSequence;
}

bool QgsCurve::nextVertex( QgsVertexId &id, QgsPoint &vertex ) const
{
  if ( id.vertex < 0 )
  {
    id.vertex = 0;
    if ( id.part < 0 )
    {
      id.part = 0;
    }
    if ( id.ring < 0 )
    {
      id.ring = 0;
    }
  }
  else
  {
    if ( id.vertex + 1 >= numPoints() )
    {
      return false;
    }
    ++id.vertex;
  }
  return pointAt( id.vertex, vertex, id.type );
}

QgsAbstractGeometry *QgsCurve::boundary() const
{
  if ( isEmpty() )
    return nullptr;

  if ( isClosed() )
    return nullptr;

  QgsMultiPointV2 *multiPoint = new QgsMultiPointV2();
  multiPoint->addGeometry( new QgsPoint( startPoint() ) );
  multiPoint->addGeometry( new QgsPoint( endPoint() ) );
  return multiPoint;
}

QgsCurve *QgsCurve::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  return curveToLine( tolerance, toleranceType );
}

int QgsCurve::vertexCount( int part, int ring ) const
{
  Q_UNUSED( part );
  Q_UNUSED( ring );
  return numPoints();
}

int QgsCurve::ringCount( int part ) const
{
  Q_UNUSED( part );
  return numPoints() > 0 ? 1 : 0;
}

int QgsCurve::partCount() const
{
  return numPoints() > 0 ? 1 : 0;
}

QgsPoint QgsCurve::vertexAt( QgsVertexId id ) const
{
  QgsPoint v;
  QgsVertexId::VertexType type;
  pointAt( id.vertex, v, type );
  return v;
}

QgsRectangle QgsCurve::boundingBox() const
{
  if ( mBoundingBox.isNull() )
  {
    mBoundingBox = calculateBoundingBox();
  }
  return mBoundingBox;
}

QPolygonF QgsCurve::asQPolygonF() const
{
  const int nb = numPoints();
  QPolygonF points;
  points.reserve( nb );
  for ( int i = 0; i < nb; ++i )
  {
    points << QPointF( xAt( i ), yAt( i ) );
  }
  return points;
}

void QgsCurve::clearCache() const
{
  mBoundingBox = QgsRectangle();
  mCoordinateSequence.clear();
  QgsAbstractGeometry::clearCache();
}

