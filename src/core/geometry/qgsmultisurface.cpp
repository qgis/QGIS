
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
#include "qgsgeometryutils.h"
#include "qgssurface.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgscurvepolygon.h"
#include "qgsmulticurve.h"

#include <QJsonObject>
#include <nlohmann/json.hpp>

QgsMultiSurface::QgsMultiSurface()
{
  mWkbType = Qgis::WkbType::MultiSurface;
}

QgsSurface *QgsMultiSurface::surfaceN( int index )
{
  return qgsgeometry_cast< QgsSurface * >( geometryN( index ) );
}

const QgsSurface *QgsMultiSurface::surfaceN( int index ) const
{
  return qgsgeometry_cast< const QgsSurface * >( geometryN( index ) );
}

QString QgsMultiSurface::geometryType() const
{
  return QStringLiteral( "MultiSurface" );
}

void QgsMultiSurface::clear()
{
  QgsGeometryCollection::clear();
  mWkbType = Qgis::WkbType::MultiSurface;
}

QgsMultiSurface *QgsMultiSurface::createEmptyWithSameType() const
{
  auto result = std::make_unique< QgsMultiSurface >();
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

QDomElement QgsMultiSurface::asGml2( QDomDocument &doc, int precision, const QString &ns, const AxisOrder axisOrder ) const
{
  // GML2 does not support curves
  QDomElement elemMultiPolygon = doc.createElementNS( ns, QStringLiteral( "MultiPolygon" ) );

  if ( isEmpty() )
    return elemMultiPolygon;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsCurvePolygon *>( geom ) )
    {
      std::unique_ptr< QgsPolygon > polygon( static_cast<const QgsCurvePolygon *>( geom )->surfaceToPolygon() );

      QDomElement elemPolygonMember = doc.createElementNS( ns, QStringLiteral( "polygonMember" ) );
      elemPolygonMember.appendChild( polygon->asGml2( doc, precision, ns, axisOrder ) );
      elemMultiPolygon.appendChild( elemPolygonMember );
    }
  }

  return elemMultiPolygon;
}

QDomElement QgsMultiSurface::asGml3( QDomDocument &doc, int precision, const QString &ns, const AxisOrder axisOrder ) const
{
  QDomElement elemMultiSurface = doc.createElementNS( ns, QStringLiteral( "MultiSurface" ) );

  if ( isEmpty() )
    return elemMultiSurface;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsSurface *>( geom ) )
    {
      QDomElement elemSurfaceMember = doc.createElementNS( ns, QStringLiteral( "surfaceMember" ) );
      elemSurfaceMember.appendChild( geom->asGml3( doc, precision, ns, axisOrder ) );
      elemMultiSurface.appendChild( elemSurfaceMember );
    }
  }

  return elemMultiSurface;
}


json QgsMultiSurface::asJsonObject( int precision ) const
{
  json polygons( json::array( ) );
  for ( const QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
  {
    if ( qgsgeometry_cast<const QgsCurvePolygon *>( geom ) )
    {
      json coordinates( json::array( ) );
      std::unique_ptr< QgsPolygon >polygon( static_cast<const QgsCurvePolygon *>( geom )->surfaceToPolygon() );
      std::unique_ptr< QgsLineString > exteriorLineString( polygon->exteriorRing()->curveToLine() );
      QgsPointSequence exteriorPts;
      exteriorLineString->points( exteriorPts );
      coordinates.push_back( QgsGeometryUtils::pointsToJson( exteriorPts, precision ) );

      std::unique_ptr< QgsLineString > interiorLineString;
      for ( int i = 0, n = polygon->numInteriorRings(); i < n; ++i )
      {
        interiorLineString.reset( polygon->interiorRing( i )->curveToLine() );
        QgsPointSequence interiorPts;
        interiorLineString->points( interiorPts );
        coordinates.push_back( QgsGeometryUtils::pointsToJson( interiorPts, precision ) );
      }
      polygons.push_back( coordinates );
    }
  }
  return
  {
    {  "type",  "MultiPolygon" },
    {  "coordinates", polygons }
  };
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
    setZMTypeFromSubGeometry( g, Qgis::WkbType::MultiSurface );
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

bool QgsMultiSurface::addGeometries( const QVector<QgsAbstractGeometry *> &geometries )
{
  for ( QgsAbstractGeometry *g : geometries )
  {
    if ( !qgsgeometry_cast<QgsSurface *>( g ) )
    {
      qDeleteAll( geometries );
      return false;
    }
  }

  if ( mGeometries.empty() && !geometries.empty() )
  {
    setZMTypeFromSubGeometry( geometries.at( 0 ), Qgis::WkbType::MultiSurface );
  }
  mGeometries.reserve( mGeometries.size() + geometries.size() );
  for ( QgsAbstractGeometry *g : geometries )
  {
    if ( is3D() && !g->is3D() )
      g->addZValue();
    else if ( !is3D() && g->is3D() )
      g->dropZValue();
    if ( isMeasure() && !g->isMeasure() )
      g->addMValue();
    else if ( !isMeasure() && g->isMeasure() )
      g->dropMValue();
    mGeometries.append( g );
  }

  clearCache();
  return true;
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
  multiCurve->reserve( mGeometries.size() );
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

QgsMultiSurface *QgsMultiSurface::simplifyByDistance( double tolerance ) const
{
  std::unique_ptr< QgsMultiSurface > res = std::make_unique< QgsMultiSurface >();
  res->reserve( mGeometries.size() );
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    res->addGeometry( mGeometries.at( i )->simplifyByDistance( tolerance ) );
  }
  return res.release();
}
