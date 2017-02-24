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


#include "qgspoint.h"
#include "qgspointv2.h"

#include <cmath>
#include <QTextStream>
#include <QObject> // for tr()

#include "qgsexception.h"

QgsPoint::QgsPoint( const QgsPoint& p )
{
  mX = p.x();
  mY = p.y();
}

QgsPoint::QgsPoint( const QgsPointV2& point )
    : mX( point.x() )
    , mY( point.y() )
{
}

QPointF QgsPoint::toQPointF() const
{
  return QPointF( mX, mY );
}

QString QgsPoint::toString() const
{
  QString rep;
  QTextStream ot( &rep );
  ot.setRealNumberPrecision( 12 );
  ot << mX << ", " << mY;
  return rep;
}

QString QgsPoint::toString( int precision ) const
{
  QString x = qIsFinite( mX ) ? QString::number( mX, 'f', precision ) : QObject::tr( "infinite" );
  QString y = qIsFinite( mY ) ? QString::number( mY, 'f', precision ) : QObject::tr( "infinite" );
  return QStringLiteral( "%1,%2" ).arg( x, y );
}

QString QgsPoint::toDegreesMinutesSeconds( int precision, const bool useSuffix, const bool padded ) const
{
  //first, limit longitude to -360 to 360 degree range
  double myWrappedX = fmod( mX, 360.0 );
  //next, wrap around longitudes > 180 or < -180 degrees, so that, e.g., "190E" -> "170W"
  if ( myWrappedX > 180.0 )
  {
    myWrappedX = myWrappedX - 360.0;
  }
  else if ( myWrappedX < -180.0 )
  {
    myWrappedX = myWrappedX + 360.0;
  }

  //first, limit latitude to -180 to 180 degree range
  double myWrappedY = fmod( mY, 180.0 );
  //next, wrap around latitudes > 90 or < -90 degrees, so that, e.g., "110S" -> "70N"
  if ( myWrappedY > 90.0 )
  {
    myWrappedY = myWrappedY - 180.0;
  }
  else if ( myWrappedY < -90.0 )
  {
    myWrappedY = myWrappedY + 180.0;
  }

  int myDegreesX = int( qAbs( myWrappedX ) );
  double myFloatMinutesX = double(( qAbs( myWrappedX ) - myDegreesX ) * 60 );
  int myIntMinutesX = int( myFloatMinutesX );
  double mySecondsX = double( myFloatMinutesX - myIntMinutesX ) * 60;

  int myDegreesY = int( qAbs( myWrappedY ) );
  double myFloatMinutesY = double(( qAbs( myWrappedY ) - myDegreesY ) * 60 );
  int myIntMinutesY = int( myFloatMinutesY );
  double mySecondsY = double( myFloatMinutesY - myIntMinutesY ) * 60;

  //make sure rounding to specified precision doesn't create seconds >= 60
  if ( qRound( mySecondsX * pow( 10.0, precision ) ) >= 60 * pow( 10.0, precision ) )
  {
    mySecondsX = qMax( mySecondsX - 60, 0.0 );
    myIntMinutesX++;
    if ( myIntMinutesX >= 60 )
    {
      myIntMinutesX -= 60;
      myDegreesX++;
    }
  }
  if ( qRound( mySecondsY * pow( 10.0, precision ) ) >= 60 * pow( 10.0, precision ) )
  {
    mySecondsY = qMax( mySecondsY - 60, 0.0 );
    myIntMinutesY++;
    if ( myIntMinutesY >= 60 )
    {
      myIntMinutesY -= 60;
      myDegreesY++;
    }
  }

  QString myXHemisphere;
  QString myYHemisphere;
  QString myXSign;
  QString myYSign;
  if ( useSuffix )
  {
    myXHemisphere = myWrappedX < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
    myYHemisphere = myWrappedY < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  }
  else
  {
    if ( myWrappedX < 0 )
    {
      myXSign = QObject::tr( "-" );
    }
    if ( myWrappedY < 0 )
    {
      myYSign = QObject::tr( "-" );
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( myDegreesX == 0 && myIntMinutesX == 0 && qRound( mySecondsX * pow( 10.0, precision ) ) == 0 )
  {
    myXSign = QString();
    myXHemisphere = QString();
  }
  if ( myDegreesY == 0 && myIntMinutesY == 0 && qRound( mySecondsY * pow( 10.0, precision ) ) == 0 )
  {
    myYSign = QString();
    myYHemisphere = QString();
  }
  //also remove directional prefix from 180 degree longitudes
  if ( myDegreesX == 180 && myIntMinutesX == 0 && qRound( mySecondsX * pow( 10.0, precision ) ) == 0 )
  {
    myXHemisphere = QString();
  }
  //pad minutes with leading digits if required
  QString myMinutesX = padded ? QStringLiteral( "%1" ).arg( myIntMinutesX, 2, 10, QChar( '0' ) ) : QString::number( myIntMinutesX );
  QString myMinutesY = padded ? QStringLiteral( "%1" ).arg( myIntMinutesY, 2, 10, QChar( '0' ) ) : QString::number( myIntMinutesY );
  //pad seconds with leading digits if required
  int digits = 2 + ( precision == 0 ? 0 : 1 + precision ); //1 for decimal place if required
  QString myStrSecondsX = padded ? QStringLiteral( "%1" ).arg( mySecondsX, digits, 'f', precision, QChar( '0' ) ) : QString::number( mySecondsX, 'f', precision );
  QString myStrSecondsY = padded ? QStringLiteral( "%1" ).arg( mySecondsY, digits, 'f', precision, QChar( '0' ) ) : QString::number( mySecondsY, 'f', precision );

  QString rep = myXSign + QString::number( myDegreesX ) + QChar( 176 ) +
                myMinutesX + QChar( 0x2032 ) +
                myStrSecondsX + QChar( 0x2033 ) +
                myXHemisphere + ',' +
                myYSign + QString::number( myDegreesY ) + QChar( 176 ) +
                myMinutesY + QChar( 0x2032 ) +
                myStrSecondsY + QChar( 0x2033 ) +
                myYHemisphere;
  return rep;
}

QString QgsPoint::toDegreesMinutes( int precision, const bool useSuffix, const bool padded ) const
{
  //first, limit longitude to -360 to 360 degree range
  double myWrappedX = fmod( mX, 360.0 );
  //next, wrap around longitudes > 180 or < -180 degrees, so that, e.g., "190E" -> "170W"
  if ( myWrappedX > 180.0 )
  {
    myWrappedX = myWrappedX - 360.0;
  }
  else if ( myWrappedX < -180.0 )
  {
    myWrappedX = myWrappedX + 360.0;
  }

  int myDegreesX = int( qAbs( myWrappedX ) );
  double myFloatMinutesX = double(( qAbs( myWrappedX ) - myDegreesX ) * 60 );

  int myDegreesY = int( qAbs( mY ) );
  double myFloatMinutesY = double(( qAbs( mY ) - myDegreesY ) * 60 );

  //make sure rounding to specified precision doesn't create minutes >= 60
  if ( qRound( myFloatMinutesX * pow( 10.0, precision ) ) >= 60 * pow( 10.0, precision ) )
  {
    myFloatMinutesX = qMax( myFloatMinutesX - 60, 0.0 );
    myDegreesX++;
  }
  if ( qRound( myFloatMinutesY * pow( 10.0, precision ) ) >= 60 * pow( 10.0, precision ) )
  {
    myFloatMinutesY = qMax( myFloatMinutesY - 60, 0.0 );
    myDegreesY++;
  }

  QString myXHemisphere;
  QString myYHemisphere;
  QString myXSign;
  QString myYSign;
  if ( useSuffix )
  {
    myXHemisphere = myWrappedX < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
    myYHemisphere = mY < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  }
  else
  {
    if ( myWrappedX < 0 )
    {
      myXSign = QObject::tr( "-" );
    }
    if ( mY < 0 )
    {
      myYSign = QObject::tr( "-" );
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( myDegreesX == 0 && qRound( myFloatMinutesX * pow( 10.0, precision ) ) == 0 )
  {
    myXSign = QString();
    myXHemisphere = QString();
  }
  if ( myDegreesY == 0 && qRound( myFloatMinutesY * pow( 10.0, precision ) ) == 0 )
  {
    myYSign = QString();
    myYHemisphere = QString();
  }
  //also remove directional prefix from 180 degree longitudes
  if ( myDegreesX == 180 && qRound( myFloatMinutesX * pow( 10.0, precision ) ) == 0 )
  {
    myXHemisphere = QString();
  }

  //pad minutes with leading digits if required
  int digits = 2 + ( precision == 0 ? 0 : 1 + precision ); //1 for decimal place if required
  QString myStrMinutesX = padded ? QStringLiteral( "%1" ).arg( myFloatMinutesX, digits, 'f', precision, QChar( '0' ) ) : QString::number( myFloatMinutesX, 'f', precision );
  QString myStrMinutesY = padded ? QStringLiteral( "%1" ).arg( myFloatMinutesY, digits, 'f', precision, QChar( '0' ) ) : QString::number( myFloatMinutesY, 'f', precision );

  QString rep = myXSign + QString::number( myDegreesX ) + QChar( 176 ) +
                myStrMinutesX + QChar( 0x2032 ) +
                myXHemisphere + ',' +
                myYSign + QString::number( myDegreesY ) + QChar( 176 ) +
                myStrMinutesY + QChar( 0x2032 ) +
                myYHemisphere;
  return rep;
}

QString QgsPoint::wellKnownText() const
{
  return QStringLiteral( "POINT(%1 %2)" ).arg( qgsDoubleToString( mX ), qgsDoubleToString( mY ) );
}

double QgsPoint::sqrDist( double x, double y ) const
{
  return ( mX - x ) * ( mX - x ) + ( mY - y ) * ( mY - y );
}

double QgsPoint::sqrDist( const QgsPoint& other ) const
{
  return sqrDist( other.x(), other.y() );
}

double QgsPoint::distance( double x, double y ) const
{
  return sqrt( sqrDist( x, y ) );
}

double QgsPoint::distance( const QgsPoint& other ) const
{
  return sqrt( sqrDist( other ) );
}

double QgsPoint::azimuth( const QgsPoint& other ) const
{
  double dx = other.x() - mX;
  double dy = other.y() - mY;
  return ( atan2( dx, dy ) * 180.0 / M_PI );
}

QgsPoint QgsPoint::project( double distance, double bearing ) const
{
  double rads = bearing * M_PI / 180.0;
  double dx = distance * sin( rads );
  double dy = distance * cos( rads );
  return QgsPoint( mX + dx, mY + dy );
}

bool QgsPoint::compare( const QgsPoint &other, double epsilon ) const
{
  return ( qgsDoubleNear( mX, other.x(), epsilon ) && qgsDoubleNear( mY, other.y(), epsilon ) );
}

// operators
bool QgsPoint::operator==( const QgsPoint & other )
{
  return ( qgsDoubleNear( mX, other.x() ) && qgsDoubleNear( mY, other.y() ) );
}

bool QgsPoint::operator!=( const QgsPoint & other ) const
{
  return !( qgsDoubleNear( mX, other.x() ) && qgsDoubleNear( mY, other.y() ) );
}

QgsPoint & QgsPoint::operator=( const QgsPoint & other )
{
  if ( &other != this )
  {
    mX = other.x();
    mY = other.y();
  }

  return *this;
}

void QgsPoint::multiply( double scalar )
{
  mX *= scalar;
  mY *= scalar;
}

int QgsPoint::onSegment( const QgsPoint& a, const QgsPoint& b ) const
{
  //algorithm from 'graphics GEMS', A. Paeth: 'A Fast 2D Point-on-line test'
  if (
    qAbs(( b.y() - a.y() ) *( mX - a.x() ) - ( mY - a.y() ) *( b.x() - a.x() ) )
    >= qMax( qAbs( b.x() - a.x() ), qAbs( b.y() - a.y() ) )
  )
  {
    return 0;
  }
  if (( b.x() < a.x() && a.x() < mX ) || ( b.y() < a.y() && a.y() < mY ) )
  {
    return 1;
  }
  if (( mX < a.x() && a.x() < b.x() ) || ( mY < a.y() && a.y() < b.y() ) )
  {
    return 1;
  }
  if (( a.x() < b.x() && b.x() < mX ) || ( a.y() < b.y() && b.y() < mY ) )
  {
    return 3;
  }
  if (( mX < b.x() && b.x() < a.x() ) || ( mY < b.y() && b.y() < a.y() ) )
  {
    return 3;
  }

  return 2;
}

double QgsPoint::sqrDistToSegment( double x1, double y1, double x2, double y2, QgsPoint& minDistPoint, double epsilon ) const
{
  double nx, ny; //normal vector

  nx = y2 - y1;
  ny = -( x2 - x1 );

  double t;
  t = ( mX * ny - mY * nx - x1 * ny + y1 * nx ) / (( x2 - x1 ) * ny - ( y2 - y1 ) * nx );

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
    minDistPoint.setX( x1 + t *( x2 - x1 ) );
    minDistPoint.setY( y1 + t *( y2 - y1 ) );
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
