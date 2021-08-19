/***************************************************************************
                         qgsellipse.cpp
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

#include "qgsunittypes.h"
#include "qgslinestring.h"
#include "qgsellipse.h"
#include "qgsgeometryutils.h"

#include <memory>
#include <limits>

void QgsEllipse::normalizeAxis()
{
  mSemiMajorAxis = std::fabs( mSemiMajorAxis );
  mSemiMinorAxis = std::fabs( mSemiMinorAxis );
  if ( mSemiMajorAxis < mSemiMinorAxis )
  {
    std::swap( mSemiMajorAxis, mSemiMinorAxis );
    mAzimuth = 180.0 / M_PI *
               QgsGeometryUtils::normalizedAngle( M_PI / 180.0 * ( mAzimuth + 90 ) );
  }
}

QgsEllipse::QgsEllipse( const QgsPoint &center, const double axis_a, const double axis_b, const double azimuth )
  : mCenter( center )
  , mSemiMajorAxis( axis_a )
  , mSemiMinorAxis( axis_b )
  , mAzimuth( azimuth )
{
  normalizeAxis();
}

QgsEllipse QgsEllipse::fromFoci( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3 )
{
  const double dist_p1p2 = pt1.distance( pt2 );
  const double dist_p1p3 = pt1.distance( pt3 );
  const double dist_p2p3 = pt2.distance( pt3 );

  const double dist = dist_p1p3 + dist_p2p3;
  const double azimuth = 180.0 / M_PI * QgsGeometryUtils::lineAngle( pt1.x(), pt1.y(), pt2.x(), pt2.y() );
  QgsPoint center = QgsGeometryUtils::midpoint( pt1, pt2 );

  const double axis_a = dist / 2.0;
  const double axis_b = std::sqrt( std::pow( axis_a, 2.0 ) - std::pow( dist_p1p2 / 2.0, 2.0 ) );

  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << pt1 << pt2 << pt3, center );

  return QgsEllipse( center, axis_a, axis_b, azimuth );
}

QgsEllipse QgsEllipse::fromExtent( const QgsPoint &pt1, const QgsPoint &pt2 )
{
  QgsPoint center = QgsGeometryUtils::midpoint( pt1, pt2 );
  const double axis_a = std::fabs( pt2.x() - pt1.x() ) / 2.0;
  const double axis_b = std::fabs( pt2.y() - pt1.y() ) / 2.0;
  const double azimuth = 90.0;

  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << pt1 << pt2, center );

  return QgsEllipse( center, axis_a, axis_b, azimuth );
}

QgsEllipse QgsEllipse::fromCenterPoint( const QgsPoint &center, const QgsPoint &pt1 )
{
  const double axis_a = std::fabs( pt1.x() - center.x() );
  const double axis_b = std::fabs( pt1.y() - center.y() );
  const double azimuth = 90.0;

  QgsPoint centerPt( center );
  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << center << pt1, centerPt );

  return QgsEllipse( centerPt, axis_a, axis_b, azimuth );
}

QgsEllipse QgsEllipse::fromCenter2Points( const QgsPoint &center, const QgsPoint &pt1, const QgsPoint &pt2 )
{
  const double azimuth = 180.0 / M_PI * QgsGeometryUtils::lineAngle( center.x(), center.y(), pt1.x(), pt1.y() );
  const double axis_a = center.distance( pt1 );

  const double length = pt2.distance( QgsGeometryUtils::projectPointOnSegment( pt2, center, pt1 ) );
  const QgsPoint pp = center.project( length, 90 + azimuth );
  const double axis_b = center.distance( pp );

  QgsPoint centerPt( center );
  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << center << pt1 << pt2, centerPt );

  return QgsEllipse( centerPt, axis_a, axis_b, azimuth );
}

bool QgsEllipse::operator ==( const QgsEllipse &elp ) const
{
  return ( ( mCenter == elp.mCenter ) &&
           qgsDoubleNear( mSemiMajorAxis, elp.mSemiMajorAxis, 1E-8 ) &&
           qgsDoubleNear( mSemiMinorAxis, elp.mSemiMinorAxis, 1E-8 ) &&
           qgsDoubleNear( mAzimuth, elp.mAzimuth, 1E-8 )
         );
}

bool QgsEllipse::operator !=( const QgsEllipse &elp ) const
{
  return !operator==( elp );
}

bool QgsEllipse::isEmpty() const
{
  return ( qgsDoubleNear( mSemiMajorAxis, 0.0, 1E-8 ) ||
           qgsDoubleNear( mSemiMinorAxis, 0.0, 1E-8 ) );
}

void QgsEllipse::setSemiMajorAxis( const double axis_a )
{
  mSemiMajorAxis = axis_a;
  normalizeAxis();
}
void QgsEllipse::setSemiMinorAxis( const double axis_b )
{
  mSemiMinorAxis = axis_b;
  normalizeAxis();
}

void QgsEllipse::setAzimuth( const double azimuth )
{
  mAzimuth = 180.0 / M_PI *
             QgsGeometryUtils::normalizedAngle( M_PI / 180.0 * azimuth );
}

double QgsEllipse::focusDistance() const
{
  return std::sqrt( mSemiMajorAxis * mSemiMajorAxis - mSemiMinorAxis * mSemiMinorAxis );
}

QVector<QgsPoint> QgsEllipse::foci() const
{
  QVector<QgsPoint> f;
  const double dist_focus = focusDistance();
  f.append( mCenter.project( dist_focus, mAzimuth ) );
  f.append( mCenter.project( -dist_focus, mAzimuth ) );

  return f;
}

double QgsEllipse::eccentricity() const
{
  if ( isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return focusDistance() / mSemiMajorAxis;
}

double QgsEllipse::area() const
{
  return M_PI * mSemiMajorAxis * mSemiMinorAxis;
}

double QgsEllipse::perimeter() const
{
  const double a = mSemiMajorAxis;
  const double b = mSemiMinorAxis;
  return M_PI * ( 3 * ( a + b ) - std::sqrt( 10 * a * b + 3 * ( a * a + b * b ) ) );
}

QVector<QgsPoint> QgsEllipse::quadrant() const
{
  QVector<QgsPoint> quad;
  quad.append( mCenter.project( mSemiMajorAxis, mAzimuth ) );
  quad.append( mCenter.project( mSemiMinorAxis, mAzimuth + 90 ) );
  quad.append( mCenter.project( -mSemiMajorAxis, mAzimuth ) );
  quad.append( mCenter.project( -mSemiMinorAxis, mAzimuth + 90 ) );

  return quad;
}

QgsPointSequence QgsEllipse::points( unsigned int segments ) const
{
  QgsPointSequence pts;

  if ( segments < 3 )
  {
    return pts;
  }


  const QgsWkbTypes::Type pType( mCenter.wkbType() );
  const double z = mCenter.z();
  const double m = mCenter.m();

  QVector<double> t;
  t.reserve( segments );
  const double azimuth = std::atan2( quadrant().at( 0 ).y() - mCenter.y(), quadrant().at( 0 ).x() - mCenter.x() );
  for ( unsigned int i = 0; i < segments; ++i )
  {
    t.append( 2 * M_PI - ( ( 2 * M_PI ) / segments * i ) ); // Since the algorithm used rotates in the trigonometric direction (counterclockwise)
  }

  for ( QVector<double>::const_iterator it = t.constBegin(); it != t.constEnd(); ++it )
  {
    const double x = mCenter.x() +
                     mSemiMajorAxis * std::cos( *it ) * std::cos( azimuth ) -
                     mSemiMinorAxis * std::sin( *it ) * std::sin( azimuth );
    const double y = mCenter.y() +
                     mSemiMajorAxis * std::cos( *it ) * std::sin( azimuth ) +
                     mSemiMinorAxis * std::sin( *it ) * std::cos( azimuth );
    pts.push_back( QgsPoint( pType, x, y, z, m ) );
  }

  return pts;
}

QgsPolygon *QgsEllipse::toPolygon( unsigned int segments ) const
{
  std::unique_ptr<QgsPolygon> polygon( new QgsPolygon() );
  if ( segments < 3 )
  {
    return polygon.release();
  }

  polygon->setExteriorRing( toLineString( segments ) );

  return polygon.release();
}

QgsLineString *QgsEllipse::toLineString( unsigned int segments ) const
{
  std::unique_ptr<QgsLineString> ext( new QgsLineString() );
  if ( segments < 3 )
  {
    return ext.release();
  }

  QgsPointSequence pts;
  pts = points( segments );
  pts.append( pts.at( 0 ) ); // close linestring

  ext->setPoints( pts );

  return ext.release();
}

QgsRectangle QgsEllipse::boundingBox() const
{
  if ( isEmpty() )
  {
    return QgsRectangle();
  }

  const double angle = mAzimuth * M_PI / 180.0;

  const double ux = mSemiMajorAxis * std::cos( angle );
  const double uy = mSemiMinorAxis * std::sin( angle );
  const double vx = mSemiMajorAxis * std::sin( angle );
  const double vy = mSemiMinorAxis * std::cos( angle );

  const double halfHeight = std::sqrt( ux * ux + uy * uy );
  const double halfWidth = std::sqrt( vx * vx + vy * vy );

  const QgsPointXY p1( mCenter.x() - halfWidth, mCenter.y() - halfHeight );
  const QgsPointXY p2( mCenter.x() + halfWidth, mCenter.y() + halfHeight );

  return QgsRectangle( p1, p2 );
}

QString QgsEllipse::toString( int pointPrecision, int axisPrecision, int azimuthPrecision ) const
{
  QString rep;
  if ( isEmpty() )
    rep = QStringLiteral( "Empty" );
  else
    rep = QStringLiteral( "Ellipse (Center: %1, Semi-Major Axis: %2, Semi-Minor Axis: %3, Azimuth: %4)" )
          .arg( mCenter.asWkt( pointPrecision ), 0, 's' )
          .arg( qgsDoubleToString( mSemiMajorAxis, axisPrecision ), 0, 'f' )
          .arg( qgsDoubleToString( mSemiMinorAxis, axisPrecision ), 0, 'f' )
          .arg( qgsDoubleToString( mAzimuth, azimuthPrecision ), 0, 'f' );

  return rep;
}

QgsPolygon *QgsEllipse::orientedBoundingBox() const
{
  std::unique_ptr<QgsPolygon> ombb( new QgsPolygon() );
  if ( isEmpty() )
  {
    return ombb.release();
  }

  const QVector<QgsPoint> q = quadrant();

  const QgsPoint p1 = q.at( 0 ).project( mSemiMinorAxis, mAzimuth - 90 );
  const QgsPoint p2 = q.at( 0 ).project( mSemiMinorAxis, mAzimuth + 90 );
  const QgsPoint p3 = q.at( 2 ).project( mSemiMinorAxis, mAzimuth + 90 );
  const QgsPoint p4 = q.at( 2 ).project( mSemiMinorAxis, mAzimuth - 90 );

  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << p1 << p2 << p3 << p4 );

  ombb->setExteriorRing( ext );

  return ombb.release();
}
