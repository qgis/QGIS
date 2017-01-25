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

QgsTriangle::QgsTriangle(const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &p3)
{
  mWkbType = QgsWkbTypes::Triangle;

  //TODO: test colinear, test distinct points
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << p1 << p2 << p3 << p1 );
  setExteriorRing( ext );

}

QgsTriangle *QgsTriangle::clone() const
{
    return new QgsTriangle( *this );
}

bool QgsTriangle::fromWkb(QgsConstWkbPtr &wkb)
{
    Q_UNUSED( wkb );
    return false;
}

bool QgsTriangle::fromWkt(const QString &wkt)
{
    Q_UNUSED( wkt );
    return false;
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

void QgsTriangle::addInteriorRing(QgsCurve *ring)
{
    Q_UNUSED( ring );
    return;
}

void QgsTriangle::setExteriorRing(QgsCurve *ring)
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
