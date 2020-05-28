/***************************************************************************
                        qgsmultilinestring.cpp
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

#include "qgsmultilinestring.h"
#include "qgsapplication.h"
#include "qgscurve.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmulticurve.h"

#include <nlohmann/json.hpp>
#include <QJsonObject>

QgsMultiLineString::QgsMultiLineString()
{
  mWkbType = QgsWkbTypes::MultiLineString;
}

QString QgsMultiLineString::geometryType() const
{
  return QStringLiteral( "MultiLineString" );
}

QgsMultiLineString *QgsMultiLineString::createEmptyWithSameType() const
{
  auto result = qgis::make_unique< QgsMultiLineString >();
  result->mWkbType = mWkbType;
  return result.release();
}

QgsMultiLineString *QgsMultiLineString::clone() const
{
  return new QgsMultiLineString( *this );
}

void QgsMultiLineString::clear()
{
  QgsMultiCurve::clear();
  mWkbType = QgsWkbTypes::MultiLineString;
}

bool QgsMultiLineString::fromWkt( const QString &wkt )
{
  return fromCollectionWkt( wkt, QVector<QgsAbstractGeometry *>() << new QgsLineString, QStringLiteral( "LineString" ) );
}

QDomElement QgsMultiLineString::asGml2( QDomDocument &doc, int precision, const QString &ns, const AxisOrder axisOrder ) const
{
  QDomElement elemMultiLineString = doc.createElementNS( ns, QStringLiteral( "MultiLineString" ) );

  if ( isEmpty() )
    return elemMultiLineString;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( const QgsLineString *lineString = qgsgeometry_cast<const QgsLineString *>( geom ) )
    {
      QDomElement elemLineStringMember = doc.createElementNS( ns, QStringLiteral( "lineStringMember" ) );
      elemLineStringMember.appendChild( lineString->asGml2( doc, precision, ns, axisOrder ) );
      elemMultiLineString.appendChild( elemLineStringMember );
    }
  }

  return elemMultiLineString;
}

QDomElement QgsMultiLineString::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement elemMultiCurve = doc.createElementNS( ns, QStringLiteral( "MultiCurve" ) );

  if ( isEmpty() )
    return elemMultiCurve;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( const QgsLineString *lineString = qgsgeometry_cast<const QgsLineString *>( geom ) )
    {
      QDomElement elemCurveMember = doc.createElementNS( ns, QStringLiteral( "curveMember" ) );
      elemCurveMember.appendChild( lineString->asGml3( doc, precision, ns, axisOrder ) );
      elemMultiCurve.appendChild( elemCurveMember );
    }
  }

  return elemMultiCurve;
}

json QgsMultiLineString::asJsonObject( int precision ) const
{
  json coordinates( json::array( ) );
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsCurve *>( geom ) )
    {
      const QgsLineString *lineString = static_cast<const QgsLineString *>( geom );
      QgsPointSequence pts;
      lineString->points( pts );
      coordinates.push_back( QgsGeometryUtils::pointsToJson( pts, precision ) );
    }
  }
  return
  {
    { "type",  "MultiLineString" },
    { "coordinates", coordinates }
  };
}

bool QgsMultiLineString::addGeometry( QgsAbstractGeometry *g )
{
  if ( !dynamic_cast<QgsLineString *>( g ) )
  {
    delete g;
    return false;
  }

  if ( mGeometries.empty() )
  {
    setZMTypeFromSubGeometry( g, QgsWkbTypes::MultiLineString );
  }
  if ( is3D() && !g->is3D() )
    g->addZValue();
  else if ( !is3D() && g->is3D() )
    g->dropZValue();
  if ( isMeasure() && !g->isMeasure() )
    g->addMValue();
  else if ( !isMeasure() && g->isMeasure() )
    g->dropMValue();
  return QgsGeometryCollection::addGeometry( g ); // clazy:exclude=skipped-base-method
}

bool QgsMultiLineString::insertGeometry( QgsAbstractGeometry *g, int index )
{
  if ( !g || QgsWkbTypes::flatType( g->wkbType() ) != QgsWkbTypes::LineString )
  {
    delete g;
    return false;
  }

  return QgsMultiCurve::insertGeometry( g, index ); // clazy:exclude=skipped-base-method
}

QgsMultiCurve *QgsMultiLineString::toCurveType() const
{
  QgsMultiCurve *multiCurve = new QgsMultiCurve();
  multiCurve->reserve( mGeometries.size() );
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    multiCurve->addGeometry( mGeometries.at( i )->toCurveType() );
  }
  return multiCurve;
}

bool QgsMultiLineString::wktOmitChildType() const
{
  return true;
}

