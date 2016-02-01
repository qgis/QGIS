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

// @deprecated
QgsConstWkbPtr::QgsConstWkbPtr( const unsigned char *p ): mEndianSwap( false )
{
  mP = const_cast< unsigned char * >( p );
  mE = std::numeric_limits<unsigned char *>::max();
}

QgsConstWkbPtr::QgsConstWkbPtr( const unsigned char *p, unsigned int l ): mEndianSwap( false )
{
  mP = const_cast< unsigned char * >( p );
  mE = mP + l;
}

QgsWKBTypes::Type QgsConstWkbPtr::readHeader() const
{
  if ( !mP )
  {
    return QgsWKBTypes::Unknown;
  }

  char wkbEndian;
  ( *this ) >> wkbEndian;
  mEndianSwap = ( wkbEndian != QgsApplication::endian() );

  QgsWKBTypes::Type wkbType;
  ( *this ) >> wkbType;
  if ( mEndianSwap )
  {
    QgsApplication::endian_swap( wkbType );
  }
  return wkbType;
}
