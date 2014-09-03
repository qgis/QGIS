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
#include "qgis.h"
#include <cmath>
#include <QTextStream>
#include <QObject> // for tr()

#include "qgsexception.h"

//
// QgsVector
//

QgsVector::QgsVector() : m_x( 0.0 ), m_y( 0.0 )
{
}

QgsVector::QgsVector( double x, double y ) : m_x( x ), m_y( y )
{
}

QgsVector QgsVector::operator-( void ) const
{
  return QgsVector( -m_x, -m_y );
}

QgsVector QgsVector::operator*( double scalar ) const
{
  return QgsVector( m_x * scalar, m_y * scalar );
}

QgsVector QgsVector::operator/( double scalar ) const
{
  return *this * ( 1.0 / scalar );
}

double QgsVector::operator*( QgsVector v ) const
{
  return m_x * v.m_x + m_y * v.m_y;
}

double QgsVector::length() const
{
  return sqrt( m_x * m_x + m_y * m_y );
}

double QgsVector::x() const
{
  return m_x;
}

double QgsVector::y() const
{
  return m_y;
}

// perpendicular vector (rotated 90° counter-clockwise)
QgsVector QgsVector::perpVector() const
{
  return QgsVector( -m_y, m_x );
}

double QgsVector::angle( void ) const
{
  double ang = atan2( m_y, m_x );
  return ang < 0.0 ? ang + 2.0 * M_PI : ang;
}

double QgsVector::angle( QgsVector v ) const
{
  return v.angle() - angle();
}

QgsVector QgsVector::rotateBy( double rot ) const
{
  double ang = atan2( m_y, m_x ) + rot;
  double len = length();
  return QgsVector( len * cos( ang ), len * sin( ang ) );
}

QgsVector QgsVector::normal() const
{
  double len = length();

  if ( len == 0.0 )
  {
    throw QgsException( "normal vector of null vector undefined" );
  }

  return *this / len;
}


//
// QgsPoint
//

QgsPoint::QgsPoint( const QgsPoint& p )
{
  m_x = p.x();
  m_y = p.y();
}

QString QgsPoint::toString() const
{
  QString rep;
  QTextStream ot( &rep );
  ot.setRealNumberPrecision( 12 );
  ot << m_x << ", " << m_y;
  return rep;
}

QString QgsPoint::toString( int thePrecision ) const
{
  QString x = qIsFinite( m_x ) ? QString::number( m_x, 'f', thePrecision ) : QObject::tr( "infinite" );
  QString y = qIsFinite( m_y ) ? QString::number( m_y, 'f', thePrecision ) : QObject::tr( "infinite" );
  return QString( "%1,%2" ).arg( x ).arg( y );
}

