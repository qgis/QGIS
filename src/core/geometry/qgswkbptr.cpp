/***************************************************************************
    qgswkbptr.cpp
    ---------------------
    begin                : May 2015
    copyright            : (C) 2015 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswkbptr.h"
#include "qgsapplication.h"
#include "qgswkbtypes.h"

QgsWkbPtr::QgsWkbPtr( QByteArray &wkb )
{
  mP = reinterpret_cast<unsigned char *>( wkb.data() );
  mStart = mP;
  mEnd = mP + wkb.length();
}

QgsWkbPtr::QgsWkbPtr( unsigned char *p, int size )
{
  mP = p;
  mStart = mP;
  mEnd = mP + size;
}

void QgsWkbPtr::verifyBound( int size ) const
{
  if ( !mP || mP + size > mEnd )
    throw QgsWkbException( QStringLiteral( "wkb access out of bounds" ) );
}

QgsConstWkbPtr::QgsConstWkbPtr( const QByteArray &wkb )
{
  mP = reinterpret_cast< unsigned char * >( const_cast<char *>( wkb.constData() ) );
  mEnd = mP + wkb.length();
  mEndianSwap = false;
  mWkbType = Qgis::WkbType::Unknown;
}

QgsConstWkbPtr::QgsConstWkbPtr( const unsigned char *p, int size )
{
  mP = const_cast< unsigned char * >( p );
  mEnd = mP + size;
  mEndianSwap = false;
  mWkbType = Qgis::WkbType::Unknown;
}

Qgis::WkbType QgsConstWkbPtr::readHeader() const
{
  if ( !mP )
    return Qgis::WkbType::Unknown;

  char wkbEndian;
  *this >> wkbEndian;
  mEndianSwap = wkbEndian != QgsApplication::endian();

  int wkbType;
  *this >> wkbType;
  mWkbType = static_cast<Qgis::WkbType>( wkbType );

  return mWkbType;
}

void QgsConstWkbPtr::verifyBound( int size ) const
{
  if ( !mP || mP + size > mEnd )
    throw QgsWkbException( QStringLiteral( "wkb access out of bounds" ) );
}

const QgsConstWkbPtr &QgsConstWkbPtr::operator>>( QPointF &point ) const
{
  read( point.rx() );
  read( point.ry() );
  return *this;
}

const QgsConstWkbPtr &QgsConstWkbPtr::operator>>( QPolygonF &points ) const
{
  const int skipZM = ( QgsWkbTypes::coordDimensions( mWkbType ) - 2 ) * sizeof( double );
  Q_ASSERT( skipZM >= 0 );

  unsigned int nPoints;
  read( nPoints );

  points.resize( nPoints );
  QPointF *ptr = points.data();

  for ( unsigned int i = 0; i < nPoints; ++i, ++ptr )
  {
    read( ptr->rx() );
    read( ptr->ry() );
    mP += skipZM;
  }
  return *this;
}
