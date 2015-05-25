/***************************************************************************
                        qgsabstractgeometryv2.cpp
  -------------------------------------------------------------------
Date                 : 04 Sept 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsabstractgeometryv2.h"
#include "qgswkbptr.h"
#include "qgsgeos.h"
#include "qgsmaptopixel.h"
#include <limits>
#include <QTransform>

QgsAbstractGeometryV2::QgsAbstractGeometryV2(): mWkbType( QgsWKBTypes::Unknown )
{
}

QgsAbstractGeometryV2::~QgsAbstractGeometryV2()
{
}

QgsAbstractGeometryV2::QgsAbstractGeometryV2( const QgsAbstractGeometryV2& geom )
{
  mWkbType = geom.mWkbType;
}

QgsAbstractGeometryV2& QgsAbstractGeometryV2::operator=( const QgsAbstractGeometryV2 & geom )
{
  if ( &geom != this )
  {
    clear();
    mWkbType = geom.mWkbType;
  }
  return *this;
}

QgsRectangle QgsAbstractGeometryV2::boundingBox() const
{
  if ( mBoundingBox.isNull() )
  {
    mBoundingBox = calculateBoundingBox();
  }
  return mBoundingBox;
}

bool QgsAbstractGeometryV2::is3D() const
{
  return(( mWkbType >= 1001 && mWkbType <= 1012 ) || ( mWkbType > 3000 || mWkbType & 0x80000000 ) );
}

bool QgsAbstractGeometryV2::isMeasure() const
{
  return ( mWkbType >= 2001 && mWkbType <= 3012 );
}

void QgsAbstractGeometryV2::setZMTypeFromSubGeometry( const QgsAbstractGeometryV2* subgeom, QgsWKBTypes::Type baseGeomType )
{
  if ( !subgeom )
  {
    return;
  }

  bool hasZ = subgeom->is3D();
  bool hasM = subgeom->isMeasure();

  if ( hasZ && hasM )
  {
    mWkbType = ( QgsWKBTypes::Type )( baseGeomType + 3000 );
  }
  else if ( hasZ )
  {
    mWkbType = ( QgsWKBTypes::Type )( baseGeomType + 1000 );
  }
  else if ( hasM )
  {
    mWkbType = ( QgsWKBTypes::Type )( baseGeomType + 2000 );
  }
  else
  {
    mWkbType = baseGeomType;
  }
}

QgsRectangle QgsAbstractGeometryV2::calculateBoundingBox() const
{
  double xmin = std::numeric_limits<double>::max();
  double ymin = std::numeric_limits<double>::max();
  double xmax = -std::numeric_limits<double>::max();
  double ymax = -std::numeric_limits<double>::max();

  QgsVertexId id;
  QgsPointV2 vertex;
  double x, y;
  while ( nextVertex( id, vertex ) )
  {
    x = vertex.x();
    y = vertex.y();
    if ( x < xmin )
      xmin = x;
    if ( x > xmax )
      xmax = x;
    if ( y < ymin )
      ymin = y;
    if ( y > ymax )
      ymax = y;
  }

  return QgsRectangle( xmin, ymin, xmax, ymax );
}

QgsPointV2 QgsAbstractGeometryV2::vertexAt( const QgsVertexId& id ) const
{
  QList< QList< QList< QgsPointV2 > > > coordinates;
  coordinateSequence( coordinates );

  if ( id.part >= coordinates.size() )
  {
    return QgsPointV2();
  }
  const QList< QList< QgsPointV2 > >& part = coordinates.at( id.part );
  if ( id.ring >= part.size() )
  {
    return QgsPointV2();
  }
  const QList< QgsPointV2 >& ring = part.at( id.ring );
  if ( id.vertex >= ring.size() )
  {
    return QgsPointV2();
  }
  return ring.at( id.vertex );
}

int QgsAbstractGeometryV2::nCoordinates() const
{
  QList< QList< QList< QgsPointV2 > > > coordinates;
  coordinateSequence( coordinates );
  int nCoords = 0;

  QList< QList< QList< QgsPointV2 > > >::const_iterator partIt = coordinates.constBegin();
  for ( ; partIt != coordinates.constEnd(); ++partIt )
  {
    const QList< QList< QgsPointV2 > >& part = *partIt;
    QList< QList< QgsPointV2 > >::const_iterator ringIt = part.constBegin();
    for ( ; ringIt != part.constEnd(); ++ringIt )
    {
      nCoords += ringIt->size();
    }
  }

  return nCoords;
}

QString QgsAbstractGeometryV2::wktTypeStr() const
{
  QString wkt = geometryType();
  if ( is3D() )
    wkt += "Z";
  if ( isMeasure() )
    wkt += "M";
  return wkt;
}

bool QgsAbstractGeometryV2::readWkbHeader( QgsConstWkbPtr& wkbPtr, QgsWKBTypes::Type& wkbType, bool& endianSwap, QgsWKBTypes::Type expectedType )
{
  if ( !static_cast<const unsigned char*>( wkbPtr ) )
  {
    return false;
  }

  char wkbEndian;
  wkbPtr >> wkbEndian;
  endianSwap = wkbEndian != QgsApplication::endian();

  wkbPtr >> wkbType;
  if ( endianSwap )
    QgsApplication::endian_swap( wkbType );

  if ( QgsWKBTypes::flatType( wkbType ) != expectedType )
  {
    wkbType = QgsWKBTypes::Unknown;
    return false;
  }
  return true;
}

bool QgsAbstractGeometryV2::isEmpty() const
{
  QgsVertexId vId; QgsPointV2 vertex;
  return !nextVertex( vId, vertex );
}
