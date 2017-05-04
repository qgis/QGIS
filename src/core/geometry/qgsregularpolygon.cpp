/***************************************************************************
                         qgsregularpolygon.cpp
                         --------------
    begin                : May 2017
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

#include "qgsregularpolygon.h"
#include "qgsgeometryutils.h"

#include <memory>

QgsRegularPolygon::QgsRegularPolygon()
  : mCenter( QgsPointV2() )
  , mVertice( QgsPointV2() )
  , mNumSides( 0 )
  , mRadius( 0.0 )
{

}


QgsRegularPolygon::QgsRegularPolygon( const QgsPointV2 center, const double radius, const double azimuth, const int numSides, const int circle )
  : mCenter( center )
{
  mCenter = QgsPointV2();
  mVertice = QgsPointV2();
  mNumSides = 0;
  mRadius = 0;
  // TODO: inclination

  if ( numSides >= 3 )
  {
    mNumSides = numSides;

    if ( circle == ConstructionOption::inscribedCircle )
    {
      mRadius = qAbs( radius );
      mVertice = mCenter.project( mRadius, azimuth );
    }
    else if ( circle == ConstructionOption::circumscribedCircle )
    {
      mRadius = apothemToRadius( qAbs( radius ), numSides );
      mVertice = mCenter.project( mRadius, azimuth - centralAngle( numSides ) / 2 );
    }
  }
}

QgsRegularPolygon::QgsRegularPolygon( const QgsPointV2 center, const QgsPointV2 pt1, const int numSides, const int circle )
  : mCenter( center )
{
  mCenter = QgsPointV2();
  mVertice = QgsPointV2();
  mNumSides = 0;
  mRadius = 0;

  if ( numSides >= 3 )
  {
    mNumSides = numSides;

    if ( circle == ConstructionOption::inscribedCircle )
    {
      mVertice = pt1;
      mRadius = center.distance( pt1 );
    }
    else if ( circle == ConstructionOption::circumscribedCircle )
    {
      mRadius = apothemToRadius( center.distance( pt1 ), numSides );
      double azimuth = center.azimuth( pt1 );
      // TODO: inclination
      mVertice = mCenter.project( mRadius, azimuth - centralAngle( numSides ) / 2 );
    }
  }
}

QgsRegularPolygon::QgsRegularPolygon( const QgsPointV2 pt1, const QgsPointV2 pt2, const int numSides )
{
  mCenter = QgsPointV2();
  mVertice = QgsPointV2();
  mNumSides = 0;
  mRadius = 0;

  if ( numSides >= 3 )
  {
    mNumSides = numSides;

    double azimuth = pt1.azimuth( pt2 );
    QgsPointV2 pm = QgsGeometryUtils::midpoint( pt1, pt2 );
    double length = pt1.distance( pm );

    double angle = ( 180 - ( 360 / numSides ) ) / 2.0;
    double hypothenuse = length / cos( angle * M_PI / 180 );
    // TODO: inclination

    mCenter = pt1.project( hypothenuse, azimuth + angle );
    mVertice = pt1;
    mRadius = qAbs( hypothenuse );
  }
}

bool QgsRegularPolygon::operator ==( const QgsRegularPolygon &rp ) const
{
  return ( ( mCenter == rp.mCenter ) &&
           ( mVertice == rp.mVertice ) &&
           ( mNumSides == rp.mNumSides ) &&
           /* useless but... */
           ( mRadius == rp.mRadius )
         );
}

bool QgsRegularPolygon::operator !=( const QgsRegularPolygon &rp ) const
{
  return !operator==( rp );
}

bool QgsRegularPolygon::isEmpty() const
{
  return ( ( mNumSides < 3 ) ||
           ( mCenter == mVertice ) ||
           /* useless but... */
           ( qgsDoubleNear( mRadius, 0.0 ) )
         );
}

void QgsRegularPolygon::setCenter( const QgsPointV2 center )
{
  double azimuth = mCenter.azimuth( mVertice );
  // TODO: double inclination = mCenter.inclination(mVertice);
  mCenter = center;
  mVertice = center.project( mRadius, azimuth );
}

void QgsRegularPolygon::setRadius( const double radius )
{
  mRadius = qAbs( radius );
  double azimuth = mCenter.azimuth( mVertice );
  // TODO: double inclination = mCenter.inclination(mVertice);
  mVertice = mCenter.project( mRadius, azimuth );
}

void QgsRegularPolygon::setVertice( const QgsPointV2 vertice )
{
  double azimuth = mCenter.azimuth( mVertice );
  // TODO: double inclination = mCenter.inclination(mVertice);
  mVertice = vertice;
  mCenter = mVertice.project( mRadius, azimuth );
}

