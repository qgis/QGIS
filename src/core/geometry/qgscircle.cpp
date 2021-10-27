/***************************************************************************
                         qgscircle.cpp
                         --------------
    begin                : March 2017
    copyright            : (C) 2017 by Loîc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscircle.h"
#include "qgslinestring.h"
#include "qgsgeometryutils.h"
#include "qgstriangle.h"

#include <memory>
#include <utility>

QgsCircle::QgsCircle() :
  QgsEllipse( QgsPoint(), 0.0, 0.0, 0.0 )
{

}

QgsCircle::QgsCircle( const QgsPoint &center, double radius, double azimuth ) :
  QgsEllipse( center, radius, radius, azimuth )
{

}

QgsCircle QgsCircle::from2Points( const QgsPoint &pt1, const QgsPoint &pt2 )
{
  QgsPoint center = QgsGeometryUtils::midpoint( pt1, pt2 );
  const double azimuth = QgsGeometryUtils::lineAngle( pt1.x(), pt1.y(), pt2.x(), pt2.y() ) * 180.0 / M_PI;
  const double radius = pt1.distance( pt2 ) / 2.0;

  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << pt1 << pt2, center );

  return QgsCircle( center, radius, azimuth );
}

static bool isPerpendicular( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double epsilon )
{
  // check the given point are perpendicular to x or y axis

  const double yDelta_a = pt2.y() - pt1.y();
  const double xDelta_a = pt2.x() - pt1.x();
  const double yDelta_b = pt3.y() - pt2.y();
  const double xDelta_b = pt3.x() - pt2.x();

  if ( ( std::fabs( xDelta_a ) <= epsilon ) && ( std::fabs( yDelta_b ) <= epsilon ) )
  {
    return false;
  }

  if ( std::fabs( yDelta_a ) <= epsilon )
  {
    return true;
  }
  else if ( std::fabs( yDelta_b ) <= epsilon )
  {
    return true;
  }
  else if ( std::fabs( xDelta_a ) <= epsilon )
  {
    return true;
  }
  else if ( std::fabs( xDelta_b ) <= epsilon )
  {
    return true;
  }

  return false;

}

QgsCircle QgsCircle::from3Points( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double epsilon )
{
  QgsPoint p1, p2, p3;

  if ( !isPerpendicular( pt1, pt2, pt3, epsilon ) )
  {
    p1 = pt1;
    p2 = pt2;
    p3 = pt3;
  }
  else if ( !isPerpendicular( pt1, pt3, pt2, epsilon ) )
  {
    p1 = pt1;
    p2 = pt3;
    p3 = pt2;
  }
  else if ( !isPerpendicular( pt2, pt1, pt3, epsilon ) )
  {
    p1 = pt2;
    p2 = pt1;
    p3 = pt3;
  }
  else if ( !isPerpendicular( pt2, pt3, pt1, epsilon ) )
  {
    p1 = pt2;
    p2 = pt3;
    p3 = pt1;
  }
  else if ( !isPerpendicular( pt3, pt2, pt1, epsilon ) )
  {
    p1 = pt3;
    p2 = pt2;
    p3 = pt1;
  }
  else if ( !isPerpendicular( pt3, pt1, pt2, epsilon ) )
  {
    p1 = pt3;
    p2 = pt1;
    p3 = pt2;
  }
  else
  {
    return QgsCircle();
  }
  QgsPoint center = QgsPoint();
  double radius = -0.0;
  // Paul Bourke's algorithm
  const double yDelta_a = p2.y() - p1.y();
  const double xDelta_a = p2.x() - p1.x();
  const double yDelta_b = p3.y() - p2.y();
  const double xDelta_b = p3.x() - p2.x();

  if ( qgsDoubleNear( xDelta_a, 0.0, epsilon ) || qgsDoubleNear( xDelta_b, 0.0, epsilon ) )
  {
    return QgsCircle();
  }

  const double aSlope = yDelta_a / xDelta_a;
  const double bSlope = yDelta_b / xDelta_b;

  // set z and m coordinate for center
  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << p1 << p2 << p3, center );

  if ( ( std::fabs( xDelta_a ) <= epsilon ) && ( std::fabs( yDelta_b ) <= epsilon ) )
  {
    center.setX( 0.5 * ( p2.x() + p3.x() ) );
    center.setY( 0.5 * ( p1.y() + p2.y() ) );
    radius = center.distance( pt1 );

    return QgsCircle( center, radius );
  }

  if ( std::fabs( aSlope - bSlope ) <= epsilon )
  {
    return QgsCircle();
  }

  center.setX(
    ( aSlope * bSlope * ( p1.y() - p3.y() ) +
      bSlope * ( p1.x() + p2.x() ) -
      aSlope * ( p2.x() + p3.x() ) ) /
    ( 2.0 * ( bSlope - aSlope ) )
  );
  center.setY(
    -1.0 * ( center.x() - ( p1.x() + p2.x() ) / 2.0 ) /
    aSlope + ( p1.y() + p2.y() ) / 2.0
  );

  radius = center.distance( p1 );

  return QgsCircle( center, radius );
}

QgsCircle QgsCircle::fromCenterDiameter( const QgsPoint &center, double diameter, double azimuth )
{
  return QgsCircle( center, diameter / 2.0, azimuth );
}

QgsCircle QgsCircle::fromCenterPoint( const QgsPoint &center, const QgsPoint &pt1 )
{
  const double azimuth = QgsGeometryUtils::lineAngle( center.x(), center.y(), pt1.x(), pt1.y() ) * 180.0 / M_PI;

  QgsPoint centerPt( center );
  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << center << pt1, centerPt );

  return QgsCircle( centerPt, centerPt.distance( pt1 ), azimuth );
}

static QVector<QgsCircle> from2ParallelsLine( const QgsPoint &pt1_par1, const QgsPoint &pt2_par1, const QgsPoint &pt1_par2, const QgsPoint &pt2_par2, const QgsPoint &pt1_line1, const QgsPoint &pt2_line1, const QgsPoint &pos, double epsilon )
{
  const double radius = QgsGeometryUtils::perpendicularSegment( pt1_par2, pt1_par1, pt2_par1 ).length() / 2.0;

  bool isInter;
  const QgsPoint ptInter;
  QVector<QgsCircle> circles;

  QgsPoint ptInter_par1line1, ptInter_par2line1;
  double angle1, angle2;
  double x, y;
  QgsGeometryUtils::angleBisector( pt1_par1.x(), pt1_par1.y(), pt2_par1.x(), pt2_par1.y(), pt1_line1.x(), pt1_line1.y(), pt2_line1.x(), pt2_line1.y(), x, y, angle1 );
  ptInter_par1line1.setX( x );
  ptInter_par1line1.setY( y );

  QgsGeometryUtils::angleBisector( pt1_par2.x(), pt1_par2.y(), pt2_par2.x(), pt2_par2.y(), pt1_line1.x(), pt1_line1.y(), pt2_line1.x(), pt2_line1.y(), x, y, angle2 );
  ptInter_par2line1.setX( x );
  ptInter_par2line1.setY( y );

  QgsPoint center;
  QgsGeometryUtils::segmentIntersection( ptInter_par1line1, ptInter_par1line1.project( 1.0, angle1 ), ptInter_par2line1, ptInter_par2line1.project( 1.0, angle2 ), center, isInter, epsilon, true );
  if ( isInter )
  {
    if ( !pos.isEmpty() )
    {
      if ( QgsGeometryUtils::leftOfLine( center, pt1_line1, pt2_line1 ) == QgsGeometryUtils::leftOfLine( pos, pt1_line1, pt2_line1 ) )
      {
        circles.append( QgsCircle( center, radius ) );
      }
    }
    else
    {
      circles.append( QgsCircle( center, radius ) );
    }
  }

  QgsGeometryUtils::segmentIntersection( ptInter_par1line1, ptInter_par1line1.project( 1.0, angle1 ), ptInter_par2line1, ptInter_par2line1.project( 1.0, angle2 + 90.0 ), center, isInter, epsilon, true );
  if ( isInter )
  {
    if ( !pos.isEmpty() )
    {
      if ( QgsGeometryUtils::leftOfLine( center, pt1_line1, pt2_line1 ) == QgsGeometryUtils::leftOfLine( pos, pt1_line1, pt2_line1 ) )
      {
        circles.append( QgsCircle( center, radius ) );
      }
    }
    else
    {
      circles.append( QgsCircle( center, radius ) );
    }
  }

  QgsGeometryUtils::segmentIntersection( ptInter_par1line1, ptInter_par1line1.project( 1.0, angle1 + 90.0 ), ptInter_par2line1, ptInter_par2line1.project( 1.0, angle2 ), center, isInter, epsilon, true );
  if ( isInter && !circles.contains( QgsCircle( center, radius ) ) )
  {
    if ( !pos.isEmpty() )
    {
      if ( QgsGeometryUtils::leftOfLine( center, pt1_line1, pt2_line1 ) == QgsGeometryUtils::leftOfLine( pos, pt1_line1, pt2_line1 ) )
      {
        circles.append( QgsCircle( center, radius ) );
      }
    }
    else
    {
      circles.append( QgsCircle( center, radius ) );
    }
  }
  QgsGeometryUtils::segmentIntersection( ptInter_par1line1, ptInter_par1line1.project( 1.0, angle1 + 90.0 ), ptInter_par2line1, ptInter_par2line1.project( 1.0, angle2 + 90.0 ), center, isInter, epsilon, true );
  if ( isInter && !circles.contains( QgsCircle( center, radius ) ) )
  {
    if ( !pos.isEmpty() )
    {
      if ( QgsGeometryUtils::leftOfLine( center, pt1_line1, pt2_line1 ) == QgsGeometryUtils::leftOfLine( pos, pt1_line1, pt2_line1 ) )
      {
        circles.append( QgsCircle( center, radius ) );
      }
    }
    else
    {
      circles.append( QgsCircle( center, radius ) );
    }
  }

  return circles;
}

QVector<QgsCircle> QgsCircle::from3TangentsMulti( const QgsPoint &pt1_tg1, const QgsPoint &pt2_tg1, const QgsPoint &pt1_tg2, const QgsPoint &pt2_tg2, const QgsPoint &pt1_tg3, const QgsPoint &pt2_tg3, double epsilon, const QgsPoint &pos )
{
  QgsPoint p1, p2, p3;
  bool isIntersect_tg1tg2 = false;
  bool isIntersect_tg1tg3 = false;
  bool isIntersect_tg2tg3 = false;
  QgsGeometryUtils::segmentIntersection( pt1_tg1, pt2_tg1, pt1_tg2, pt2_tg2, p1, isIntersect_tg1tg2, epsilon );
  QgsGeometryUtils::segmentIntersection( pt1_tg1, pt2_tg1, pt1_tg3, pt2_tg3, p2, isIntersect_tg1tg3, epsilon );
  QgsGeometryUtils::segmentIntersection( pt1_tg2, pt2_tg2, pt1_tg3, pt2_tg3, p3, isIntersect_tg2tg3, epsilon );

  QVector<QgsCircle> circles;
  if ( !isIntersect_tg1tg2 && !isIntersect_tg2tg3 ) // three lines are parallels
    return circles;

  if ( !isIntersect_tg1tg2 )
    return from2ParallelsLine( pt1_tg1, pt2_tg1, pt1_tg2, pt2_tg2, pt1_tg3, pt2_tg3, pos, epsilon );
  else if ( !isIntersect_tg1tg3 )
    return from2ParallelsLine( pt1_tg1, pt2_tg1, pt1_tg3, pt2_tg3, pt1_tg2, pt2_tg2, pos, epsilon );
  else if ( !isIntersect_tg2tg3 )
    return from2ParallelsLine( pt1_tg2, pt2_tg2, pt1_tg3, pt2_tg3, pt1_tg1, pt1_tg1, pos, epsilon );

  if ( p1.is3D() )
  {
    p1.convertTo( QgsWkbTypes::dropZ( p1.wkbType() ) );
  }

  if ( p2.is3D() )
  {
    p2.convertTo( QgsWkbTypes::dropZ( p2.wkbType() ) );
  }

  if ( p3.is3D() )
  {
    p3.convertTo( QgsWkbTypes::dropZ( p3.wkbType() ) );
  }

  circles.append( QgsTriangle( p1, p2, p3 ).inscribedCircle() );
  return circles;
}

QgsCircle QgsCircle::from3Tangents( const QgsPoint &pt1_tg1, const QgsPoint &pt2_tg1, const QgsPoint &pt1_tg2, const QgsPoint &pt2_tg2, const QgsPoint &pt1_tg3, const QgsPoint &pt2_tg3, double epsilon, const QgsPoint &pos )
{
  const QVector<QgsCircle> circles = from3TangentsMulti( pt1_tg1, pt2_tg1, pt1_tg2, pt2_tg2, pt1_tg3, pt2_tg3, epsilon, pos );
  if ( circles.length() != 1 )
    return QgsCircle();
  return circles.at( 0 );
}

QgsCircle QgsCircle::minimalCircleFrom3Points( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double epsilon )
{
  const double l1 = pt2.distance( pt3 );
  const double l2 = pt3.distance( pt1 );
  const double l3 = pt1.distance( pt2 );

  if ( ( l1 * l1 ) - ( l2 * l2 + l3 * l3 ) >= epsilon )
    return QgsCircle().from2Points( pt2, pt3 );
  else if ( ( l2 * l2 ) - ( l1 * l1 + l3 * l3 ) >= epsilon )
    return QgsCircle().from2Points( pt3, pt1 );
  else if ( ( l3 * l3 ) - ( l1 * l1 + l2 * l2 ) >= epsilon )
    return QgsCircle().from2Points( pt1, pt2 );
  else
    return QgsCircle().from3Points( pt1, pt2, pt3, epsilon );
}

int QgsCircle::intersections( const QgsCircle &other, QgsPoint &intersection1, QgsPoint &intersection2, bool useZ ) const
{
  if ( useZ && mCenter.is3D() && other.center().is3D() && !qgsDoubleNear( mCenter.z(), other.center().z() ) )
    return 0;

  QgsPointXY int1, int2;

  const int res = QgsGeometryUtils::circleCircleIntersections( QgsPointXY( mCenter ), radius(),
                  QgsPointXY( other.center() ), other.radius(),
                  int1, int2 );
  if ( res == 0 )
    return 0;

  intersection1 = QgsPoint( int1 );
  intersection2 = QgsPoint( int2 );
  if ( useZ && mCenter.is3D() )
  {
    intersection1.addZValue( mCenter.z() );
    intersection2.addZValue( mCenter.z() );
  }
  return res;
}

bool QgsCircle::tangentToPoint( const QgsPointXY &p, QgsPointXY &pt1, QgsPointXY &pt2 ) const
{
  return QgsGeometryUtils::tangentPointAndCircle( QgsPointXY( mCenter ), radius(), p, pt1, pt2 );
}

int QgsCircle::outerTangents( const QgsCircle &other, QgsPointXY &line1P1, QgsPointXY &line1P2, QgsPointXY &line2P1, QgsPointXY &line2P2 ) const
{
  return QgsGeometryUtils::circleCircleOuterTangents( QgsPointXY( mCenter ), radius(),
         QgsPointXY( other.center() ), other.radius(), line1P1, line1P2, line2P1, line2P2 );
}

int QgsCircle::innerTangents( const QgsCircle &other, QgsPointXY &line1P1, QgsPointXY &line1P2, QgsPointXY &line2P1, QgsPointXY &line2P2 ) const
{
  return QgsGeometryUtils::circleCircleInnerTangents( QgsPointXY( mCenter ), radius(),
         QgsPointXY( other.center() ), other.radius(), line1P1, line1P2, line2P1, line2P2 );
}

QgsCircle QgsCircle::fromExtent( const QgsPoint &pt1, const QgsPoint &pt2 )
{
  const double delta_x = std::fabs( pt1.x() - pt2.x() );
  const double delta_y = std::fabs( pt1.x() - pt2.y() );
  if ( !qgsDoubleNear( delta_x, delta_y ) )
  {
    return QgsCircle();
  }

  QgsPoint center = QgsGeometryUtils::midpoint( pt1, pt2 );
  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << pt1 << pt2, center );

  return QgsCircle( center, delta_x / 2.0, 0 );
}

double QgsCircle::area() const
{
  return M_PI * mSemiMajorAxis * mSemiMajorAxis;
}

double QgsCircle::perimeter() const
{
  return 2.0 * M_PI * mSemiMajorAxis;
}

void QgsCircle::setSemiMajorAxis( const double semiMajorAxis )
{
  mSemiMajorAxis = std::fabs( semiMajorAxis );
  mSemiMinorAxis = mSemiMajorAxis;
}

void QgsCircle::setSemiMinorAxis( const double semiMinorAxis )
{
  mSemiMajorAxis = std::fabs( semiMinorAxis );
  mSemiMinorAxis = mSemiMajorAxis;
}

QVector<QgsPoint> QgsCircle::northQuadrant() const
{
  QVector<QgsPoint> quad;
  quad.append( QgsPoint( mCenter.x(), mCenter.y() + mSemiMajorAxis, mCenter.z(), mCenter.m() ) );
  quad.append( QgsPoint( mCenter.x() + mSemiMajorAxis, mCenter.y(), mCenter.z(), mCenter.m() ) );
  quad.append( QgsPoint( mCenter.x(), mCenter.y() - mSemiMajorAxis, mCenter.z(), mCenter.m() ) );
  quad.append( QgsPoint( mCenter.x() - mSemiMajorAxis, mCenter.y(), mCenter.z(), mCenter.m() ) );

  return quad;
}

QgsCircularString *QgsCircle::toCircularString( bool oriented ) const
{
  std::unique_ptr<QgsCircularString> circString( new QgsCircularString() );
  QgsPointSequence points;
  QVector<QgsPoint> quad;
  if ( oriented )
  {
    quad = quadrant();
  }
  else
  {
    quad = northQuadrant();
  }
  quad.append( quad.at( 0 ) );
  for ( QVector<QgsPoint>::const_iterator it = quad.constBegin(); it != quad.constEnd(); ++it )
  {
    points.append( *it );
  }
  circString->setPoints( points );

  return circString.release();
}

bool QgsCircle::contains( const QgsPoint &point, double epsilon ) const
{
  return ( mCenter.distance( point ) <= mSemiMajorAxis + epsilon );
}

QgsRectangle QgsCircle::boundingBox() const
{
  return QgsRectangle( mCenter.x() - mSemiMajorAxis, mCenter.y() - mSemiMajorAxis, mCenter.x() + mSemiMajorAxis, mCenter.y() + mSemiMajorAxis );
}

QString QgsCircle::toString( int pointPrecision, int radiusPrecision, int azimuthPrecision ) const
{
  QString rep;
  if ( isEmpty() )
    rep = QStringLiteral( "Empty" );
  else
    rep = QStringLiteral( "Circle (Center: %1, Radius: %2, Azimuth: %3)" )
          .arg( mCenter.asWkt( pointPrecision ), 0, 's' )
          .arg( qgsDoubleToString( mSemiMajorAxis, radiusPrecision ), 0, 'f' )
          .arg( qgsDoubleToString( mAzimuth, azimuthPrecision ), 0, 'f' );

  return rep;

}

QDomElement QgsCircle::asGml2( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  // Gml2 does not support curve. It will be converted to a linestring via CircularString
  std::unique_ptr< QgsCircularString > circularString( toCircularString() );
  const QDomElement gml = circularString->asGml2( doc, precision, ns, axisOrder );
  return gml;
}

QDomElement QgsCircle::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QgsPointSequence pts;
  pts << northQuadrant().at( 0 ) << northQuadrant().at( 1 ) << northQuadrant().at( 2 );

  QDomElement elemCircle = doc.createElementNS( ns, QStringLiteral( "Circle" ) );

  if ( isEmpty() )
    return elemCircle;

  elemCircle.appendChild( QgsGeometryUtils::pointsToGML3( pts, doc, precision, ns, mCenter.is3D(), axisOrder ) );
  return elemCircle;
}
