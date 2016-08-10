
/***************************************************************************
                        qgsmultisurface.cpp
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
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

#include "qgsmultisurface.h"
#include "qgsapplication.h"
#include "qgsgeometryutils.h"
#include "qgssurface.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgscurvepolygon.h"
#include "qgsmulticurve.h"

QgsMultiSurface::QgsMultiSurface()
    : QgsGeometryCollection()
{
  mWkbType = QgsWkbTypes::MultiSurface;
}

QgsMultiSurface *QgsMultiSurface::clone() const
{
  return new QgsMultiSurface( *this );
}

bool QgsMultiSurface::fromWkt( const QString& wkt )
{
  return fromCollectionWkt( wkt,
                            QList<QgsAbstractGeometry*>() << new QgsPolygonV2 << new QgsCurvePolygon,
                            "Polygon" );
}

QDomElement QgsMultiSurface::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  // GML2 does not support curves
  QDomElement elemMultiPolygon = doc.createElementNS( ns, "MultiPolygon" );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsSurface*>( geom ) )
    {
      QgsPolygonV2* polygon = static_cast<const QgsSurface*>( geom )->surfaceToPolygon();

      QDomElement elemPolygonMember = doc.createElementNS( ns, "polygonMember" );
      elemPolygonMember.appendChild( polygon->asGML2( doc, precision, ns ) );
      elemMultiPolygon.appendChild( elemPolygonMember );

      delete polygon;
    }
  }

  return elemMultiPolygon;
}

QDomElement QgsMultiSurface::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiSurface = doc.createElementNS( ns, "MultiSurface" );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsSurface*>( geom ) )
    {
      QDomElement elemSurfaceMember = doc.createElementNS( ns, "surfaceMember" );
      elemSurfaceMember.appendChild( geom->asGML3( doc, precision, ns ) );
      elemMultiSurface.appendChild( elemSurfaceMember );
    }
  }

  return elemMultiSurface;
}

QString QgsMultiSurface::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QString json = "{\"type\": \"MultiPolygon\", \"coordinates\": [";
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsSurface*>( geom ) )
    {
      json += '[';

      QgsPolygonV2* polygon = static_cast<const QgsSurface*>( geom )->surfaceToPolygon();

      QgsLineString* exteriorLineString = polygon->exteriorRing()->curveToLine();
      QgsPointSequence exteriorPts;
      exteriorLineString->points( exteriorPts );
      json += QgsGeometryUtils::pointsToJSON( exteriorPts, precision ) + ", ";
      delete exteriorLineString;

      for ( int i = 0, n = polygon->numInteriorRings(); i < n; ++i )
      {
        QgsLineString* interiorLineString = polygon->interiorRing( i )->curveToLine();
        QgsPointSequence interiorPts;
        interiorLineString->points( interiorPts );
        json += QgsGeometryUtils::pointsToJSON( interiorPts, precision ) + ", ";
        delete interiorLineString;
      }
      if ( json.endsWith( ", " ) )
      {
        json.chop( 2 ); // Remove last ", "
      }

      delete polygon;

      json += "], ";
    }
  }
  if ( json.endsWith( ", " ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += "] }";
  return json;
}

bool QgsMultiSurface::addGeometry( QgsAbstractGeometry* g )
{
  if ( !dynamic_cast<QgsSurface*>( g ) )
  {
    delete g;
    return false;
  }

  setZMTypeFromSubGeometry( g, QgsWkbTypes::MultiSurface );
  return QgsGeometryCollection::addGeometry( g );
}

QgsAbstractGeometry* QgsMultiSurface::boundary() const
{
  QgsMultiCurve* multiCurve = new QgsMultiCurve();
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    if ( QgsSurface* surface = dynamic_cast<QgsSurface*>( mGeometries.at( i ) ) )
    {
      multiCurve->addGeometry( surface->boundary() );
    }
  }
  if ( multiCurve->numGeometries() == 0 )
  {
    delete multiCurve;
    return nullptr;
  }
  return multiCurve;
}
