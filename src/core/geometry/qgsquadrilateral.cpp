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

QgsQuadrilateral QgsQuadrilateral::rectangleFrom3points( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, ConstructionOption mode )
{
  QgsQuadrilateral rect;
  double azimuth = p1.azimuth( p2 ) + 90.0 * QgsGeometryUtils::leftOfLine( p3.x(), p3.y(), p1.x(), p1.y(), p2.x(), p2.y() );
  switch ( mode )
  {
    case Distance:
    {
      double inclination = 90.0;
      double distance = 0;

      if ( p2.is3D() && p3.is3D() )
      {
        inclination = p2.inclination( p3 );
        distance = p2.distance3D( p3 );
      }
      else
      {
        distance = p2.distance( p3 );
      }

      rect.setPoints( p1, p2, p2.project( distance, azimuth, inclination ), p1.project( distance, azimuth, inclination ) );
      break;
    }
    case Projected:
    {
      QgsVector3D v3 = QgsVector3D::perpendicularPoint( QgsVector3D( p1.x(), p1.y(), p1.z() ),
                       QgsVector3D( p2.x(), p2.y(), p2.z() ),
                       QgsVector3D( p3.x(), p3.y(), p3.z() ) );
      QgsPoint point3( v3.x(), v3.y(), v3.z() );
      double inclination = p3.inclination( point3 );
      double distance = p3.distance3D( point3 );

      rect.setPoints( p1, p2, p2.project( distance, azimuth, inclination ), p1.project( distance, azimuth, inclination ) );
      break;
    }
  }

  return rect;

}

QgsQuadrilateral QgsQuadrilateral::rectangleFromExtent( const QgsPoint &p1, const QgsPoint &p2 )
{
  double z = p1.z();

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

  return QgsQuadrilateral( QgsPoint( xMin, yMin, z ),
                           QgsPoint( xMin, yMax, z ),
                           QgsPoint( xMax, yMax, z ),
                           QgsPoint( xMax, yMin, z ) );
}

QgsQuadrilateral QgsQuadrilateral::squareFromDiagonal( const QgsPoint &p1, const QgsPoint &p2 )
{
  QgsPoint point2, point3 = QgsPoint( p2.x(), p2.y() ), point4;

  double azimuth = p1.azimuth( p2 ) + 90.0;
  double distance = p1.distance( p2 ) / 2.0;
  QgsPoint midPoint = QgsGeometryUtils::midpoint( p1, p2 );

  point2 = midPoint.project( -distance, azimuth );
  point4 = midPoint.project( distance, azimuth );

  if ( p1.is3D() )
  {
    double z = 0;
    z = p1.z();
    point3.addZValue( z );
    point2.addZValue( z );
    point4.addZValue( z );
  }

  return QgsQuadrilateral( p1, point2, point3, point4 );

}

QgsQuadrilateral QgsQuadrilateral::rectangleFromCenterPoint( const QgsPoint &center, const QgsPoint &point )
{

  double xOffset = std::fabs( point.x() - center.x() );
  double yOffset = std::fabs( point.y() - center.y() );

  return QgsQuadrilateral( QgsPoint( center.x() - xOffset, center.y() - yOffset, center.z() ),
                           QgsPoint( center.x() - xOffset, center.y() + yOffset, center.z() ),
                           QgsPoint( center.x() + xOffset, center.y() + yOffset, center.z() ),
                           QgsPoint( center.x() + xOffset, center.y() - yOffset, center.z() ) );

}

QgsQuadrilateral QgsQuadrilateral::fromRectangle( const QgsRectangle &rectangle )
{
  return QgsQuadrilateral(
           QgsPoint( rectangle.xMaximum(), rectangle.yMinimum() ),
           QgsPoint( rectangle.xMinimum(), rectangle.yMaximum() ),
           QgsPoint( rectangle.xMaximum(), rectangle.yMaximum() ),
           QgsPoint( rectangle.xMaximum(), rectangle.yMinimum() )
         );
}

bool QgsQuadrilateral::operator==( const QgsQuadrilateral &other ) const
{
  if ( !isValid() || !other.isValid() )
  {
    return false;
  }
  return ( ( mPoint1 == other.mPoint1 ) &&
           ( mPoint2 == other.mPoint2 ) &&
           ( mPoint3 == other.mPoint3 ) &&
           ( mPoint4 == other.mPoint4 )
         );
}

bool QgsQuadrilateral::operator!=( const QgsQuadrilateral &other ) const
{
  return !operator==( other );
}

// Returns true is segments are not self-intersected
//
// p3    p1
// | \  /|
// |  \/ |
// |  /\ |
// | /  \|
// p2    p4

static bool isNotAntiParallelogram( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  QgsPoint inter;
  bool isIntersection = false;
  QgsGeometryUtils::segmentIntersection( p1, p2, p3, p4, inter, isIntersection );

  return !isIntersection;
}

static bool isNotCollinear( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  bool isCollinear =
    (
      ( QgsGeometryUtils::segmentSide( p1, p2, p3 ) == 0 ) ||
      ( QgsGeometryUtils::segmentSide( p2, p3, p4 ) == 0 ) ||
      ( QgsGeometryUtils::segmentSide( p3, p4, p1 ) == 0 ) ||
      ( QgsGeometryUtils::segmentSide( p3, p4, p2 ) == 0 )
    );

  return !isCollinear;

}

// Convenient method to validate inputs
static bool validate( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  return (
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
  std::unique_ptr<QgsPolygon> polygon = qgis::make_unique< QgsPolygon >();
  if ( !isValid() )
  {
    return polygon.release();
  }

  polygon->setExteriorRing( toLineString( force2D ) );

  return polygon.release();
}

QgsLineString *QgsQuadrilateral::toLineString( bool force2D ) const
{
  std::unique_ptr<QgsLineString> ext = qgis::make_unique< QgsLineString>();
  if ( !isValid() )
  {
    return ext.release();
  }

  QgsPointSequence pts;
  pts = points();

  ext->setPoints( pts );

  if ( force2D )
    ext->dropZValue();

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
