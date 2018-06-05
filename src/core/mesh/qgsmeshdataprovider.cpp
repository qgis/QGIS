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

QgsMeshDataProvider::QgsMeshDataProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options )
  : QgsDataProvider( uri, options )
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
  : mX( x ), mY( y )
{}

QgsMeshDatasetValue::QgsMeshDatasetValue( double scalar )
  : mX( scalar )
{}

double QgsMeshDatasetValue::scalar() const
{
  if ( std::isnan( mY ) )
  {
    return mX;
  }
  else if ( std::isnan( mX ) )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  else
  {
    return std::sqrt( ( mX ) * ( mX ) + ( mY ) * ( mY ) );
  }
}

void QgsMeshDatasetValue::set( double scalar )
{
  setX( scalar );
}

void QgsMeshDatasetValue::setX( double x )
{
  mX = x;
}

void QgsMeshDatasetValue::setY( double y )
{
  mY = y;
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
  bool equal = std::isnan( mX ) == std::isnan( other.x() );
  equal &= std::isnan( mY ) == std::isnan( other.y() );

  if ( equal )
  {
    if ( std::isnan( mY ) )
    {
      equal &= qgsDoubleNear( other.x(), mX, 1E-8 );
    }
    else
    {
      equal &= qgsDoubleNear( other.x(), mX, 1E-8 );
      equal &= qgsDoubleNear( other.y(), mY, 1E-8 );
    }
  }
  return equal;
}

QgsMeshDatasetMetadata::QgsMeshDatasetMetadata(
  bool isScalar,
  bool isValid,
  bool isOnVertices,
  const QMap<QString, QString> &extraOptions )
  : mIsScalar( isScalar )
  , mIsValid( isValid )
  , mIsOnVertices( isOnVertices )
  , mExtraOptions( extraOptions )
{
}

QMap<QString, QString> QgsMeshDatasetMetadata::extraOptions() const
{
  return mExtraOptions;
}

bool QgsMeshDatasetMetadata::isVector() const
{
  return !mIsScalar;
}

bool QgsMeshDatasetMetadata::isScalar() const
{
  return mIsScalar;
}

bool QgsMeshDatasetMetadata::isValid() const
{
  return mIsValid;
}


bool QgsMeshDatasetMetadata::isOnVertices() const
{
  return mIsOnVertices;
}
