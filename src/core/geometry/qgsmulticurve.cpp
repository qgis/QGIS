/***************************************************************************
                        qgsmulticurve.cpp
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

#include "qgsmulticurve.h"
#include "qgsapplication.h"
#include "qgscurve.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmultipoint.h"

#include <QJsonObject>
#include <memory>
#include <nlohmann/json.hpp>

QgsMultiCurve::QgsMultiCurve()
{
  mWkbType = QgsWkbTypes::MultiCurve;
}

QString QgsMultiCurve::geometryType() const
{
  return QStringLiteral( "MultiCurve" );
}

QgsMultiCurve *QgsMultiCurve::createEmptyWithSameType() const
{
  auto result = qgis::make_unique< QgsMultiCurve >();
  result->mWkbType = mWkbType;
  return result.release();
}

QgsMultiCurve *QgsMultiCurve::clone() const
{
  return new QgsMultiCurve( *this );
}

void QgsMultiCurve::clear()
{
  QgsGeometryCollection::clear();
  mWkbType = QgsWkbTypes::MultiCurve;
}

QgsMultiCurve *QgsMultiCurve::toCurveType() const
{
  return clone();
}

bool QgsMultiCurve::fromWkt( const QString &wkt )
{
  return fromCollectionWkt( wkt,
                            QVector<QgsAbstractGeometry *>() << new QgsLineString << new QgsCircularString << new QgsCompoundCurve,
                            QStringLiteral( "LineString" ) );
}

QDomElement QgsMultiCurve::asGml2( QDomDocument &doc, int precision, const QString &ns, const  AxisOrder axisOrder ) const
{
  // GML2 does not support curves
  QDomElement elemMultiLineString = doc.createElementNS( ns, QStringLiteral( "MultiLineString" ) );

  if ( isEmpty() )
    return elemMultiLineString;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsCurve *>( geom ) )
    {
      std::unique_ptr< QgsLineString > lineString( static_cast<const QgsCurve *>( geom )->curveToLine() );

      QDomElement elemLineStringMember = doc.createElementNS( ns, QStringLiteral( "lineStringMember" ) );
      elemLineStringMember.appendChild( lineString->asGml2( doc, precision, ns, axisOrder ) );
      elemMultiLineString.appendChild( elemLineStringMember );
    }
  }

  return elemMultiLineString;
}

QDomElement QgsMultiCurve::asGml3( QDomDocument &doc, int precision, const QString &ns, const AxisOrder axisOrder ) const
{
  QDomElement elemMultiCurve = doc.createElementNS( ns, QStringLiteral( "MultiCurve" ) );

  if ( isEmpty() )
    return elemMultiCurve;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsCurve *>( geom ) )
    {
      const QgsCurve *curve = static_cast<const QgsCurve *>( geom );

      QDomElement elemCurveMember = doc.createElementNS( ns, QStringLiteral( "curveMember" ) );
      elemCurveMember.appendChild( curve->asGml3( doc, precision, ns, axisOrder ) );
      elemMultiCurve.appendChild( elemCurveMember );
    }
  }

  return elemMultiCurve;
}

json QgsMultiCurve::asJsonObject( int precision ) const
{
  json coordinates( json::array( ) );
  for ( const QgsAbstractGeometry *geom : qgis::as_const( mGeometries ) )
  {
    if ( qgsgeometry_cast<const QgsCurve *>( geom ) )
    {
      std::unique_ptr< QgsLineString > lineString( static_cast<const QgsCurve *>( geom )->curveToLine() );
      QgsPointSequence pts;
      lineString->points( pts );
      coordinates.push_back( QgsGeometryUtils::pointsToJson( pts, precision ) );
    }
  }
  return
  {
    {  "type",  "MultiLineString"  },
    {  "coordinates", coordinates }
  };
}

bool QgsMultiCurve::addGeometry( QgsAbstractGeometry *g )
{
  if ( !qgsgeometry_cast<QgsCurve *>( g ) )
  {
    delete g;
    return false;
  }

  if ( mGeometries.empty() )
  {
    setZMTypeFromSubGeometry( g, QgsWkbTypes::MultiCurve );
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

bool QgsMultiCurve::insertGeometry( QgsAbstractGeometry *g, int index )
{
  if ( !g || !qgsgeometry_cast<QgsCurve *>( g ) )
  {
    delete g;
    return false;
  }

  return QgsGeometryCollection::insertGeometry( g, index );
}

QgsMultiCurve *QgsMultiCurve::reversed() const
{
  QgsMultiCurve *reversedMultiCurve = new QgsMultiCurve();
  reversedMultiCurve->reserve( mGeometries.size() );
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsCurve *>( geom ) )
    {
      reversedMultiCurve->addGeometry( static_cast<const QgsCurve *>( geom )->reversed() );
    }
  }
  return reversedMultiCurve;
}

QgsAbstractGeometry *QgsMultiCurve::boundary() const
{
  std::unique_ptr< QgsMultiPoint > multiPoint( new QgsMultiPoint() );
  multiPoint->reserve( mGeometries.size() * 2 );
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    if ( QgsCurve *curve = qgsgeometry_cast<QgsCurve *>( mGeometries.at( i ) ) )
    {
      if ( !curve->isClosed() )
      {
        multiPoint->addGeometry( new QgsPoint( curve->startPoint() ) );
        multiPoint->addGeometry( new QgsPoint( curve->endPoint() ) );
      }
    }
  }
  if ( multiPoint->numGeometries() == 0 )
  {
    return nullptr;
  }
  return multiPoint.release();
}
