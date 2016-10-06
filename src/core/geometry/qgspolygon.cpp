/***************************************************************************
                         qgspolygon.cpp
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

#include "qgspolygon.h"
#include "qgsapplication.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgswkbptr.h"

QgsPolygonV2::QgsPolygonV2()
    : QgsCurvePolygon()
{
  mWkbType = QgsWkbTypes::Polygon;
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
  QgsCurvePolygon::clear();
  mWkbType = QgsWkbTypes::Polygon;
}

bool QgsPolygonV2::fromWkb( QgsConstWkbPtr wkbPtr )
{
  clear();
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWkbTypes::Type type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::Polygon )
  {
    return false;
  }
  mWkbType = type;

  QgsWkbTypes::Type ringType;
  switch ( mWkbType )
  {
    case QgsWkbTypes::PolygonZ:
      ringType = QgsWkbTypes::LineStringZ;
      break;
    case QgsWkbTypes::PolygonM:
      ringType = QgsWkbTypes::LineStringM;
      break;
    case QgsWkbTypes::PolygonZM:
      ringType = QgsWkbTypes::LineStringZM;
      break;
    case QgsWkbTypes::Polygon25D:
      ringType = QgsWkbTypes::LineString25D;
      break;
    default:
      ringType = QgsWkbTypes::LineString;
      break;
  }

  int nRings;
  wkbPtr >> nRings;
  for ( int i = 0; i < nRings; ++i )
  {
    QgsLineString* line = new QgsLineString();
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
  Q_FOREACH ( const QgsCurve* curve, mInteriorRings )
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
    QgsPointSequence pts;
    mExteriorRing->points( pts );
    QgsGeometryUtils::pointsToWKB( wkb, pts, mExteriorRing->is3D(), mExteriorRing->isMeasure() );
  }
  Q_FOREACH ( const QgsCurve* curve, mInteriorRings )
  {
    QgsPointSequence pts;
    curve->points( pts );
    QgsGeometryUtils::pointsToWKB( wkb, pts, curve->is3D(), curve->isMeasure() );
  }

  return geomPtr;
}

void QgsPolygonV2::addInteriorRing( QgsCurve* ring )
{
  if ( !ring )
    return;

  if ( ring->hasCurvedSegments() )
  {
    //can't add a curved ring to a QgsPolygonV2
    QgsLineString* segmented = ring->curveToLine();
    delete ring;
    ring = segmented;
  }

  QgsLineString* lineString = dynamic_cast< QgsLineString*>( ring );
  if ( lineString && !lineString->isClosed() )
  {
    lineString->close();
  }

  if ( mWkbType == QgsWkbTypes::Polygon25D )
  {
    ring->convertTo( QgsWkbTypes::LineString25D );
    mInteriorRings.append( ring );
  }
  else
  {
    QgsCurvePolygon::addInteriorRing( ring );
  }
  clearCache();
}

void QgsPolygonV2::setExteriorRing( QgsCurve* ring )
{
  if ( !ring )
  {
    return;
  }
  delete mExteriorRing;

  if ( ring->hasCurvedSegments() )
  {
    //need to segmentize ring as polygon does not support curves
    QgsCurve* line = ring->segmentize();
    delete ring;
    ring = line;
  }

  QgsLineString* lineString = dynamic_cast< QgsLineString*>( ring );
  if ( lineString && !lineString->isClosed() )
  {
    lineString->close();
  }

  mExteriorRing = ring;

  //set proper wkb type
  setZMTypeFromSubGeometry( ring, QgsWkbTypes::Polygon );

  //match dimensionality for rings
  Q_FOREACH ( QgsCurve* ring, mInteriorRings )
  {
    ring->convertTo( mExteriorRing->wkbType() );
  }

  clearCache();
}

QgsAbstractGeometry* QgsPolygonV2::boundary() const
{
  if ( !mExteriorRing )
    return nullptr;

  if ( mInteriorRings.isEmpty() )
  {
    return mExteriorRing->clone();
  }
  else
  {
    QgsMultiLineString* multiLine = new QgsMultiLineString();
    multiLine->addGeometry( mExteriorRing->clone() );
    int nInteriorRings = mInteriorRings.size();
    for ( int i = 0; i < nInteriorRings; ++i )
    {
      multiLine->addGeometry( mInteriorRings.at( i )->clone() );
    }
    return multiLine;
  }
}

QgsPolygonV2* QgsPolygonV2::surfaceToPolygon() const
{
  return clone();
}

QgsAbstractGeometry* QgsPolygonV2::toCurveType() const
{
  QgsCurvePolygon* curvePolygon = new QgsCurvePolygon();
  curvePolygon->setExteriorRing( mExteriorRing->clone() );
  int nInteriorRings = mInteriorRings.size();
  for ( int i = 0; i < nInteriorRings; ++i )
  {
    curvePolygon->addInteriorRing( mInteriorRings.at( i )->clone() );
  }
  return curvePolygon;
}