void QgsRegularPolygon::setNumSides( const int numSides )
{
  if ( numSides >= 3 )
  {
    mNumSides = numSides;
  }
}

void QgsRegularPolygon::points( QgsPointSequence &pts ) const
{
  pts.clear();
  if ( isEmpty() )
  {
    return;
  }

  double azimuth =  mCenter.azimuth( mVertice );
  double azimuth_add = centralAngle();
  // TODO: inclination

  unsigned int n = 1;
  while ( n <= mNumSides )
  {
    pts.push_back( mCenter.project( mRadius, azimuth ) );
    azimuth += azimuth_add;
    if ( ( azimuth_add > 0 ) && ( azimuth > 180.0 ) )
    {
      azimuth -= 360.0;
    }

    n++;
  }

}

QgsPolygonV2 *QgsRegularPolygon::toPolygon() const
{
  std::unique_ptr<QgsPolygonV2> polygon( new QgsPolygonV2() );
  if ( isEmpty() )
  {
    return polygon.release();
  }

  polygon->setExteriorRing( toLineString( ) );

  return polygon.release();
}

QgsLineString *QgsRegularPolygon::toLineString() const
{
  std::unique_ptr<QgsLineString> ext( new QgsLineString() );
  if ( isEmpty() )
  {
    return ext.release();
  }

  QgsPointSequence pts;
  points( pts );

  ext->setPoints( pts );

  return ext.release();
}

QgsTriangle QgsRegularPolygon::toTriangle() const
{
  if ( isEmpty() || ( mNumSides != 3 ) )
  {
    return QgsTriangle();
  }

  QgsPointSequence pts;
  points( pts );

  return QgsTriangle( pts.at( 0 ), pts.at( 1 ), pts.at( 2 ) );
}

QList<QgsTriangle> QgsRegularPolygon::triangulate() const
{
  QList<QgsTriangle> l_tri;
  if ( isEmpty() )
  {
    return l_tri;
  }

  QgsPointSequence pts;
  points( pts );
  unsigned int n = 0;
  while ( n < mNumSides - 1 )
  {
    l_tri.append( QgsTriangle( pts.at( n ), pts.at( n + 1 ), mCenter ) );
    n++;
  }
  l_tri.append( QgsTriangle( pts.at( n ), pts.at( 0 ), mCenter ) );

  return l_tri;
}

QgsCircle QgsRegularPolygon::inscribedCircle() const
{
  // TODO: inclined circle
  return QgsCircle( mCenter, apothem() );
}

QgsCircle QgsRegularPolygon::circumscribedCircle() const
{
  // TODO: inclined circle
  return QgsCircle( mCenter, mRadius );
}

QString QgsRegularPolygon::toString( int pointPrecision, int radiusPrecision, int anglePrecision ) const
{
  QString rep;
  if ( isEmpty() )
    rep = QStringLiteral( "Empty" );
  else
    rep = QStringLiteral( "RegularPolygon (Center: %1, First Vertice: %2, Radius: %3, Azimuth: %4)" )
          .arg( mCenter.asWkt( pointPrecision ), 0, 's' )
          .arg( mVertice.asWkt( pointPrecision ), 0, 's' )
          .arg( qgsDoubleToString( mRadius, radiusPrecision ), 0, 'f' )
          .arg( qgsDoubleToString( mCenter.azimuth( mVertice ), anglePrecision ), 0, 'f' );
  // TODO: inclination
  // .arg( qgsDoubleToString( mCenter.inclination(mVertice), anglePrecision ), 0, 'f' );

  return rep;
}

double QgsRegularPolygon::area() const
{
  if ( isEmpty() )
  {
    return 0.0;
  }

  return ( mRadius * mRadius * mNumSides * sin( centralAngle() * M_PI / 180.0 ) ) / 2 ;
}

double QgsRegularPolygon::perimeter() const
{
  if ( isEmpty() )
  {
    return 0.0;
  }

  return length() * mNumSides;
}

double QgsRegularPolygon::length() const
{
  if ( isEmpty() )
  {
    return 0.0;
  }

  return mRadius * 2 * sin( M_PI / mNumSides );
}

double QgsRegularPolygon::apothemToRadius( const double apothem, const unsigned int numSides ) const
{
  return apothem / cos( M_PI / numSides );
}

double QgsRegularPolygon::interiorAngle( const unsigned int nbSides ) const
{
  return ( nbSides - 2 ) * 180 / nbSides;
}

double QgsRegularPolygon::centralAngle( const unsigned int nbSides ) const
{
  return 360.0 / nbSides;
}

double QgsRegularPolygon::interiorAngle() const
{
  return interiorAngle( mNumSides );
}

double QgsRegularPolygon::centralAngle() const
{
  return centralAngle( mNumSides );
}
