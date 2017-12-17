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

#include <memory>

#include "qgscurve.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgsmultipoint.h"

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
  QgsCoordinateSequence sequence;
  sequence.append( QgsRingSequence() );
  sequence.back().append( QgsPointSequence() );
  points( sequence.back().back() );

  return sequence;
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

void QgsCurve::adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex, QgsVertexId &nextVertex ) const
{
  int n = numPoints();
  if ( vertex.vertex < 0 || vertex.vertex >= n )
  {
    previousVertex = QgsVertexId();
    nextVertex = QgsVertexId();
    return;
  }

  if ( vertex.vertex == 0 )
  {
    previousVertex = QgsVertexId();
  }
  else
  {
    previousVertex = QgsVertexId( vertex.part, vertex.ring, vertex.vertex - 1 );
  }
  if ( vertex.vertex == n - 1 )
  {
    nextVertex = QgsVertexId();
  }
  else
  {
    nextVertex = QgsVertexId( vertex.part, vertex.ring, vertex.vertex + 1 );
  }
}

int QgsCurve::vertexNumberFromVertexId( QgsVertexId id ) const
{
  if ( id.part != 0 || id.ring != 0 )
    return -1;
  if ( id.vertex < 0 || id.vertex >= numPoints() )
    return -1;
  return id.vertex;
}

QgsAbstractGeometry *QgsCurve::boundary() const
{
  if ( isEmpty() )
    return nullptr;

  if ( isClosed() )
    return nullptr;

  QgsMultiPoint *multiPoint = new QgsMultiPoint();
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

QgsCurve *QgsCurve::toCurveType() const
{
  return clone();
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
  QgsAbstractGeometry::clearCache();
}

int QgsCurve::childCount() const
{
  return numPoints();
}

QgsPoint QgsCurve::childPoint( int index ) const
{
  QgsPoint point;
  QgsVertexId::VertexType type;
  bool res = pointAt( index, point, type );
  Q_ASSERT( res );
  Q_UNUSED( res );
  return point;
}

bool QgsCurve::snapToGridPrivate( double hSpacing, double vSpacing, double dSpacing, double mSpacing,
                                  const QVector<double> &srcX, const QVector<double> &srcY, const QVector<double> &srcZ, const QVector<double> &srcM,
                                  QVector<double> &outX, QVector<double> &outY, QVector<double> &outZ, QVector<double> &outM ) const
{
  int length = numPoints();

  if ( length <= 0 )
    return false;

  bool hasZ = is3D();
  bool hasM = isMeasure();

  // helper functions
  auto roundVertex = [hSpacing, vSpacing, dSpacing, mSpacing, hasZ, hasM, &srcX, &srcY, &srcZ, &srcM]( QgsPoint & out, int i )
  {
    if ( hSpacing > 0 )
      out.setX( std::round( srcX.at( i ) / hSpacing ) * hSpacing );
    else
      out.setX( srcX.at( i ) );

    if ( vSpacing > 0 )
      out.setY( std::round( srcY.at( i ) / vSpacing ) * vSpacing );
    else
      out.setY( srcY.at( i ) );

    if ( hasZ )
    {
      if ( dSpacing > 0 )
        out.setZ( std::round( srcZ.at( i ) / dSpacing ) * dSpacing );
      else
        out.setZ( srcZ.at( i ) );
    }

    if ( hasM )
    {
      if ( mSpacing > 0 )
        out.setM( std::round( srcM.at( i ) / mSpacing ) * mSpacing );
      else
        out.setM( srcM.at( i ) );
    }
  };


  auto append = [hasZ, hasM, &outX, &outY, &outM, &outZ]( QgsPoint const & point )
  {
    outX.append( point.x() );

    outY.append( point.y() );

    if ( hasZ )
      outZ.append( point.z() );

    if ( hasM )
      outM.append( point.m() );
  };

  auto isPointEqual = [dSpacing, mSpacing, hasZ, hasM]( const QgsPoint & a, const QgsPoint & b )
  {
    return ( a.x() == b.x() )
           && ( a.y() == b.y() )
           && ( !hasZ || dSpacing <= 0 || a.z() == b.z() )
           && ( !hasM || mSpacing <= 0 || a.m() == b.m() );
  };

  // temporary values
  QgsWkbTypes::Type pointType = QgsWkbTypes::zmType( QgsWkbTypes::Point, hasZ, hasM );
  QgsPoint last( pointType );
  QgsPoint current( pointType );

  // Actual code (what does all the work)
  roundVertex( last, 0 );
  append( last );

  for ( int i = 1; i < length; ++i )
  {
    roundVertex( current, i );
    if ( !isPointEqual( current, last ) )
    {
      append( current );
      last = current;
    }
  }

  // if it's not closed, with 2 points you get a correct line
  // if it is, you need at least 4 (3 + the vertex that closes)
  if ( outX.length() < 2 || ( isClosed() && outX.length() < 4 ) )
    return false;

  return true;
}
