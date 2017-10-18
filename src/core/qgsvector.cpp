/***************************************************************************
  qgsvector.cpp - QgsVector

 ---------------------
 begin                : 24.2.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvector.h"
#include "qgis.h"
#include "qgsexception.h"

QgsVector::QgsVector( double x, double y )
  : mX( x )
  , mY( y )
{
}

QgsVector QgsVector::operator-() const
{
  return QgsVector( -mX, -mY );
}

QgsVector QgsVector::operator*( double scalar ) const
{
  return QgsVector( mX * scalar, mY * scalar );
}

QgsVector QgsVector::operator/( double scalar ) const
{
  return *this * ( 1.0 / scalar );
}

double QgsVector::operator*( QgsVector v ) const
{
  return mX * v.mX + mY * v.mY;
}

QgsVector QgsVector::operator+( QgsVector other ) const
{
  return QgsVector( mX + other.mX, mY + other.mY );
}

QgsVector &QgsVector::operator+=( QgsVector other )
{
  mX += other.mX;
  mY += other.mY;
  return *this;
}

QgsVector QgsVector::operator-( QgsVector other ) const
{
  return QgsVector( mX - other.mX, mY - other.mY );
}

QgsVector &QgsVector::operator-=( QgsVector other )
{
  mX -= other.mX;
  mY -= other.mY;
  return *this;
}

double QgsVector::length() const
{
  return std::sqrt( mX * mX + mY * mY );
}

double QgsVector::x() const
{
  return mX;
}

double QgsVector::y() const
{
  return mY;
}

QgsVector QgsVector::perpVector() const
{
  return QgsVector( -mY, mX );
}

double QgsVector::angle() const
{
  double angle = std::atan2( mY, mX );
  return angle < 0.0 ? angle + 2.0 * M_PI : angle;
}

double QgsVector::angle( QgsVector v ) const
{
  return v.angle() - angle();
}

QgsVector QgsVector::rotateBy( double rot ) const
{
  double angle = std::atan2( mY, mX ) + rot;
  double len = length();
  return QgsVector( len * std::cos( angle ), len * std::sin( angle ) );
}

QgsVector QgsVector::normalized() const
{
  double len = length();

  if ( len == 0.0 )
  {
    throw QgsException( QStringLiteral( "normalized vector of null vector undefined" ) );
  }

  return *this / len;
}

bool QgsVector::operator==( QgsVector other ) const
{
  return qgsDoubleNear( mX, other.mX ) && qgsDoubleNear( mY, other.mY );
}

bool QgsVector::operator!=( QgsVector other ) const
{
  return !qgsDoubleNear( mX, other.mX ) || !qgsDoubleNear( mY, other.mY );
}
