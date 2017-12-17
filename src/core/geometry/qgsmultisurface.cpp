
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
{
  mWkbType = QgsWkbTypes::MultiSurface;
}

QString QgsMultiSurface::geometryType() const
{
  return QStringLiteral( "MultiSurface" );
}

void QgsMultiSurface::clear()
{
  QgsGeometryCollection::clear();
  mWkbType = QgsWkbTypes::MultiSurface;
}

QgsMultiSurface *QgsMultiSurface::createEmptyWithSameType() const
{
  auto result = qgis::make_unique< QgsMultiSurface >();
  result->mWkbType = mWkbType;
  return result.release();
}

QgsMultiSurface *QgsMultiSurface::clone() const
{
  return new QgsMultiSurface( *this );
}

QgsMultiSurface *QgsMultiSurface::toCurveType() const
{
  return clone();
}

bool QgsMultiSurface::fromWkt( const QString &wkt )
{
  return fromCollectionWkt( wkt,
                            QVector<QgsAbstractGeometry *>() << new QgsPolygon << new QgsCurvePolygon,
                            QStringLiteral( "Polygon" ) );
}

QDomElement QgsMultiSurface::asGml2( QDomDocument &doc, int precision, const QString &ns ) const
{
  // GML2 does not support curves
  QDomElement elemMultiPolygon = doc.createElementNS( ns, QStringLiteral( "MultiPolygon" ) );

  if ( isEmpty() )
    return elemMultiPolygon;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsSurface *>( geom ) )
    {
      std::unique_ptr< QgsPolygon > polygon( static_cast<const QgsSurface *>( geom )->surfaceToPolygon() );

      QDomElement elemPolygonMember = doc.createElementNS( ns, QStringLiteral( "polygonMember" ) );
      elemPolygonMember.appendChild( polygon->asGml2( doc, precision, ns ) );
      elemMultiPolygon.appendChild( elemPolygonMember );
    }
  }

  return elemMultiPolygon;
}

QDomElement QgsMultiSurface::asGml3( QDomDocument &doc, int precision, const QString &ns ) const
{
  QDomElement elemMultiSurface = doc.createElementNS( ns, QStringLiteral( "MultiSurface" ) );

  if ( isEmpty() )
    return elemMultiSurface;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsSurface *>( geom ) )
    {
      QDomElement elemSurfaceMember = doc.createElementNS( ns, QStringLiteral( "surfaceMember" ) );
      elemSurfaceMember.appendChild( geom->asGml3( doc, precision, ns ) );
      elemMultiSurface.appendChild( elemSurfaceMember );
    }
  }

  return elemMultiSurface;
}

QString QgsMultiSurface::asJson( int precision ) const
{
  // GeoJSON does not support curves
  QString json = QStringLiteral( "{\"type\": \"MultiPolygon\", \"coordinates\": [" );
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsSurface *>( geom ) )
    {
      json += '[';

      std::unique_ptr< QgsPolygon >polygon( static_cast<const QgsSurface *>( geom )->surfaceToPolygon() );

      std::unique_ptr< QgsLineString > exteriorLineString( polygon->exteriorRing()->curveToLine() );
      QgsPointSequence exteriorPts;
      exteriorLineString->points( exteriorPts );
      json += QgsGeometryUtils::pointsToJSON( exteriorPts, precision ) + ", ";

      std::unique_ptr< QgsLineString > interiorLineString;
      for ( int i = 0, n = polygon->numInteriorRings(); i < n; ++i )
      {
        interiorLineString.reset( polygon->interiorRing( i )->curveToLine() );
        QgsPointSequence interiorPts;
        interiorLineString->points( interiorPts );
        json += QgsGeometryUtils::pointsToJSON( interiorPts, precision ) + ", ";
      }
      if ( json.endsWith( QLatin1String( ", " ) ) )
      {
        json.chop( 2 ); // Remove last ", "
      }

      json += QLatin1String( "], " );
    }
  }
  if ( json.endsWith( QLatin1String( ", " ) ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += QLatin1String( "] }" );
  return json;
}

bool QgsMultiSurface::addGeometry( QgsAbstractGeometry *g )
{
  if ( !qgsgeometry_cast<QgsSurface *>( g ) )
  {
    delete g;
    return false;
  }

  if ( mGeometries.empty() )
  {
    setZMTypeFromSubGeometry( g, QgsWkbTypes::MultiSurface );
  }
  if ( is3D() && !g->is3D() )
    g->addZValue();
  else if ( !is3D() && g->is3D() )
    g->dropZValue();
  if ( isMeasure() && !g->isMeasure() )
    g->addMValue();
  else if ( !isMeasure() && g->isMeasure() )
    g->dropMValue();

  return QgsGeometryCollection::addGeometry( g );
}

bool QgsMultiSurface::insertGeometry( QgsAbstractGeometry *g, int index )
{
  if ( !g || !qgsgeometry_cast< QgsSurface * >( g ) )
  {
    delete g;
    return false;
  }

  return QgsGeometryCollection::insertGeometry( g, index );
}

QgsAbstractGeometry *QgsMultiSurface::boundary() const
{
  std::unique_ptr< QgsMultiCurve > multiCurve( new QgsMultiCurve() );
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    if ( QgsSurface *surface = qgsgeometry_cast<QgsSurface *>( mGeometries.at( i ) ) )
    {
      multiCurve->addGeometry( surface->boundary() );
    }
  }
  if ( multiCurve->numGeometries() == 0 )
  {
    return nullptr;
  }
  return multiCurve.release();
}
