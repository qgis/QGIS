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

QgsPolygon::QgsPolygon()
{
  mWkbType = QgsWkbTypes::Polygon;
}

///@cond DOXYGEN_SHUTTUP
QgsPolygon::QgsPolygon( QgsLineString *exterior, const QList<QgsLineString *> &rings )
{
  setExteriorRing( exterior );
  for ( QgsLineString *ring : rings )
  {
    addInteriorRing( ring );
  }
  clearCache();
}
///@endcond

QString QgsPolygon::geometryType() const
{
  return QStringLiteral( "Polygon" );
}

QgsPolygon *QgsPolygon::createEmptyWithSameType() const
{
  auto result = std::make_unique< QgsPolygon >();
  result->mWkbType = mWkbType;
  return result.release();
}

QgsPolygon *QgsPolygon::clone() const
{
  return new QgsPolygon( *this );
}

void QgsPolygon::clear()
{
  QgsCurvePolygon::clear();
  mWkbType = QgsWkbTypes::Polygon;
}

bool QgsPolygon::fromWkb( QgsConstWkbPtr &wkbPtr )
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
    std::unique_ptr< QgsLineString > line( new QgsLineString() );
    line->fromWkbPoints( ringType, wkbPtr );
    /*if ( !line->isRing() )
    {
      delete line; continue;
    }*/

    if ( !mExteriorRing )
    {
      mExteriorRing = std::move( line );
    }
    else
    {
      mInteriorRings.append( line.release() );
    }
  }

  return true;
}

int QgsPolygon::wkbSize( QgsAbstractGeometry::WkbFlags ) const
{
  int binarySize = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );

  // Endianness and WkbType is not stored for LinearRings
  if ( mExteriorRing )
  {
    binarySize += sizeof( quint32 ) + mExteriorRing->numPoints() * ( 2 + mExteriorRing->is3D() + mExteriorRing->isMeasure() ) * sizeof( double );
  }
  for ( const QgsCurve *curve : mInteriorRings )
  {
    binarySize += sizeof( quint32 ) + curve->numPoints() * ( 2 + curve->is3D() + curve->isMeasure() ) * sizeof( double );
  }

  return binarySize;
}

QByteArray QgsPolygon::asWkb( QgsAbstractGeometry::WkbFlags flags ) const
{
  QByteArray wkbArray;
  wkbArray.resize( QgsPolygon::wkbSize() );
  QgsWkbPtr wkb( wkbArray );
  wkb << static_cast<char>( QgsApplication::endian() );

  QgsWkbTypes::Type type = wkbType();
  if ( flags & FlagExportTrianglesAsPolygons )
  {
    switch ( type )
    {
      case QgsWkbTypes::Triangle:
        type = QgsWkbTypes::Polygon;
        break;
      case QgsWkbTypes::TriangleZ:
        type = QgsWkbTypes::PolygonZ;
        break;
      case QgsWkbTypes::TriangleM:
        type = QgsWkbTypes::PolygonM;
        break;
      case QgsWkbTypes::TriangleZM:
        type = QgsWkbTypes::PolygonZM;
        break;
      default:
        break;
    }
  }
  wkb << static_cast<quint32>( type );

  wkb << static_cast<quint32>( ( nullptr != mExteriorRing ) + mInteriorRings.size() );
  if ( mExteriorRing )
  {
    QgsPointSequence pts;
    mExteriorRing->points( pts );
    QgsGeometryUtils::pointsToWKB( wkb, pts, mExteriorRing->is3D(), mExteriorRing->isMeasure(), flags );
  }
  for ( const QgsCurve *curve : mInteriorRings )
  {
    QgsPointSequence pts;
    curve->points( pts );
    QgsGeometryUtils::pointsToWKB( wkb, pts, curve->is3D(), curve->isMeasure(), flags );
  }

  return wkbArray;
}

