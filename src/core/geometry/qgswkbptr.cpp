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

QgsWkbPtr::QgsWkbPtr( unsigned char *p, int size )
{
  mP = p;
  mStart = mP;
  mEnd = mP + size;
}

void QgsWkbPtr::verifyBound( int size ) const
{
  if ( !mP || mP + size > mEnd )
    throw QgsWkbException( "wkb access out of bounds" );
}

QgsConstWkbPtr::QgsConstWkbPtr( const unsigned char *p, int size )
{
  mP = const_cast< unsigned char * >( p );
  mEnd = mP + size;
  mEndianSwap = false;
  mWkbType = QgsWKBTypes::Unknown;
}

QgsWKBTypes::Type QgsConstWkbPtr::readHeader() const
{
  if ( !mP )
    return QgsWKBTypes::Unknown;

  char wkbEndian;
  *this >> wkbEndian;
  mEndianSwap = wkbEndian != QgsApplication::endian();

  int wkbType;
  *this >> wkbType;
  mWkbType = static_cast<QgsWKBTypes::Type>( wkbType );

  return mWkbType;
}

void QgsConstWkbPtr::verifyBound( int size ) const
{
  if ( !mP || mP + size > mEnd )
    throw QgsWkbException( "wkb access out of bounds" );
}

const QgsConstWkbPtr &QgsConstWkbPtr::operator>>( QPointF &point ) const
{
  read( point.rx() );
  read( point.ry() );
  return *this;
}

const QgsConstWkbPtr &QgsConstWkbPtr::operator>>( QPolygonF &points ) const
{
  int skipZM = ( QgsWKBTypes::coordDimensions( mWkbType ) - 2 ) * sizeof( double );
  Q_ASSERT( skipZM >= 0 );

  unsigned int nPoints;
  read( nPoints );

  points.resize( nPoints );
  QPointF* ptr = points.data();

  for ( unsigned int i = 0; i < nPoints; ++i, ++ptr )
  {
    read( ptr->rx() );
    read( ptr->ry() );
    mP += skipZM;
  }
  return *this;
}
