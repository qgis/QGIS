/***************************************************************************
                         qgsmeshdataprovider.cpp
                         -----------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshdataprovider.h"
#include "qgis.h"

QgsMeshDataProvider::QgsMeshDataProvider( const QString &uri )
  : QgsDataProvider( uri )
{
}


QgsRectangle QgsMeshDataProvider::extent() const
{
  QgsRectangle rec;
  rec.setMinimal();
  for ( int i = 0; i < vertexCount(); ++i )
  {
    QgsMeshVertex v = vertex( i );
    rec.setXMinimum( std::min( rec.xMinimum(), v.x() ) );
    rec.setYMinimum( std::min( rec.yMinimum(), v.y() ) );
    rec.setXMaximum( std::max( rec.xMaximum(), v.x() ) );
    rec.setYMaximum( std::max( rec.yMaximum(), v.y() ) );
  }
  return rec;

}

QgsMeshDatasetValue::QgsMeshDatasetValue( double x, double y )
{
  setX( x );
  setY( y );
}

QgsMeshDatasetValue::QgsMeshDatasetValue( double scalar )
{
  set( scalar );
}

void QgsMeshDatasetValue::setNodata( bool nodata )
{
  mIsNodata = nodata;
}

bool QgsMeshDatasetValue::isNodata() const
{return mIsNodata;}

bool QgsMeshDatasetValue::isScalar() const
{
  return mIsScalar;
}

double QgsMeshDatasetValue::scalar() const
{
  if ( isNodata() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  if ( isScalar() )
  {
    return mX;
  }
  else
  {
    return std::sqrt( ( mX ) * ( mX ) + ( mY ) * ( mY ) );
  }
}

void QgsMeshDatasetValue::set( double scalar )
{
  setX( scalar );
  mIsScalar = true;
}

void QgsMeshDatasetValue::setX( double x )
{
  mX = x;
  if ( std::isnan( x ) )
  {
    mIsNodata = true;
  }
}

void QgsMeshDatasetValue::setY( double y )
{
  mY = y;
  if ( std::isnan( y ) )
  {
    mIsScalar = true;
  }
}

double QgsMeshDatasetValue::x() const
{
  return mX;
}

double QgsMeshDatasetValue::y() const
{
  return mY;
}

bool QgsMeshDatasetValue::operator==( const QgsMeshDatasetValue &other ) const
{
  bool equal = true;
  if ( isNodata() )
    equal = other.isNodata();
  else
  {
    if ( isScalar() )
    {
      equal &= other.isScalar();
      equal &= qgsDoubleNear( other.x(), mX, 1E-8 );
    }
    else
    {
      equal &= !other.isScalar();
      equal &= qgsDoubleNear( other.x(), mX, 1E-8 );
      equal &= qgsDoubleNear( other.y(), mY, 1E-8 );
    }
  }
  return equal;
}
