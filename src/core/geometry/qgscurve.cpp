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

QgsBox3D QgsCurve::boundingBox3D() const
{
  if ( mBoundingBox.isNull() )
  {
    mBoundingBox = calculateBoundingBox3D();
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

  const QgsGeos geos( this, 0, Qgis::GeosCreationFlags() );
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
  mBoundingBox = QgsBox3D();
  mHasCachedValidity = false;
  mValidityFailureReason.clear();
  mHasCachedSummedUpArea = false;
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
                                  QVector<double> &outX, QVector<double> &outY, QVector<double> &outZ, QVector<double> &outM, bool removeRedundantPoints ) const
{
  const int length = numPoints();
  if ( length < 2 )
    return false;

  const bool hasZ = is3D();
  const bool hasM = isMeasure();

  outX.reserve( length );
  outY.reserve( length );
  if ( hasZ )
    outZ.reserve( length );
  if ( hasM )
    outM.reserve( length );

  const double *xIn = srcX.constData();
  const double *yIn = srcY.constData();
  const double *zIn = hasZ ? srcZ.constData() : nullptr;
  const double *mIn = hasM ? srcM.constData() : nullptr;

  double previousX = 0;
  double previousY = 0;
  double previousZ = 0;
  double previousM = 0;
  int outSize = 0;
  for ( int i = 0; i < length; ++i )
  {
    const double currentX = *xIn++;
    const double currentY = *yIn++;
    const double currentZ = zIn ? *zIn++ : 0;
    const double currentM = mIn ? *mIn++ : 0;

    const double roundedX = hSpacing > 0 ? ( std::round( currentX / hSpacing ) * hSpacing ) : currentX;
    const double roundedY = vSpacing > 0 ? ( std::round( currentY / vSpacing ) * vSpacing ) : currentY;
    const double roundedZ = hasZ && dSpacing > 0 ? ( std::round( currentZ / dSpacing ) * dSpacing ) : currentZ;
    const double roundedM = hasM && mSpacing > 0 ? ( std::round( currentM / mSpacing ) * mSpacing ) : currentM;

    if ( i == 0 )
    {
      outX.append( roundedX );
      outY.append( roundedY );
      if ( hasZ )
        outZ.append( roundedZ );
      if ( hasM )
        outM.append( roundedM );
      outSize++;
    }
    else
    {
      const bool isPointEqual = qgsDoubleNear( roundedX, previousX )
                                && qgsDoubleNear( roundedY, previousY )
                                && ( !hasZ || dSpacing <= 0 || qgsDoubleNear( roundedZ, previousZ ) )
                                && ( !hasM || mSpacing <= 0 || qgsDoubleNear( roundedM, previousM ) );
      if ( isPointEqual )
        continue;

      // maybe previous point is redundant and is just a midpoint on a straight line -- let's check
      bool previousPointRedundant = false;
      if ( removeRedundantPoints && outSize > 1 && !hasZ && !hasM )
      {
        previousPointRedundant = QgsGeometryUtilsBase::leftOfLine( outX.at( outSize - 1 ),
                                 outY.at( outSize - 1 ),
                                 outX.at( outSize - 2 ),
                                 outY.at( outSize - 2 ),
                                 roundedX, roundedY ) == 0;
      }
      if ( previousPointRedundant )
      {
        outX[ outSize - 1 ] = roundedX;
        outY[ outSize - 1 ] = roundedY;
      }
      else
      {
        outX.append( roundedX );
        outY.append( roundedY );
        if ( hasZ )
          outZ.append( roundedZ );
        if ( hasM )
          outM.append( roundedM );
        outSize++;
      }
    }

    previousX = roundedX;
    previousY = roundedY;
    previousZ = roundedZ;
    previousM = roundedM;
  }

  if ( removeRedundantPoints && isClosed() && outSize > 4 && !hasZ && !hasM )
  {
    // maybe first/last vertex is redundant, let's try to remove that too
    const bool firstVertexIsRedundant = QgsGeometryUtilsBase::leftOfLine( outX.at( 0 ),
                                        outY.at( 0 ),
                                        outX.at( outSize - 2 ),
                                        outY.at( outSize - 2 ),
                                        outX.at( 1 ), outY.at( 1 ) ) == 0;
    if ( firstVertexIsRedundant )
    {
      outX.removeAt( 0 );
      outY.removeAt( 0 );
      outX[ outSize - 2 ] = outX.at( 0 );
      outY[ outSize - 2 ] = outY.at( 0 );
    }
  }

  // we previously reserved size based on a worst case scenario, let's free
  // unnecessary memory reservation now
  outX.squeeze();
  outY.squeeze();
  if ( hasZ )
    outZ.squeeze();
  if ( hasM )
    outM.squeeze();

  return outSize >= 4 || ( !isClosed() && outSize >= 2 );
}
