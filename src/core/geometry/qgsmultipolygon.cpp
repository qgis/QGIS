/***************************************************************************
                        qgsmultipolygon.cpp
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

#include "qgsmultipolygon.h"
#include "qgsapplication.h"
#include "qgsgeometryutils.h"
#include "qgssurface.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgscurvepolygon.h"
#include "qgsmultilinestring.h"

QgsMultiPolygonV2::QgsMultiPolygonV2()
{
  mWkbType = QgsWkbTypes::MultiPolygon;
}

QString QgsMultiPolygonV2::geometryType() const
{
  return QStringLiteral( "MultiPolygon" );
}

void QgsMultiPolygonV2::clear()
{
  QgsMultiSurface::clear();
  mWkbType = QgsWkbTypes::MultiPolygon;
}

QgsMultiPolygonV2 *QgsMultiPolygonV2::createEmptyWithSameType() const
{
  auto result = qgis::make_unique< QgsMultiPolygonV2 >();
  result->mWkbType = mWkbType;
  return result.release();
}

QgsMultiPolygonV2 *QgsMultiPolygonV2::clone() const
{
  return new QgsMultiPolygonV2( *this );
}

bool QgsMultiPolygonV2::fromWkt( const QString &wkt )
{
  return fromCollectionWkt( wkt, QList<QgsAbstractGeometry *>() << new QgsPolygonV2, QStringLiteral( "Polygon" ) );
}

QDomElement QgsMultiPolygonV2::asGML2( QDomDocument &doc, int precision, const QString &ns ) const
{
  // GML2 does not support curves
  QDomElement elemMultiPolygon = doc.createElementNS( ns, QStringLiteral( "MultiPolygon" ) );

  if ( isEmpty() )
    return elemMultiPolygon;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsPolygon *>( geom ) )
    {
      QDomElement elemPolygonMember = doc.createElementNS( ns, QStringLiteral( "polygonMember" ) );
      elemPolygonMember.appendChild( geom->asGML2( doc, precision, ns ) );
      elemMultiPolygon.appendChild( elemPolygonMember );
    }
  }

  return elemMultiPolygon;
}

QDomElement QgsMultiPolygonV2::asGML3( QDomDocument &doc, int precision, const QString &ns ) const
{
  QDomElement elemMultiSurface = doc.createElementNS( ns, QStringLiteral( "MultiPolygon" ) );

  if ( isEmpty() )
    return elemMultiSurface;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsPolygon *>( geom ) )
    {
      QDomElement elemSurfaceMember = doc.createElementNS( ns, QStringLiteral( "polygonMember" ) );
      elemSurfaceMember.appendChild( geom->asGML3( doc, precision, ns ) );
      elemMultiSurface.appendChild( elemSurfaceMember );
    }
  }

  return elemMultiSurface;
}

QString QgsMultiPolygonV2::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QString json = QStringLiteral( "{\"type\": \"MultiPolygon\", \"coordinates\": [" );
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsPolygon *>( geom ) )
    {
      json += '[';

      const QgsPolygon *polygon = static_cast<const QgsPolygon *>( geom );

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

bool QgsMultiPolygonV2::addGeometry( QgsAbstractGeometry *g )
{
  if ( !qgsgeometry_cast<QgsPolygon *>( g ) )
  {
    delete g;
    return false;
  }

  if ( mGeometries.empty() )
  {
    setZMTypeFromSubGeometry( g, QgsWkbTypes::MultiPolygon );
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

bool QgsMultiPolygonV2::insertGeometry( QgsAbstractGeometry *g, int index )
{
  if ( !g || !qgsgeometry_cast< QgsPolygon * >( g ) )
  {
    delete g;
    return false;
  }

  return QgsMultiSurface::insertGeometry( g, index );
}

QgsMultiSurface *QgsMultiPolygonV2::toCurveType() const
{
  QgsMultiSurface *multiSurface = new QgsMultiSurface();
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    multiSurface->addGeometry( mGeometries.at( i )->clone() );
  }
  return multiSurface;
}

QgsAbstractGeometry *QgsMultiPolygonV2::boundary() const
{
  std::unique_ptr< QgsMultiLineString > multiLine( new QgsMultiLineString() );
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    if ( QgsPolygon *polygon = qgsgeometry_cast<QgsPolygon *>( mGeometries.at( i ) ) )
    {
      QgsAbstractGeometry *polygonBoundary = polygon->boundary();

      if ( QgsLineString *lineStringBoundary = qgsgeometry_cast< QgsLineString * >( polygonBoundary ) )
      {
        multiLine->addGeometry( lineStringBoundary );
      }
      else if ( QgsMultiLineString *multiLineStringBoundary = qgsgeometry_cast< QgsMultiLineString * >( polygonBoundary ) )
      {
        for ( int j = 0; j < multiLineStringBoundary->numGeometries(); ++j )
        {
          multiLine->addGeometry( multiLineStringBoundary->geometryN( j )->clone() );
        }
        delete multiLineStringBoundary;
      }
      else
      {
        delete polygonBoundary;
      }
    }
  }
  if ( multiLine->numGeometries() == 0 )
  {
    return nullptr;
  }
  return multiLine.release();
}

bool QgsMultiPolygonV2::wktOmitChildType() const
{
  return true;
}
