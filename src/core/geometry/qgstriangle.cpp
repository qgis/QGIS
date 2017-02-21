/***************************************************************************
                         qgstriangle.cpp
                         -------------------
    begin                : January 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstriangle.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgswkbptr.h"

QgsTriangle::QgsTriangle()
    : QgsPolygonV2()
{
  mWkbType = QgsWkbTypes::Triangle;
}

QgsTriangle::QgsTriangle( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &p3 )
{
  mWkbType = QgsWkbTypes::Triangle;

  //TODO: test colinear, test distinct points
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << p1 << p2 << p3 << p1 );
  setExteriorRing( ext );

}

void QgsTriangle::clear()
{
  QgsCurvePolygon::clear();
  mWkbType = QgsWkbTypes::Triangle;
}

QgsTriangle *QgsTriangle::clone() const
{
  return new QgsTriangle( *this );
}

bool QgsTriangle::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  clear();
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWkbTypes::Type type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::Triangle )
  {
    return false;
  }
  mWkbType = type;

  QgsWkbTypes::Type ringType;
  switch ( mWkbType )
  {
    case QgsWkbTypes::TriangleZ:
      ringType = QgsWkbTypes::LineStringZ;
      break;
    case QgsWkbTypes::TriangleM:
      ringType = QgsWkbTypes::LineStringM;
      break;
    case QgsWkbTypes::TriangleZM:
      ringType = QgsWkbTypes::LineStringZM;
      break;
    case QgsWkbTypes::Triangle25D:
      ringType = QgsWkbTypes::LineString25D;
      break;
    default:
      ringType = QgsWkbTypes::LineString;
      break;
  }

  int nRings;
  wkbPtr >> nRings;
  if ( nRings > 1 )
  {
    return false;
  }

  QgsLineString* line = new QgsLineString();
  line->fromWkbPoints( ringType, wkbPtr );
  if ( !mExteriorRing )
  {
    mExteriorRing = line;
  }

  return true;
}

bool QgsTriangle::fromWkt( const QString &wkt )
{

  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::geometryType( parts.first ) != QgsWkbTypes::PolygonGeometry )
    return false;

  mWkbType = parts.first;

  QString defaultChildWkbType = QStringLiteral( "LineString%1%2" ).arg( is3D() ? "Z" : "", isMeasure() ? "M" : "" );

  Q_FOREACH ( const QString& childWkt, QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType ) )
  {
    QPair<QgsWkbTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    QgsWkbTypes::Type flatCurveType = QgsWkbTypes::flatType( childParts.first );
    if ( flatCurveType == QgsWkbTypes::LineString )
      mInteriorRings.append( new QgsLineString() );
    else
    {
      clear();
      return false;
    }
    if ( !mInteriorRings.back()->fromWkt( childWkt ) )
    {
      clear();
      return false;
    }
  }

  if ( mInteriorRings.isEmpty() )
  {
    clear();
    return false;
  }
  mExteriorRing = mInteriorRings.at( 0 );
  mInteriorRings.removeFirst();

  //scan through rings and check if dimensionality of rings is different to CurvePolygon.
  //if so, update the type dimensionality of the CurvePolygon to match
  bool hasZ = false;
  bool hasM = false;
  if ( mExteriorRing )
  {
    hasZ = hasZ || mExteriorRing->is3D();
    hasM = hasM || mExteriorRing->isMeasure();
  }
  if ( hasZ )
    addZValue( 0 );
  if ( hasM )
    addMValue( 0 );

  return true;
}

/*
QByteArray QgsTriangle::asWkb() const
{
    return QByteArray();
}*/

QgsAbstractGeometry *QgsTriangle::toCurveType() const
{
  QgsCurvePolygon* curvePolygon = new QgsCurvePolygon();
  curvePolygon->setExteriorRing( mExteriorRing->clone() );

  return curvePolygon;
}

void QgsTriangle::addInteriorRing( QgsCurve *ring )
{
  Q_UNUSED( ring );
  return;
}

void QgsTriangle::setExteriorRing( QgsCurve *ring )
{
  if ( !ring )
  {
    return;
  }

  if ( ring->hasCurvedSegments() )
  {
    //need to segmentize ring as polygon does not support curves
    QgsCurve* line = ring->segmentize();
    delete ring;
    ring = line;
  }

  if ( ring->numPoints() == 4 )
  {
    if ( !ring->isClosed() )
    {
      return;
    }
  }
  else if ( ring->numPoints() == 3 )
  {
    if ( ring->isClosed() )
    {
      return;
    }
    QgsLineString* lineString = dynamic_cast< QgsLineString*>( ring );
    if ( lineString && !lineString->isClosed() )
    {
      lineString->close();
    }
    ring = lineString;
  }

  delete mExteriorRing;

  mExteriorRing = ring;

  //set proper wkb type
  setZMTypeFromSubGeometry( ring, QgsWkbTypes::Triangle );

  clearCache();
}

QgsAbstractGeometry *QgsTriangle::boundary() const
{
  if ( !mExteriorRing )
    return nullptr;

  return mExteriorRing->clone();
}