QString QgsPoint::toDegreesMinutesSeconds( int thePrecision, const bool useSuffix, const bool padded ) const
{
  //first, limit longitude to -360 to 360 degree range
  double myWrappedX = fmod( m_x, 360.0 );
  //next, wrap around longitudes > 180 or < -180 degrees, so that eg "190E" -> "170W"
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
  int myIntMinutesX = int( myFloatMinutesX );
  double mySecondsX = double( myFloatMinutesX - myIntMinutesX ) * 60;

  int myDegreesY = int( qAbs( m_y ) );
  double myFloatMinutesY = double(( qAbs( m_y ) - myDegreesY ) * 60 );
  int myIntMinutesY = int( myFloatMinutesY );
  double mySecondsY = double( myFloatMinutesY - myIntMinutesY ) * 60;

  //make sure rounding to specified precision doesn't create seconds >= 60
  if ( qRound( mySecondsX * pow( 10.0, thePrecision ) ) >= 60 * pow( 10.0, thePrecision ) )
  {
    mySecondsX = qMax( mySecondsX - 60, 0.0 );
    myIntMinutesX++;
    if ( myIntMinutesX >= 60 )
    {
      myIntMinutesX -= 60;
      myDegreesX++;
    }
  }
  if ( qRound( mySecondsY * pow( 10.0, thePrecision ) ) >= 60 * pow( 10.0, thePrecision ) )
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
    myYHemisphere = m_y < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  }
  else
  {
    if ( myWrappedX < 0 )
    {
      myXSign = QObject::tr( "-" );
    }
    if ( m_y < 0 )
    {
      myYSign = QObject::tr( "-" );
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( myDegreesX == 0 && myIntMinutesX == 0 && qRound( mySecondsX * pow( 10.0, thePrecision ) ) == 0 )
  {
    myXSign = QString();
    myXHemisphere = QString();
  }
  if ( myDegreesY == 0 && myIntMinutesY == 0 && qRound( mySecondsY * pow( 10.0, thePrecision ) ) == 0 )
  {
    myYSign = QString();
    myYHemisphere = QString();
  }
  //also remove directional prefix from 180 degree longitudes
  if ( myDegreesX == 180 && myIntMinutesX == 0 && qRound( mySecondsX * pow( 10.0, thePrecision ) ) == 0 )
  {
    myXHemisphere = QString();
  }
  //pad minutes with leading digits if required
  QString myMinutesX = padded ? QString( "%1" ).arg( myIntMinutesX, 2, 10, QChar( '0' ) ) : QString::number( myIntMinutesX );
  QString myMinutesY = padded ? QString( "%1" ).arg( myIntMinutesY, 2, 10, QChar( '0' ) ) : QString::number( myIntMinutesY );
  //pad seconds with leading digits if required
  int digits = 2 + ( thePrecision == 0 ? 0 : 1 + thePrecision ); //1 for decimal place if required
  QString myStrSecondsX = padded ? QString( "%1" ).arg( mySecondsX, digits, 'f', thePrecision, QChar( '0' ) ) : QString::number( mySecondsX, 'f', thePrecision );
  QString myStrSecondsY = padded ? QString( "%1" ).arg( mySecondsY, digits, 'f', thePrecision, QChar( '0' ) ) : QString::number( mySecondsY, 'f', thePrecision );

  QString rep = myXSign + QString::number( myDegreesX ) + QChar( 176 ) +
                myMinutesX + QString( "'" ) +
                myStrSecondsX + QString( "\"" ) +
                myXHemisphere + QString( "," ) +
                myYSign + QString::number( myDegreesY ) + QChar( 176 ) +
                myMinutesY + QString( "'" ) +
                myStrSecondsY + QString( "\"" ) +
                myYHemisphere;
  return rep;
}

QString QgsPoint::toDegreesMinutes( int thePrecision, const bool useSuffix, const bool padded ) const
{
  //first, limit longitude to -360 to 360 degree range
  double myWrappedX = fmod( m_x, 360.0 );
  //next, wrap around longitudes > 180 or < -180 degrees, so that eg "190E" -> "170W"
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

  int myDegreesY = int( qAbs( m_y ) );
  double myFloatMinutesY = double(( qAbs( m_y ) - myDegreesY ) * 60 );

  //make sure rounding to specified precision doesn't create minutes >= 60
  if ( qRound( myFloatMinutesX * pow( 10.0, thePrecision ) ) >= 60 * pow( 10.0, thePrecision ) )
  {
    myFloatMinutesX = qMax( myFloatMinutesX - 60, 0.0 );
    myDegreesX++;
  }
  if ( qRound( myFloatMinutesY * pow( 10.0, thePrecision ) ) >= 60 * pow( 10.0, thePrecision ) )
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
    myYHemisphere = m_y < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  }
  else
  {
    if ( myWrappedX < 0 )
    {
      myXSign = QObject::tr( "-" );
    }
    if ( m_y < 0 )
    {
      myYSign = QObject::tr( "-" );
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( myDegreesX == 0 && qRound( myFloatMinutesX * pow( 10.0, thePrecision ) ) == 0 )
  {
    myXSign = QString();
    myXHemisphere = QString();
  }
  if ( myDegreesY == 0 && qRound( myFloatMinutesY * pow( 10.0, thePrecision ) ) == 0 )
  {
    myYSign = QString();
    myYHemisphere = QString();
  }
  //also remove directional prefix from 180 degree longitudes
  if ( myDegreesX == 180 && qRound( myFloatMinutesX * pow( 10.0, thePrecision ) ) == 0 )
  {
    myXHemisphere = QString();
  }

  //pad minutes with leading digits if required
  int digits = 2 + ( thePrecision == 0 ? 0 : 1 + thePrecision ); //1 for decimal place if required
  QString myStrMinutesX = padded ? QString( "%1" ).arg( myFloatMinutesX, digits, 'f', thePrecision, QChar( '0' ) ) : QString::number( myFloatMinutesX, 'f', thePrecision );
  QString myStrMinutesY = padded ? QString( "%1" ).arg( myFloatMinutesY, digits, 'f', thePrecision, QChar( '0' ) ) : QString::number( myFloatMinutesY, 'f', thePrecision );

  QString rep = myXSign + QString::number( myDegreesX ) + QChar( 176 ) +
                myStrMinutesX + QString( "'" ) +
                myXHemisphere + QString( "," ) +
                myYSign + QString::number( myDegreesY ) + QChar( 176 ) +
                myStrMinutesY + QString( "'" ) +
                myYHemisphere;
  return rep;
}

QString QgsPoint::wellKnownText() const
{
  return QString( "POINT(%1 %2)" ).arg( qgsDoubleToString( m_x ) ).arg( qgsDoubleToString( m_y ) );
}

double QgsPoint::sqrDist( double x, double y ) const
{
  return ( m_x - x ) * ( m_x - x ) + ( m_y - y ) * ( m_y - y );
}

double QgsPoint::sqrDist( const QgsPoint& other ) const
{
  return sqrDist( other.x(), other.y() );
}

double QgsPoint::azimuth( const QgsPoint& other )
{
  double dx = other.x() - m_x;
  double dy = other.y() - m_y;
  return ( atan2( dx, dy ) * 180.0 / M_PI );
}

// operators
bool QgsPoint::operator==( const QgsPoint & other )
{
  if (( m_x == other.x() ) && ( m_y == other.y() ) )
    return true;
  else
    return false;
}

bool QgsPoint::operator!=( const QgsPoint & other ) const
{
  if (( m_x == other.x() ) && ( m_y == other.y() ) )
    return false;
  else
    return true;
}

QgsPoint & QgsPoint::operator=( const QgsPoint & other )
{
  if ( &other != this )
  {
    m_x = other.x();
    m_y = other.y();
  }

  return *this;
}

void QgsPoint::multiply( const double& scalar )
{
  m_x *= scalar;
  m_y *= scalar;
}

int QgsPoint::onSegment( const QgsPoint& a, const QgsPoint& b ) const
{
  //algorithm from 'graphics GEMS', A. Paeth: 'A Fast 2D Point-on-line test'
  if (
    qAbs(( b.y() - a.y() ) *( m_x - a.x() ) - ( m_y - a.y() ) *( b.x() - a.x() ) )
    >= qMax( qAbs( b.x() - a.x() ), qAbs( b.y() - a.y() ) )
  )
  {
    return 0;
  }
  if (( b.x() < a.x() && a.x() < m_x ) || ( b.y() < a.y() && a.y() < m_y ) )
  {
    return 1;
  }
  if (( m_x < a.x() && a.x() < b.x() ) || ( m_y < a.y() && a.y() < b.y() ) )
  {
    return 1;
  }
  if (( a.x() < b.x() && b.x() < m_x ) || ( a.y() < b.y() && b.y() < m_y ) )
  {
    return 3;
  }
  if (( m_x < b.x() && b.x() < a.x() ) || ( m_y < b.y() && b.y() < a.y() ) )
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
  t = ( m_x * ny - m_y * nx - x1 * ny + y1 * nx ) / (( x2 - x1 ) * ny - ( y2 - y1 ) * nx );

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
    minDistPoint.setX( m_x );
    minDistPoint.setY( m_y );
    return 0.0;
  }
  return dist;
}
