
/***************************************************************************
                        qgsmultisurfacev2.cpp
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

#include "qgsmultisurfacev2.h"
#include "qgsapplication.h"
#include "qgsgeometryutils.h"
#include "qgssurfacev2.h"
#include "qgslinestringv2.h"
#include "qgspolygonv2.h"
#include "qgscurvepolygonv2.h"

QgsAbstractGeometryV2 *QgsMultiSurfaceV2::clone() const
{
  return new QgsMultiSurfaceV2( *this );
}

bool QgsMultiSurfaceV2::fromWkt( const QString& wkt )
{
  return fromCollectionWkt( wkt,
                            QList<QgsAbstractGeometryV2*>() << new QgsPolygonV2 << new QgsCurvePolygonV2,
                            "Polygon" );
}

QDomElement QgsMultiSurfaceV2::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  // GML2 does not support curves
  QDomElement elemMultiPolygon = doc.createElementNS( ns, "MultiPolygon" );
  foreach ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsSurfaceV2*>( geom ) )
    {
      QgsPolygonV2* polygon = static_cast<const QgsSurfaceV2*>( geom )->surfaceToPolygon();

      QDomElement elemPolygonMember = doc.createElementNS( ns, "polygonMember" );
      elemPolygonMember.appendChild( polygon->asGML2( doc, precision, ns ) );
      elemMultiPolygon.appendChild( elemPolygonMember );

      delete polygon;
    }
  }

  return elemMultiPolygon;
}

QDomElement QgsMultiSurfaceV2::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiSurface = doc.createElementNS( ns, "MultiSurface" );
  foreach ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsSurfaceV2*>( geom ) )
    {
      QDomElement elemSurfaceMember = doc.createElementNS( ns, "surfaceMember" );
      elemSurfaceMember.appendChild( geom->asGML3( doc, precision, ns ) );
      elemMultiSurface.appendChild( elemSurfaceMember );
    }
  }

  return elemMultiSurface;
}

QString QgsMultiSurfaceV2::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QString json = "{\"type\": \"MultiPolygon\", \"coordinates\": [";
  foreach ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsSurfaceV2*>( geom ) )
    {
      json += "[";

      QgsPolygonV2* polygon = static_cast<const QgsSurfaceV2*>( geom )->surfaceToPolygon();

      QgsLineStringV2* exteriorLineString = polygon->exteriorRing()->curveToLine();
      QList<QgsPointV2> exteriorPts;
      exteriorLineString->points( exteriorPts );
      json += QgsGeometryUtils::pointsToJSON( exteriorPts, precision ) + ", ";
      delete exteriorLineString;

      for ( int i = 0, n = polygon->numInteriorRings(); i < n; ++i )
      {
        QgsLineStringV2* interiorLineString = polygon->interiorRing( i )->curveToLine();
        QList<QgsPointV2> interiorPts;
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

bool QgsMultiSurfaceV2::addGeometry( QgsAbstractGeometryV2* g )
{
  if ( !dynamic_cast<QgsSurfaceV2*>( g ) )
  {
    delete g;
    return false;
  }

  setZMTypeFromSubGeometry( g, QgsWKBTypes::MultiSurface );
  return QgsGeometryCollectionV2::addGeometry( g );
}

QgsAbstractGeometryV2* QgsMultiSurfaceV2::segmentize() const
{
  QgsMultiSurfaceV2* c = new QgsMultiSurfaceV2();
  QVector< QgsAbstractGeometryV2* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    c->addGeometry(( *geomIt )->segmentize() );
  }
  return c;
}
