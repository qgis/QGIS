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
               QgsGeometryUtilsBase::normalizedAngle( M_PI / 180.0 * ( mAzimuth + 90 ) );
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
  const double azimuth = 180.0 / M_PI * QgsGeometryUtilsBase::lineAngle( pt1.x(), pt1.y(), pt2.x(), pt2.y() );
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
  const double azimuth = 180.0 / M_PI * QgsGeometryUtilsBase::lineAngle( center.x(), center.y(), pt1.x(), pt1.y() );
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
             QgsGeometryUtilsBase::normalizedAngle( M_PI / 180.0 * azimuth );
}

double QgsEllipse::focusDistance() const
{
  return std::sqrt( mSemiMajorAxis * mSemiMajorAxis - mSemiMinorAxis * mSemiMinorAxis );
}

QVector<QgsPoint> QgsEllipse::foci() const
{
  const double dist_focus = focusDistance();
  return
  {
    mCenter.project( dist_focus, mAzimuth ),
    mCenter.project( -dist_focus, mAzimuth ),
  };
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
  return
  {
    mCenter.project( mSemiMajorAxis, mAzimuth ),
    mCenter.project( mSemiMinorAxis, mAzimuth + 90 ),
    mCenter.project( -mSemiMajorAxis, mAzimuth ),
    mCenter.project( -mSemiMinorAxis, mAzimuth + 90 )
  };
}

QgsPointSequence QgsEllipse::points( unsigned int segments ) const
{
  QgsPointSequence pts;

  QVector<double> x;
  QVector<double> y;
  QVector<double> z;
  QVector<double> m;

  pointsInternal( segments, x, y, z, m );
  const bool hasZ = !z.empty();
  const bool hasM = !m.empty();
  pts.reserve( x.size() );
  for ( int i = 0; i < x.size(); ++i )
  {
    pts.append( QgsPoint( x[i], y[i],
                          hasZ ? z[i] : std::numeric_limits< double >::quiet_NaN(),
                          hasM ? m[i] : std::numeric_limits< double >::quiet_NaN() ) );
  }
  return pts;
}

void QgsEllipse::pointsInternal( unsigned int segments, QVector<double> &x, QVector<double> &y, QVector<double> &z, QVector<double> &m ) const
{
  if ( segments < 3 )
  {
    return;
  }

  const double centerX = mCenter.x();
  const double centerY = mCenter.y();
  const double centerZ = mCenter.z();
  const double centerM = mCenter.m();
  const bool hasZ = mCenter.is3D();
  const bool hasM = mCenter.isMeasure();

  std::vector<double> t( segments );
  const QgsPoint p1 = mCenter.project( mSemiMajorAxis, mAzimuth );
  const double azimuth = std::atan2( p1.y() - mCenter.y(), p1.x() - mCenter.x() );
  for ( unsigned int i = 0; i < segments; ++i )
  {
    t[i] = 2 * M_PI - ( ( 2 * M_PI ) / segments * i ); // Since the algorithm used rotates in the trigonometric direction (counterclockwise)
  }

  x.resize( segments );
  y.resize( segments );
  if ( hasZ )
    z.resize( segments );
  if ( hasM )
    m.resize( segments );
  double *xOut = x.data();
  double *yOut = y.data();
  double *zOut = hasZ ? z.data() : nullptr;
  double *mOut = hasM ? m.data() : nullptr;

  const double cosAzimuth = std::cos( azimuth );
  const double sinAzimuth = std::sin( azimuth );
  for ( double it : t )
  {
    *xOut++ = centerX +
              mSemiMajorAxis * std::cos( it ) * cosAzimuth -
              mSemiMinorAxis * std::sin( it ) * sinAzimuth;
    *yOut++ = centerY +
              mSemiMajorAxis * std::cos( it ) * sinAzimuth +
              mSemiMinorAxis * std::sin( it ) * cosAzimuth;
    if ( zOut )
      *zOut++ = centerZ;
    if ( mOut )
      *mOut++ = centerM;
  }
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
  if ( segments < 3 )
  {
    return new QgsLineString();
  }

  QVector<double> x;
  QVector<double> y;
  QVector<double> z;
  QVector<double> m;

  pointsInternal( segments, x, y, z, m );
  if ( x.empty() )
    return new QgsLineString();

  // close linestring
  x.append( x.at( 0 ) );
  y.append( y.at( 0 ) );
  if ( !z.empty() )
    z.append( z.at( 0 ) );
  if ( !m.empty() )
    m.append( m.at( 0 ) );

  return new QgsLineString( x, y, z, m );
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
