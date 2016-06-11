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

QgsPolygonV2::QgsPolygonV2()
    : QgsCurvePolygonV2()
{
  mWkbType = QgsWKBTypes::Polygon;
}

bool QgsPolygonV2::operator==( const QgsPolygonV2& other ) const
{
  //run cheap checks first
  if ( mWkbType != other.mWkbType )
    return false;

  if (( !mExteriorRing && other.mExteriorRing ) || ( mExteriorRing && !other.mExteriorRing ) )
    return false;

  if ( mInteriorRings.count() != other.mInteriorRings.count() )
    return false;

  // compare rings
  if ( mExteriorRing && other.mExteriorRing )
  {
    if ( *mExteriorRing != *other.mExteriorRing )
      return false;
  }

  for ( int i = 0; i < mInteriorRings.count(); ++i )
  {
    if (( !mInteriorRings.at( i ) && other.mInteriorRings.at( i ) ) ||
        ( mInteriorRings.at( i ) && !other.mInteriorRings.at( i ) ) )
      return false;

    if ( mInteriorRings.at( i ) && other.mInteriorRings.at( i ) &&
         *mInteriorRings.at( i ) != *other.mInteriorRings.at( i ) )
      return false;
  }

  return true;
}

bool QgsPolygonV2::operator!=( const QgsPolygonV2& other ) const
{
  return !operator==( other );
}

QgsPolygonV2* QgsPolygonV2::clone() const
{
  return new QgsPolygonV2( *this );
}

void QgsPolygonV2::clear()
{
  QgsCurvePolygonV2::clear();
  mWkbType = QgsWKBTypes::Polygon;
}

bool QgsPolygonV2::fromWkb( QgsConstWkbPtr wkbPtr )
{
  clear();
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWKBTypes::Type type = wkbPtr.readHeader();
  if ( QgsWKBTypes::flatType( type ) != QgsWKBTypes::Polygon )
  {
    return false;
  }
  mWkbType = type;

  QgsWKBTypes::Type ringType;
  switch ( mWkbType )
  {
    case QgsWKBTypes::PolygonZ:
      ringType = QgsWKBTypes::LineStringZ;
      break;
    case QgsWKBTypes::PolygonM:
      ringType = QgsWKBTypes::LineStringM;
      break;
    case QgsWKBTypes::PolygonZM:
      ringType = QgsWKBTypes::LineStringZM;
      break;
    case QgsWKBTypes::Polygon25D:
      ringType = QgsWKBTypes::LineString25D;
      break;
    default:
      ringType = QgsWKBTypes::LineString;
      break;
  }

  int nRings;
  wkbPtr >> nRings;
  for ( int i = 0; i < nRings; ++i )
  {
    QgsLineStringV2* line = new QgsLineStringV2();
    line->fromWkbPoints( ringType, wkbPtr );
    /*if ( !line->isRing() )
    {
      delete line; continue;
    }*/

    if ( !mExteriorRing )
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
  Q_FOREACH ( const QgsCurveV2* curve, mInteriorRings )
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
  QgsWkbPtr wkb( geomPtr, binarySize );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>(( nullptr != mExteriorRing ) + mInteriorRings.size() );
  if ( mExteriorRing )
  {
    QgsPointSequenceV2 pts;
    mExteriorRing->points( pts );
    QgsGeometryUtils::pointsToWKB( wkb, pts, mExteriorRing->is3D(), mExteriorRing->isMeasure() );
  }
  Q_FOREACH ( const QgsCurveV2* curve, mInteriorRings )
  {
    QgsPointSequenceV2 pts;
    curve->points( pts );
    QgsGeometryUtils::pointsToWKB( wkb, pts, curve->is3D(), curve->isMeasure() );
  }

  return geomPtr;
}

void QgsPolygonV2::addInteriorRing( QgsCurveV2* ring )
{
  if ( !ring )
    return;

  if ( ring->hasCurvedSegments() )
  {
    //can't add a curved ring to a QgsPolygonV2
    QgsLineStringV2* segmented = ring->curveToLine();
    delete ring;
    ring = segmented;
  }

  QgsLineStringV2* lineString = dynamic_cast< QgsLineStringV2*>( ring );
  if ( lineString && !lineString->isClosed() )
  {
    lineString->close();
  }

  if ( mWkbType == QgsWKBTypes::Polygon25D )
  {
    ring->convertTo( QgsWKBTypes::LineString25D );
    mInteriorRings.append( ring );
  }
  else
  {
    QgsCurvePolygonV2::addInteriorRing( ring );
  }
  clearCache();
}

void QgsPolygonV2::setExteriorRing( QgsCurveV2* ring )
{
  if ( !ring )
  {
    return;
  }
  delete mExteriorRing;

  if ( ring->hasCurvedSegments() )
  {
    //need to segmentize ring as polygon does not support curves
    QgsCurveV2* line = ring->segmentize();
    delete ring;
    ring = line;
  }

  QgsLineStringV2* lineString = dynamic_cast< QgsLineStringV2*>( ring );
  if ( lineString && !lineString->isClosed() )
  {
    lineString->close();
  }

  mExteriorRing = ring;

  //set proper wkb type
  setZMTypeFromSubGeometry( ring, QgsWKBTypes::Polygon );

  //match dimensionality for rings
  Q_FOREACH ( QgsCurveV2* ring, mInteriorRings )
  {
    ring->convertTo( mExteriorRing->wkbType() );
  }

  clearCache();
}

QgsPolygonV2* QgsPolygonV2::surfaceToPolygon() const
{
  return clone();
}

QgsAbstractGeometryV2* QgsPolygonV2::toCurveType() const
{
  QgsCurvePolygonV2* curvePolygon = new QgsCurvePolygonV2();
  curvePolygon->setExteriorRing( mExteriorRing->clone() );
  int nInteriorRings = mInteriorRings.size();
  for ( int i = 0; i < nInteriorRings; ++i )
  {
    curvePolygon->addInteriorRing( mInteriorRings.at( i )->clone() );
  }
  return curvePolygon;
}
