/***************************************************************************
                         qgspolygonv2.cpp
                         ----------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspolygonv2.h"
#include "qgsapplication.h"
#include "qgsgeometryutils.h"
#include "qgslinestringv2.h"
#include "qgswkbptr.h"

QgsAbstractGeometryV2* QgsPolygonV2::clone() const
{
  return new QgsPolygonV2( *this );
}

bool QgsPolygonV2::fromWkb( const unsigned char* wkb )
{
  clear();
  if ( !wkb )
  {
    return false;
  }

  QgsConstWkbPtr wkbPtr( wkb );
  QgsWKBTypes::Type type = wkbPtr.readHeader();
  if ( QgsWKBTypes::flatType( type ) != QgsWKBTypes::Polygon )
  {
    return false;
  }
  mWkbType = type;

  int nRings;
  wkbPtr >> nRings;
  for ( int i = 0; i < nRings; ++i )
  {
    QgsLineStringV2* line = new QgsLineStringV2();
    line->fromWkbPoints( mWkbType, wkbPtr );
    if ( i == 0 )
    {
      mExteriorRing = line;
    }
    else
    {
      mInteriorRings.append( line );
    }
  }

  return true;
}

int QgsPolygonV2::wkbSize() const
{
  int size = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  if ( mExteriorRing )
  {
    // Endianness and WkbType is not stored for LinearRings
    size += mExteriorRing->wkbSize() - ( sizeof( char ) + sizeof( quint32 ) );
  }
  foreach ( const QgsCurveV2* curve, mInteriorRings )
  {
    // Endianness and WkbType is not stored for LinearRings
    size += curve->wkbSize() - ( sizeof( char ) + sizeof( quint32 ) );
  }
  return size;
}

unsigned char* QgsPolygonV2::asWkb( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>(( mExteriorRing != 0 ) + mInteriorRings.size() );
  if ( mExteriorRing )
  {
    QList<QgsPointV2> pts;
    mExteriorRing->points( pts );
    QgsGeometryUtils::pointsToWKB( wkb, pts, mExteriorRing->is3D(), mExteriorRing->isMeasure() );
  }
  foreach ( const QgsCurveV2* curve, mInteriorRings )
  {
    QList<QgsPointV2> pts;
    curve->points( pts );
    QgsGeometryUtils::pointsToWKB( wkb, pts, curve->is3D(), curve->isMeasure() );
  }
  return geomPtr;
}

QgsPolygonV2* QgsPolygonV2::surfaceToPolygon() const
{
  return dynamic_cast<QgsPolygonV2*>( clone() );
}