void QgsPolygon::addInteriorRing( QgsCurve *ring )
{
  if ( !ring )
    return;

  if ( ring->hasCurvedSegments() )
  {
    //can't add a curved ring to a QgsPolygonV2
    QgsLineString *segmented = ring->curveToLine();
    delete ring;
    ring = segmented;
  }

  QgsLineString *lineString = qgsgeometry_cast< QgsLineString *>( ring );
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

void QgsPolygon::setExteriorRing( QgsCurve *ring )
{
  if ( !ring )
  {
    return;
  }

  if ( ring->hasCurvedSegments() )
  {
    //need to segmentize ring as polygon does not support curves
    QgsCurve *line = ring->segmentize();
    delete ring;
    ring = line;
  }

  QgsLineString *lineString = qgsgeometry_cast< QgsLineString *>( ring );
  if ( lineString && !lineString->isClosed() )
  {
    lineString->close();
  }

  mExteriorRing.reset( ring );

  //set proper wkb type
  setZMTypeFromSubGeometry( ring, QgsWkbTypes::Polygon );

  //match dimensionality for rings
  for ( QgsCurve *ring : std::as_const( mInteriorRings ) )
  {
    ring->convertTo( mExteriorRing->wkbType() );
  }

  clearCache();
}

QgsAbstractGeometry *QgsPolygon::boundary() const
{
  if ( !mExteriorRing )
    return nullptr;

  if ( mInteriorRings.isEmpty() )
  {
    return mExteriorRing->clone();
  }
  else
  {
    QgsMultiLineString *multiLine = new QgsMultiLineString();
    int nInteriorRings = mInteriorRings.size();
    multiLine->reserve( nInteriorRings + 1 );
    multiLine->addGeometry( mExteriorRing->clone() );
    for ( int i = 0; i < nInteriorRings; ++i )
    {
      multiLine->addGeometry( mInteriorRings.at( i )->clone() );
    }
    return multiLine;
  }
}

double QgsPolygon::pointDistanceToBoundary( double x, double y ) const
{
  if ( !mExteriorRing )
    return std::numeric_limits< double >::quiet_NaN();

  bool inside = false;
  double minimumDistance = std::numeric_limits<double>::max();
  double minDistX = 0.0;
  double minDistY = 0.0;

  int numRings = mInteriorRings.size() + 1;
  for ( int ringIndex = 0; ringIndex < numRings; ++ringIndex )
  {
    const QgsLineString *ring = static_cast< const QgsLineString * >( ringIndex == 0 ? mExteriorRing.get() : mInteriorRings.at( ringIndex - 1 ) );

    int len = ring->numPoints() - 1; //assume closed
    for ( int i = 0, j = len - 1; i < len; j = i++ )
    {
      double aX = ring->xAt( i );
      double aY = ring->yAt( i );
      double bX = ring->xAt( j );
      double bY = ring->yAt( j );

      if ( ( ( aY > y ) != ( bY > y ) ) &&
           ( x < ( bX - aX ) * ( y - aY ) / ( bY - aY ) + aX ) )
        inside = !inside;

      minimumDistance = std::min( minimumDistance, QgsGeometryUtils::sqrDistToLine( x, y, aX, aY, bX, bY, minDistX, minDistY, 4 * std::numeric_limits<double>::epsilon() ) );
    }
  }

  return ( inside ? 1 : -1 ) * std::sqrt( minimumDistance );
}

QgsPolygon *QgsPolygon::surfaceToPolygon() const
{
  return clone();
}

QgsCurvePolygon *QgsPolygon::toCurveType() const
{
  QgsCurvePolygon *curvePolygon = new QgsCurvePolygon();
  if ( mExteriorRing )
  {
    curvePolygon->setExteriorRing( mExteriorRing->clone() );
    int nInteriorRings = mInteriorRings.size();
    for ( int i = 0; i < nInteriorRings; ++i )
    {
      curvePolygon->addInteriorRing( mInteriorRings.at( i )->clone() );
    }
  }
  return curvePolygon;
}
