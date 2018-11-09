/***************************************************************************
                          qgspoint.cpp -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgspointxy.h"
#include "qgspoint.h"

#include <cmath>
#include <QTextStream>
#include <QObject> // for tr()

#include "qgsexception.h"

QgsPointXY::QgsPointXY( const QgsPointXY &p )
{
  mX = p.x();
  mY = p.y();
}

QgsPointXY::QgsPointXY( const QgsPoint &point )
  : mX( point.x() )
  , mY( point.y() )
{
}

QString QgsPointXY::toString( int precision ) const
{
  if ( precision < 0 )
  {
    QString rep;
    QTextStream ot( &rep );
    ot.setRealNumberPrecision( 12 );
    ot << mX << ", " << mY;
    return rep;
  }
  else
  {
    QString x = std::isfinite( mX ) ? QString::number( mX, 'f', precision ) : QObject::tr( "infinite" );
    QString y = std::isfinite( mY ) ? QString::number( mY, 'f', precision ) : QObject::tr( "infinite" );
    return QStringLiteral( "%1,%2" ).arg( x, y );
  }
}

QString QgsPointXY::asWkt() const
{
  return QStringLiteral( "POINT(%1 %2)" ).arg( qgsDoubleToString( mX ), qgsDoubleToString( mY ) );
}

double QgsPointXY::azimuth( const QgsPointXY &other ) const
{
  double dx = other.x() - mX;
  double dy = other.y() - mY;
  return ( std::atan2( dx, dy ) * 180.0 / M_PI );
}

QgsPointXY QgsPointXY::project( double distance, double bearing ) const
{
  double rads = bearing * M_PI / 180.0;
  double dx = distance * std::sin( rads );
  double dy = distance * std::cos( rads );
  return QgsPointXY( mX + dx, mY + dy );
}

double QgsPointXY::sqrDistToSegment( double x1, double y1, double x2, double y2, QgsPointXY &minDistPoint, double epsilon ) const
{
  double nx, ny; //normal vector

  nx = y2 - y1;
  ny = -( x2 - x1 );

  double t;
  t = ( mX * ny - mY * nx - x1 * ny + y1 * nx ) / ( ( x2 - x1 ) * ny - ( y2 - y1 ) * nx );

  if ( t < 0.0 )
  {
    minDistPoint.setX( x1 );
    minDistPoint.setY( y1 );
  }
  else if ( t > 1.0 )
  {
    minDistPoint.setX( x2 );
    minDistPoint.setY( y2 );
  }
  else
  {
    minDistPoint.setX( x1 + t * ( x2 - x1 ) );
    minDistPoint.setY( y1 + t * ( y2 - y1 ) );
  }

  double dist = sqrDist( minDistPoint );
  //prevent rounding errors if the point is directly on the segment
  if ( qgsDoubleNear( dist, 0.0, epsilon ) )
  {
    minDistPoint.setX( mX );
    minDistPoint.setY( mY );
    return 0.0;
  }
  return dist;
}
