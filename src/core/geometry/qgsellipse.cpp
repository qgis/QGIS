/***************************************************************************
                         qgspointv2.h
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

void QgsEllipse::normalizeAxis()
{
  mSemiMajorAxis = fabs( mSemiMajorAxis );
  mSemiMinorAxis = fabs( mSemiMinorAxis );
  if ( mSemiMajorAxis < mSemiMinorAxis )
  {
    std::swap( mSemiMajorAxis, mSemiMinorAxis );
    mAzimuth = QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AngleRadians, QgsUnitTypes::AngleDegrees ) *
               QgsGeometryUtils::normalizedAngle( QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AngleDegrees, QgsUnitTypes::AngleRadians ) * ( mAzimuth + 90 ) );
  }
}

QgsEllipse::QgsEllipse() :
  mCenter( QgsPointV2() ),
  mSemiMajorAxis( 0.0 ),
  mSemiMinorAxis( 0.0 ),
  mAzimuth( 90.0 )
{

}

QgsEllipse::QgsEllipse( const QgsPointV2 &center, const double axis_a, const double axis_b, const double azimuth ) :
  mCenter( center ),
  mSemiMajorAxis( axis_a ),
  mSemiMinorAxis( axis_b ),
  mAzimuth( azimuth )
{
  normalizeAxis();
}

QgsEllipse QgsEllipse::fromFoci( const QgsPointV2 &pt1, const QgsPointV2 &pt2, const QgsPointV2 &pt3 )
{
  double dist_p1p2 = pt1.distance( pt2 );
  double dist_p1p3 = pt1.distance( pt3 );
  double dist_p2p3 = pt2.distance( pt3 );

  double dist = dist_p1p3 + dist_p2p3;
  double azimuth = QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AngleRadians, QgsUnitTypes::AngleDegrees ) * QgsGeometryUtils::lineAngle( pt1.x(), pt1.y(), pt2.x(), pt2.y() );
  QgsPointV2 center = QgsGeometryUtils::midpoint( pt1, pt2 );

  double axis_a = dist / 2.0;
  double axis_b = sqrt( pow( axis_a, 2.0 ) - pow( dist_p1p2 / 2.0, 2.0 ) );

  return QgsEllipse( center, axis_a, axis_b, azimuth );
}

QgsEllipse QgsEllipse::byExtent( const QgsPointV2 &pt1, const QgsPointV2 &pt2 )
{
  QgsPointV2 center = QgsGeometryUtils::midpoint( pt1, pt2 );
  double axis_a = fabs( pt2.x() - pt1.x() ) / 2.0;
  double axis_b = fabs( pt2.y() - pt1.y() ) / 2.0;
  double azimuth = 90.0;

  return QgsEllipse( center, axis_a, axis_b, azimuth );
}

QgsEllipse QgsEllipse::byCenterPoint( const QgsPointV2 &ptc, const QgsPointV2 &pt1 )
{
  double axis_a = fabs( pt1.x() - ptc.x() );
  double axis_b = fabs( pt1.y() - ptc.y() );
  double azimuth = 90.0;

  return QgsEllipse( ptc, axis_a, axis_b, azimuth );
}

QgsEllipse QgsEllipse::byCenter2Points( const QgsPointV2 &ptc, const QgsPointV2 &pt1, const QgsPointV2 &pt2 )
{
  double azimuth = QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AngleRadians, QgsUnitTypes::AngleDegrees ) * QgsGeometryUtils::lineAngle( ptc.x(), ptc.y(), pt1.x(), pt1.y() );
  double axis_a = ptc.distance( pt1 );

  double length = pt2.distance( QgsGeometryUtils::projPointOnSegment( pt2, ptc, pt1 ) );
  QgsPointV2 pp = ptc.project( length, 90 + azimuth );
  double axis_b = ptc.distance( pp );

  return QgsEllipse( ptc, axis_a, axis_b, azimuth );
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
  mAzimuth = QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AngleRadians, QgsUnitTypes::AngleDegrees ) *
             QgsGeometryUtils::normalizedAngle( QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AngleDegrees, QgsUnitTypes::AngleRadians ) * azimuth );
}

double QgsEllipse::focusDistance() const
{
  return sqrt( mSemiMajorAxis * mSemiMajorAxis - mSemiMinorAxis * mSemiMinorAxis );
}

QVector<QgsPointV2> QgsEllipse::foci() const
{
  QVector<QgsPointV2> f;
  double dist_focus = focusDistance();
  f.append( mCenter.project( dist_focus, mAzimuth ) );
  f.append( mCenter.project( -dist_focus, mAzimuth ) );

  return f;
}

double QgsEllipse::eccentricity() const
{
  if ( isEmpty() )
  {
    return NAN;
  }
  return focusDistance() / mSemiMajorAxis;
}

double QgsEllipse::area() const
{
  return M_PI * mSemiMajorAxis * mSemiMinorAxis;
}

double QgsEllipse::perimeter() const
{
  double a = mSemiMajorAxis;
  double b = mSemiMinorAxis;
  return M_PI * ( 3 * ( a + b ) - sqrt( 10 * a * b + 3 * ( a * a + b * b ) ) );
}

QVector<QgsPointV2> QgsEllipse::quadrant() const
{
  QVector<QgsPointV2> quad;
  quad.append( mCenter.project( mSemiMajorAxis, mAzimuth ) );
  quad.append( mCenter.project( mSemiMinorAxis, mAzimuth + 90 ) );
  quad.append( mCenter.project( -mSemiMajorAxis, mAzimuth ) );
  quad.append( mCenter.project( -mSemiMinorAxis, mAzimuth + 90 ) );

  return quad;
}

void QgsEllipse::points( QgsPointSequence &pts, unsigned int segments ) const
{
  pts.clear();
  if ( segments < 3 )
  {
    return;
  }


  QgsWkbTypes::Type pType( mCenter.wkbType() );
  double z = mCenter.z();
  double m = mCenter.m();

  QVector<double> t;
  double azimuth =  atan2( quadrant().at( 0 ).y() - mCenter.y(), quadrant().at( 0 ).x() - mCenter.x() );
  for ( unsigned int i = 0; i < segments; ++i )
  {
    t.append( 2 * M_PI - ( ( 2 * M_PI ) / segments * i ) ); // Since the algorithm used rotates in the trigonometric direction (counterclockwise)
  }

  for ( QVector<double>::const_iterator it = t.constBegin(); it != t.constEnd(); ++it )
  {
    double x = mCenter.x() +
               mSemiMajorAxis * cos( *it ) * cos( azimuth ) -
               mSemiMinorAxis * sin( *it ) * sin( azimuth );
    double y = mCenter.y() +
               mSemiMajorAxis * cos( *it ) * sin( azimuth ) +
               mSemiMinorAxis * sin( *it ) * cos( azimuth );
    pts.push_back( QgsPointV2( pType, x, y, z, m ) );
  }
}

QgsPolygonV2 QgsEllipse::toPolygon( unsigned int segments ) const
{
  if ( segments < 3 )
  {
    return QgsPolygonV2();
  }

  QgsPolygonV2 p;
  QgsLineString *ext = 0;
  ext = toLineString( segments ).clone();

  p.setExteriorRing( ext );

  return p;
}

QgsLineString QgsEllipse::toLineString( unsigned int segments ) const
{
  if ( segments < 3 )
  {
    return QgsLineString();
  }

  QgsPointSequence pts;
  points( pts, segments );

  QgsLineString ext;
  ext.setPoints( pts );

  return ext;
}

QgsRectangle QgsEllipse::boundingBox() const
{
  return orientedBoundingBox().boundingBox();
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

QgsPolygonV2 QgsEllipse::orientedBoundingBox() const
{
  if ( isEmpty() )
  {
    return QgsPolygonV2();
  }

  QVector<QgsPointV2> q = quadrant();

  QgsPointV2 p1 = q.at( 0 ).project( mSemiMinorAxis, mAzimuth - 90 );
  QgsPointV2 p2 = q.at( 0 ).project( mSemiMinorAxis, mAzimuth + 90 );
  QgsPointV2 p3 = q.at( 2 ).project( mSemiMinorAxis, mAzimuth + 90 );
  QgsPointV2 p4 = q.at( 2 ).project( mSemiMinorAxis, mAzimuth - 90 );
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << p1 << p2 << p3 << p4 );
  QgsPolygonV2 ombb;
  ombb.setExteriorRing( ext );

  return ombb;
}
