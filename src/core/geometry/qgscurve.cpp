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
#include "qgsgeos.h"
#include "qgsvertexid.h"

bool QgsCurve::operator==( const QgsAbstractGeometry &other ) const
{
  const QgsCurve *otherCurve = qgsgeometry_cast< const QgsCurve * >( &other );
  if ( !otherCurve )
    return false;

  return equals( *otherCurve );
}

bool QgsCurve::operator!=( const QgsAbstractGeometry &other ) const
{
  return !operator==( other );
}

bool QgsCurve::isClosed2D() const
{
  if ( numPoints() == 0 )
    return false;

  //don't consider M-coordinates when testing closedness
  const QgsPoint start = startPoint();
  const QgsPoint end = endPoint();

  return qgsDoubleNear( start.x(), end.x() ) &&
         qgsDoubleNear( start.y(), end.y() );
}
bool QgsCurve::isClosed() const
{
  bool closed = isClosed2D();
  if ( is3D() && closed )
  {
    const QgsPoint start = startPoint();
    const QgsPoint end = endPoint();
    closed &= qgsDoubleNear( start.z(), end.z() ) || ( std::isnan( start.z() ) && std::isnan( end.z() ) );
  }
  return closed;
}

bool QgsCurve::isRing() const
{
  return ( isClosed() && numPoints() >= 4 );
}

QPainterPath QgsCurve::asQPainterPath() const
{
  QPainterPath p;
  addToPainterPath( p );
  return p;
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
  const int n = numPoints();
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
  multiPoint->reserve( 2 );
  multiPoint->addGeometry( new QgsPoint( startPoint() ) );
  multiPoint->addGeometry( new QgsPoint( endPoint() ) );
  return multiPoint;
}

QString QgsCurve::asKml( int precision ) const
{
  std::unique_ptr<QgsLineString> lineString( curveToLine() );
  if ( !lineString )
  {
    return QString();
  }
  QString kml = lineString->asKml( precision );
  return kml;
}

QgsCurve *QgsCurve::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  return curveToLine( tolerance, toleranceType );
}

int QgsCurve::vertexCount( int part, int ring ) const
{
  Q_UNUSED( part )
  Q_UNUSED( ring )
  return numPoints();
}

int QgsCurve::ringCount( int part ) const
{
  Q_UNUSED( part )
  return numPoints() > 0 ? 1 : 0;
}

int QgsCurve::partCount() const
{
  return numPoints() > 0 ? 1 : 0;
}

QgsPoint QgsCurve::vertexAt( QgsVertexId id ) const
{
  QgsPoint v;
  Qgis::VertexType type;
  pointAt( id.vertex, v, type );
  return v;
}

QgsCurve *QgsCurve::toCurveType() const
{
  return clone();
}

void QgsCurve::normalize()
{
  if ( isEmpty() )
    return;

  if ( !isClosed() )
  {
    return;
  }

  int minCoordinateIndex = 0;
  QgsPoint minCoord;
  int i = 0;
  for ( auto it = vertices_begin(); it != vertices_end(); ++it )
  {
    const QgsPoint vertex = *it;
    if ( minCoord.isEmpty() || minCoord.compareTo( &vertex ) > 0 )
    {
      minCoord = vertex;
      minCoordinateIndex = i;
    }
    i++;
  }

  scroll( minCoordinateIndex );
}

QgsRectangle QgsCurve::boundingBox() const
{
  if ( mBoundingBox.isNull() )
  {
    mBoundingBox = calculateBoundingBox();
  }
  return mBoundingBox;
}

bool QgsCurve::isValid( QString &error, Qgis::GeometryValidityFlags flags ) const
{
  if ( flags == 0 && mHasCachedValidity )
  {
    // use cached validity results
    error = mValidityFailureReason;
    return error.isEmpty();
  }

  const QgsGeos geos( this );
  const bool res = geos.isValid( &error, flags & Qgis::GeometryValidityFlag::AllowSelfTouchingHoles, nullptr );
  if ( flags == 0 )
  {
    mValidityFailureReason = !res ? error : QString();
    mHasCachedValidity = true;
  }
  return res;
}

QPolygonF QgsCurve::asQPolygonF() const
{
  std::unique_ptr< QgsLineString > segmentized( curveToLine() );
  return segmentized->asQPolygonF();
}

double QgsCurve::straightDistance2d() const
{
  return startPoint().distance( endPoint() );
}

double QgsCurve::sinuosity() const
{
  const double d = straightDistance2d();
  if ( qgsDoubleNear( d, 0.0 ) )
    return std::numeric_limits<double>::quiet_NaN();

  return length() / d;
}

Qgis::AngularDirection QgsCurve::orientation() const
{
  double a = 0;
  sumUpArea( a );
  return a < 0 ? Qgis::AngularDirection::Clockwise : Qgis::AngularDirection::CounterClockwise;
}

void QgsCurve::clearCache() const
{
  mBoundingBox = QgsRectangle();
  mHasCachedValidity = false;
  mValidityFailureReason.clear();
  QgsAbstractGeometry::clearCache();
}

int QgsCurve::childCount() const
{
  return numPoints();
}

QgsPoint QgsCurve::childPoint( int index ) const
{
  QgsPoint point;
  Qgis::VertexType type;
  const bool res = pointAt( index, point, type );
  Q_ASSERT( res );
  Q_UNUSED( res )
  return point;
}

bool QgsCurve::snapToGridPrivate( double hSpacing, double vSpacing, double dSpacing, double mSpacing,
                                  const QVector<double> &srcX, const QVector<double> &srcY, const QVector<double> &srcZ, const QVector<double> &srcM,
                                  QVector<double> &outX, QVector<double> &outY, QVector<double> &outZ, QVector<double> &outM ) const
{
  const int length = numPoints();

  if ( length <= 0 )
    return false;

  const bool hasZ = is3D();
  const bool hasM = isMeasure();

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
  const QgsWkbTypes::Type pointType = QgsWkbTypes::zmType( QgsWkbTypes::Point, hasZ, hasM );
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
