/***************************************************************************
                         qgsquadrilateral.cpp
                         -------------------
    begin                : November 2018
    copyright            : (C) 2018 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquadrilateral.h"
#include "qgsgeometryutils.h"

QgsQuadrilateral::QgsQuadrilateral() = default;

QgsQuadrilateral::QgsQuadrilateral( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  setPoints( p1, p2, p3, p4 );
}

QgsQuadrilateral::QgsQuadrilateral( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3, const QgsPointXY &p4 )
{
  setPoints( QgsPoint( p1 ), QgsPoint( p2 ), QgsPoint( p3 ), QgsPoint( p4 ) );
}

QgsQuadrilateral QgsQuadrilateral::rectangleFrom3Points( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, ConstructionOption mode )
{
  QgsWkbTypes::Type pType( QgsWkbTypes::Point );

  double z = std::numeric_limits< double >::quiet_NaN();
  double m = std::numeric_limits< double >::quiet_NaN();

  // We don't need the m value right away, it will only be inserted at the end.
  if ( p1.isMeasure() )
    m = p1.m();
  if ( p2.isMeasure() && std::isnan( m ) )
    m = p2.m();
  if ( p3.isMeasure() && std::isnan( m ) )
    m = p3.m();

  if ( p1.is3D() )
    z = p1.z();
  if ( p2.is3D() && std::isnan( z ) )
    z = p2.z();
  if ( p3.is3D() && std::isnan( z ) )
    z = p3.z();
  if ( !std::isnan( z ) )
  {
    pType = QgsWkbTypes::addZ( pType );
  }
  else
  {
    // This is only necessary to facilitate the calculation of the perpendicular
    // point with QgsVector3D.
    if ( mode == Projected )
      z = 0;
  }
  const QgsPoint point1( pType, p1.x(), p1.y(), std::isnan( p1.z() ) ? z : p1.z() );
  const QgsPoint point2( pType, p2.x(), p2.y(), std::isnan( p2.z() ) ? z : p2.z() );
  const QgsPoint point3( pType, p3.x(), p3.y(), std::isnan( p3.z() ) ? z : p3.z() );

  QgsQuadrilateral rect;
  double inclination = 90.0;
  double distance = 0;
  const double azimuth = point1.azimuth( point2 ) + 90.0 * QgsGeometryUtils::leftOfLine( point3.x(), point3.y(), point1.x(), point1.y(), point2.x(), point2.y() );
  switch ( mode )
  {
    case Distance:
    {
      if ( point2.is3D() && point3.is3D() )
      {
        inclination = point2.inclination( point3 );
        distance = point2.distance3D( point3 );
      }
      else
      {
        distance = point2.distance( point3 );
      }

      break;
    }
    case Projected:
    {
      const QgsVector3D v3 = QgsVector3D::perpendicularPoint( QgsVector3D( point1.x(), point1.y(), std::isnan( point1.z() ) ? z : point1.z() ),
                             QgsVector3D( point2.x(), point2.y(), std::isnan( point2.z() ) ? z : point2.z() ),
                             QgsVector3D( point3.x(), point3.y(), std::isnan( point3.z() ) ? z : point3.z() ) );
      const QgsPoint pV3( pType, v3.x(), v3.y(), v3.z() );
      if ( p3.is3D() )
      {
        inclination = pV3.inclination( p3 );
        distance = p3.distance3D( pV3 );
      }
      else
        distance = p3.distance( pV3 );

      break;
    }
  }

  // Final points
  QgsPoint fp1 = point1;
  QgsPoint fp2 = point2;
  QgsPoint fp3 = point2.project( distance, azimuth, inclination );
  QgsPoint fp4 = point1.project( distance, azimuth, inclination ) ;

  if ( pType != QgsWkbTypes::PointZ )
  {
    fp1.dropZValue();
    fp2.dropZValue();
    fp3.dropZValue();
    fp4.dropZValue();
  }

  if ( !std::isnan( m ) )
  {
    fp1.addMValue( m );
    fp2.addMValue( m );
    fp3.addMValue( m );
    fp4.addMValue( m );
  }

  rect.setPoints( fp1, fp2, fp3, fp4 );
  return rect;

}

QgsQuadrilateral QgsQuadrilateral::rectangleFromExtent( const QgsPoint &p1, const QgsPoint &p2 )
{
  if ( QgsPoint( p1.x(), p1.y() ) == QgsPoint( p2.x(), p2.y() ) )
    return QgsQuadrilateral();

  QgsQuadrilateral quad;
  const double z = p1.z();
  const double m = p1.m();

  double xMin = 0, xMax = 0, yMin = 0, yMax = 0;

  if ( p1.x() < p2.x() )
  {
    xMin = p1.x();
    xMax = p2.x();
  }
  else
  {

    xMin = p2.x();
    xMax = p1.x();
  }

  if ( p1.y() < p2.y() )
  {
    yMin = p1.y();
    yMax = p2.y();
  }
  else
  {

    yMin = p2.y();
    yMax = p1.y();
  }

  quad.setPoints( QgsPoint( p1.wkbType(), xMin, yMin, z, m ),
                  QgsPoint( p1.wkbType(), xMin, yMax, z, m ),
                  QgsPoint( p1.wkbType(), xMax, yMax, z, m ),
                  QgsPoint( p1.wkbType(), xMax, yMin, z, m ) );

  return quad;
}

QgsQuadrilateral QgsQuadrilateral::squareFromDiagonal( const QgsPoint &p1, const QgsPoint &p2 )
{

  if ( QgsPoint( p1.x(), p1.y() ) == QgsPoint( p2.x(), p2.y() ) )
    return QgsQuadrilateral();

  const double z = p1.z();
  const double m = p1.m();

  QgsQuadrilateral quad;
  QgsPoint point2, point3 = QgsPoint( p2.x(), p2.y() ), point4;

  const double azimuth = p1.azimuth( point3 ) + 90.0;
  const double distance = p1.distance( point3 ) / 2.0;
  const QgsPoint midPoint = QgsGeometryUtils::midpoint( p1, point3 );

  point2 = midPoint.project( -distance, azimuth );
  point4 = midPoint.project( distance, azimuth );

  // add z and m, could be NaN
  point2 = QgsPoint( p1.wkbType(), point2.x(), point2.y(), z, m );
  point3 = QgsPoint( p1.wkbType(), point3.x(), point3.y(), z, m );
  point4 = QgsPoint( p1.wkbType(), point4.x(), point4.y(), z, m );

  quad.setPoints( p1, point2, point3, point4 );

  return quad;
}

QgsQuadrilateral QgsQuadrilateral::rectangleFromCenterPoint( const QgsPoint &center, const QgsPoint &point )
{
  if ( QgsPoint( center.x(), center.y() ) == QgsPoint( point.x(), point.y() ) )
    return QgsQuadrilateral();
  const double xOffset = std::fabs( point.x() - center.x() );
  const double yOffset = std::fabs( point.y() - center.y() );

  return QgsQuadrilateral( QgsPoint( center.wkbType(), center.x() - xOffset, center.y() - yOffset, center.z(), center.m() ),
                           QgsPoint( center.wkbType(), center.x() - xOffset, center.y() + yOffset, center.z(), center.m() ),
                           QgsPoint( center.wkbType(), center.x() + xOffset, center.y() + yOffset, center.z(), center.m() ),
                           QgsPoint( center.wkbType(), center.x() + xOffset, center.y() - yOffset, center.z(), center.m() ) );
}

QgsQuadrilateral QgsQuadrilateral::fromRectangle( const QgsRectangle &rectangle )
{
  QgsQuadrilateral quad;
  quad.setPoints(
    QgsPoint( rectangle.xMinimum(), rectangle.yMinimum() ),
    QgsPoint( rectangle.xMinimum(), rectangle.yMaximum() ),
    QgsPoint( rectangle.xMaximum(), rectangle.yMaximum() ),
    QgsPoint( rectangle.xMaximum(), rectangle.yMinimum() )
  );
  return quad;
}

// Convenient method for comparison
// TODO: should be have a equals method for QgsPoint allowing tolerance.
static bool equalPoint( const QgsPoint &p1, const QgsPoint &p2, double epsilon )
{
  bool equal = true;
  equal &= qgsDoubleNear( p1.x(), p2.x(), epsilon );
  equal &= qgsDoubleNear( p1.y(), p2.y(), epsilon );
  if ( p1.is3D() || p2.is3D() )
    equal &= qgsDoubleNear( p1.z(), p2.z(), epsilon ) || ( std::isnan( p1.z() ) && std::isnan( p2.z() ) );
  if ( p1.isMeasure() || p2.isMeasure() )
    equal &= qgsDoubleNear( p1.m(), p2.m(), epsilon ) || ( std::isnan( p1.m() ) && std::isnan( p2.m() ) );

  return equal;
}

bool QgsQuadrilateral::equals( const QgsQuadrilateral &other, double epsilon ) const
{
  if ( !( isValid() || other.isValid() ) )
  {
    return true;
  }
  else if ( !isValid() || !other.isValid() )
  {
    return false;
  }
  return ( ( equalPoint( mPoint1, other.mPoint1, epsilon ) ) &&
           ( equalPoint( mPoint2, other.mPoint2, epsilon ) ) &&
           ( equalPoint( mPoint3, other.mPoint3, epsilon ) ) &&
           ( equalPoint( mPoint4, other.mPoint4, epsilon ) ) );
}

bool QgsQuadrilateral::operator==( const QgsQuadrilateral &other ) const
{
  return equals( other );
}

bool QgsQuadrilateral::operator!=( const QgsQuadrilateral &other ) const
{
  return !operator==( other );
}

// Returns true if segments are not self-intersected ( [2-3] / [4-1] or [1-2] /
// [3-4] )
//
// p3    p1      p1    p3
// | \  /|       | \  /|
// |  \/ |       |  \/ |
// |  /\ |   or  |  /\ |
// | /  \|       | /  \|
// p2    p4      p2    p4

static bool isNotAntiParallelogram( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  QgsPoint inter;
  bool isIntersection1234 = QgsGeometryUtils::segmentIntersection( p1, p2, p3, p4, inter, isIntersection1234 );
  bool isIntersection2341 = QgsGeometryUtils::segmentIntersection( p2, p3, p4, p1, inter, isIntersection2341 );

  return !( isIntersection1234 || isIntersection2341 );
}

static bool isNotCollinear( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  const bool isCollinear =
    (
      ( QgsGeometryUtils::segmentSide( p1, p2, p3 ) == 0 ) ||
      ( QgsGeometryUtils::segmentSide( p1, p2, p4 ) == 0 ) ||
      ( QgsGeometryUtils::segmentSide( p1, p3, p4 ) == 0 ) ||
      ( QgsGeometryUtils::segmentSide( p2, p3, p4 ) == 0 )
    );


  return !isCollinear;
}

static bool notHaveDoublePoints( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  const bool doublePoints =
    (
      ( p1 == p2 ) || ( p1 == p3 ) || ( p1 == p4 ) || ( p2 == p3 ) || ( p2 == p4 ) || ( p3 == p4 ) );

  return !doublePoints;
}

static bool haveSameType( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  const bool sameType = !( ( p1.wkbType() != p2.wkbType() ) || ( p1.wkbType() != p3.wkbType() ) || ( p1.wkbType() != p4.wkbType() ) );
  return sameType;
}
// Convenient method to validate inputs
static bool validate( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  return (
           haveSameType( p1, p2, p3, p4 ) &&
           notHaveDoublePoints( p1, p2, p3, p4 ) &&
           isNotAntiParallelogram( p1, p2, p3, p4 ) &&
           isNotCollinear( p1, p2, p3, p4 )
         );
}

bool QgsQuadrilateral::isValid() const
{
  return validate( mPoint1, mPoint2, mPoint3, mPoint4 );
}

bool QgsQuadrilateral::setPoint( const QgsPoint &newPoint, Point index )
{
  switch ( index )
  {
    case Point1:
      if ( validate( newPoint, mPoint2, mPoint3, mPoint4 ) == false )
        return false;
      mPoint1 = newPoint;
      break;
    case Point2:
      if ( validate( mPoint1, newPoint, mPoint3, mPoint4 ) == false )
        return false;
      mPoint2 = newPoint;
      break;
    case Point3:
      if ( validate( mPoint1, mPoint2, newPoint, mPoint4 ) == false )
        return false;
      mPoint3 = newPoint;
      break;
    case Point4:
      if ( validate( mPoint1, mPoint2, mPoint3, newPoint ) == false )
        return false;
      mPoint4 = newPoint;
      break;
  }

  return true;
}

bool QgsQuadrilateral::setPoints( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  if ( validate( p1, p2, p3, p4 ) == false )
    return false;

  mPoint1 = p1;
  mPoint2 = p2;
  mPoint3 = p3;
  mPoint4 = p4;

  return true;
}

QgsPointSequence QgsQuadrilateral::points() const
{
  QgsPointSequence pts;

  pts << mPoint1 << mPoint2 << mPoint3 << mPoint4 << mPoint1;

  return pts;
}

QgsPolygon *QgsQuadrilateral::toPolygon( bool force2D ) const
{
  std::unique_ptr<QgsPolygon> polygon = std::make_unique< QgsPolygon >();
  if ( !isValid() )
  {
    return polygon.release();
  }

  polygon->setExteriorRing( toLineString( force2D ) );

  return polygon.release();
}

QgsLineString *QgsQuadrilateral::toLineString( bool force2D ) const
{
  std::unique_ptr<QgsLineString> ext = std::make_unique< QgsLineString>();
  if ( !isValid() )
  {
    return ext.release();
  }

  QgsPointSequence pts;
  pts = points();

  ext->setPoints( pts );

  if ( force2D )
    ext->dropZValue();

  if ( force2D )
    ext->dropMValue();

  return ext.release();
}

QString QgsQuadrilateral::toString( int pointPrecision ) const
{
  QString rep;
  if ( !isValid() )
    rep = QStringLiteral( "Empty" );
  else
    rep = QStringLiteral( "Quadrilateral (Point 1: %1, Point 2: %2, Point 3: %3, Point 4: %4)" )
          .arg( mPoint1.asWkt( pointPrecision ), 0, 's' )
          .arg( mPoint2.asWkt( pointPrecision ), 0, 's' )
          .arg( mPoint3.asWkt( pointPrecision ), 0, 's' )
          .arg( mPoint4.asWkt( pointPrecision ), 0, 's' );

  return rep;
}

double QgsQuadrilateral::area() const
{
  return toPolygon()->area();
}

double QgsQuadrilateral::perimeter() const
{
  return toPolygon()->perimeter();
}
