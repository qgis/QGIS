/***************************************************************************
                        qgsmulticurvev2.cpp
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

#include "qgsmulticurvev2.h"
#include "qgsapplication.h"
#include "qgscurvev2.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgsgeometryutils.h"
#include "qgslinestringv2.h"

QgsMultiCurveV2::QgsMultiCurveV2()
    : QgsGeometryCollectionV2()
{
  mWkbType = QgsWKBTypes::MultiCurve;
}

QgsMultiCurveV2 *QgsMultiCurveV2::clone() const
{
  return new QgsMultiCurveV2( *this );
}

bool QgsMultiCurveV2::fromWkt( const QString& wkt )
{
  return fromCollectionWkt( wkt,
                            QList<QgsAbstractGeometryV2*>() << new QgsLineStringV2 << new QgsCircularStringV2 << new QgsCompoundCurveV2,
                            "LineString" );
}

QDomElement QgsMultiCurveV2::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  // GML2 does not support curves
  QDomElement elemMultiLineString = doc.createElementNS( ns, "MultiLineString" );
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsCurveV2*>( geom ) )
    {
      QgsLineStringV2* lineString = static_cast<const QgsCurveV2*>( geom )->curveToLine();

      QDomElement elemLineStringMember = doc.createElementNS( ns, "lineStringMember" );
      elemLineStringMember.appendChild( lineString->asGML2( doc, precision, ns ) );
      elemMultiLineString.appendChild( elemLineStringMember );

      delete lineString;
    }
  }

  return elemMultiLineString;
}

QDomElement QgsMultiCurveV2::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiCurve = doc.createElementNS( ns, "MultiCurve" );
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsCurveV2*>( geom ) )
    {
      const QgsCurveV2* curve = static_cast<const QgsCurveV2*>( geom );

      QDomElement elemCurveMember = doc.createElementNS( ns, "curveMember" );
      elemCurveMember.appendChild( curve->asGML3( doc, precision, ns ) );
      elemMultiCurve.appendChild( elemCurveMember );
    }
  }

  return elemMultiCurve;
}

QString QgsMultiCurveV2::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QString json = "{\"type\": \"MultiLineString\", \"coordinates\": [";
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsCurveV2*>( geom ) )
    {
      QgsLineStringV2* lineString = static_cast<const QgsCurveV2*>( geom )->curveToLine();
      QgsPointSequenceV2 pts;
      lineString->points( pts );
      json += QgsGeometryUtils::pointsToJSON( pts, precision ) + ", ";
      delete lineString;
    }
  }
  if ( json.endsWith( ", " ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += "] }";
  return json;
}

bool QgsMultiCurveV2::addGeometry( QgsAbstractGeometryV2* g )
{
  if ( !dynamic_cast<QgsCurveV2*>( g ) )
  {
    delete g;
    return false;
  }

  setZMTypeFromSubGeometry( g, QgsWKBTypes::MultiCurve );
  return QgsGeometryCollectionV2::addGeometry( g );
}

QgsMultiCurveV2* QgsMultiCurveV2::reversed() const
{
  QgsMultiCurveV2* reversedMultiCurve = new QgsMultiCurveV2();
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsCurveV2*>( geom ) )
    {
      reversedMultiCurve->addGeometry( static_cast<const QgsCurveV2*>( geom )->reversed() );
    }
  }
  return reversedMultiCurve;
}
