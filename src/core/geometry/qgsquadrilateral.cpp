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

QgsQuadrilateral::QgsQuadrilateral()
{

}

QgsQuadrilateral::QgsQuadrilateral( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 ):
  mPoint1( p1 ),
  mPoint2( p2 ),
  mPoint3( p3 ),
  mPoint4( p4 )
{

}

QgsQuadrilateral::QgsQuadrilateral( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3, const QgsPointXY &p4 ):
  mPoint1( QgsPoint( p1 ) ),
  mPoint2( QgsPoint( p2 ) ),
  mPoint3( QgsPoint( p3 ) ),
  mPoint4( QgsPoint( p4 ) )
{

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

  double xOffset = fabs( point.x() - center.x() );
  double yOffset = fabs( point.y() - center.y() );

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
  if ( isEmpty() && other.isEmpty() )
  {
    return true;
  }
  else if ( isEmpty() || other.isEmpty() )
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

bool QgsQuadrilateral::isEmpty() const
{
  return ( ( mPoint1 == QgsPoint() ) &&
           ( mPoint2 == QgsPoint() ) &&
           ( mPoint3 == QgsPoint() ) &&
           ( mPoint4 == QgsPoint() )
         );
}

void QgsQuadrilateral::setPoint( const QgsPoint &newPoint, Point index )
{
  switch ( index )
  {
    case Point1:
      mPoint1 = newPoint;
      break;
    case Point2:
      mPoint2 = newPoint;
      break;
    case Point3:
      mPoint3 = newPoint;
      break;
    case Point4:
      mPoint4 = newPoint;
      break;
  }
}

void QgsQuadrilateral::setPoints( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, const QgsPoint &p4 )
{
  mPoint1 = p1;
  mPoint2 = p2;
  mPoint3 = p3;
  mPoint4 = p4;
}

QgsPointSequence QgsQuadrilateral::points() const
{
  QgsPointSequence pts;

  pts << mPoint1 << mPoint2 << mPoint3 << mPoint4 << mPoint1;

  return pts;
}

QgsPolygon *QgsQuadrilateral::toPolygon( bool force2D ) const
{
  std::unique_ptr<QgsPolygon> polygon( new QgsPolygon() );
  if ( isEmpty() )
  {
    return polygon.release();
  }

  polygon->setExteriorRing( toLineString( force2D ) );

  return polygon.release();
}

QgsLineString *QgsQuadrilateral::toLineString( bool force2D ) const
{
  std::unique_ptr<QgsLineString> ext( new QgsLineString() );
  if ( isEmpty() )
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
  if ( isEmpty() )
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
