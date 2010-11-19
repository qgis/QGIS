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
/* $Id$ */


#include "qgspoint.h"
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
  QString rep = QString::number( m_x, 'f', thePrecision ) + QString( "," ) +
                QString::number( m_y, 'f', thePrecision );
  return rep;
}

QString QgsPoint::toDegreesMinutesSeconds( int thePrecision ) const
{
  int myDegreesX = int( qAbs( m_x ) );
  float myFloatMinutesX = float(( qAbs( m_x ) - myDegreesX ) * 60 );
  int myIntMinutesX = int( myFloatMinutesX );
  float mySecondsX = float( myFloatMinutesX - myIntMinutesX ) * 60;

  int myDegreesY = int( qAbs( m_y ) );
  float myFloatMinutesY = float(( qAbs( m_y ) - myDegreesY ) * 60 );
  int myIntMinutesY = int( myFloatMinutesY );
  float mySecondsY = float( myFloatMinutesY - myIntMinutesY ) * 60;

  QString myXHemisphere = m_x < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
  QString myYHemisphere = m_y < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  QString rep = QString::number( myDegreesX ) + QChar( 176 ) +
                QString::number( myIntMinutesX ) + QString( "'" ) +
                QString::number( mySecondsX, 'f', thePrecision ) + QString( "\"" ) +
                myXHemisphere + QString( "," ) +
                QString::number( myDegreesY ) + QChar( 176 ) +
                QString::number( myIntMinutesY ) + QString( "'" ) +
                QString::number( mySecondsY, 'f', thePrecision ) + QString( "\"" ) +
                myYHemisphere;
  return rep;
}


QString QgsPoint::wellKnownText() const
{
  return QString( "POINT(%1 %2)" ).arg( QString::number( m_x, 'f', 18 ) ).arg( QString::number( m_y, 'f', 18 ) );
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

double QgsPoint::sqrDistToSegment( double x1, double y1, double x2, double y2, QgsPoint& minDistPoint ) const
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

  return ( sqrDist( minDistPoint ) );
}
