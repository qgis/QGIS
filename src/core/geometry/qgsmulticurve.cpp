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

QgsMultiCurve::QgsMultiCurve()
    : QgsGeometryCollection()
{
  mWkbType = QgsWkbTypes::MultiCurve;
}

QgsMultiCurve *QgsMultiCurve::clone() const
{
  return new QgsMultiCurve( *this );
}

bool QgsMultiCurve::fromWkt( const QString& wkt )
{
  return fromCollectionWkt( wkt,
                            QList<QgsAbstractGeometry*>() << new QgsLineString << new QgsCircularString << new QgsCompoundCurve,
                            "LineString" );
}

QDomElement QgsMultiCurve::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  // GML2 does not support curves
  QDomElement elemMultiLineString = doc.createElementNS( ns, "MultiLineString" );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsCurve*>( geom ) )
    {
      QgsLineString* lineString = static_cast<const QgsCurve*>( geom )->curveToLine();

      QDomElement elemLineStringMember = doc.createElementNS( ns, "lineStringMember" );
      elemLineStringMember.appendChild( lineString->asGML2( doc, precision, ns ) );
      elemMultiLineString.appendChild( elemLineStringMember );

      delete lineString;
    }
  }

  return elemMultiLineString;
}

QDomElement QgsMultiCurve::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiCurve = doc.createElementNS( ns, "MultiCurve" );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsCurve*>( geom ) )
    {
      const QgsCurve* curve = static_cast<const QgsCurve*>( geom );

      QDomElement elemCurveMember = doc.createElementNS( ns, "curveMember" );
      elemCurveMember.appendChild( curve->asGML3( doc, precision, ns ) );
      elemMultiCurve.appendChild( elemCurveMember );
    }
  }

  return elemMultiCurve;
}

QString QgsMultiCurve::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QString json = "{\"type\": \"MultiLineString\", \"coordinates\": [";
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsCurve*>( geom ) )
    {
      QgsLineString* lineString = static_cast<const QgsCurve*>( geom )->curveToLine();
      QgsPointSequence pts;
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

bool QgsMultiCurve::addGeometry( QgsAbstractGeometry* g )
{
  if ( !dynamic_cast<QgsCurve*>( g ) )
  {
    delete g;
    return false;
  }

  setZMTypeFromSubGeometry( g, QgsWkbTypes::MultiCurve );
  return QgsGeometryCollection::addGeometry( g );
}

QgsMultiCurve* QgsMultiCurve::reversed() const
{
  QgsMultiCurve* reversedMultiCurve = new QgsMultiCurve();
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsCurve*>( geom ) )
    {
      reversedMultiCurve->addGeometry( static_cast<const QgsCurve*>( geom )->reversed() );
    }
  }
  return reversedMultiCurve;
}

QgsAbstractGeometry* QgsMultiCurve::boundary() const
{
  QgsMultiPointV2* multiPoint = new QgsMultiPointV2();
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    if ( QgsCurve* curve = dynamic_cast<QgsCurve*>( mGeometries.at( i ) ) )
    {
      if ( !curve->isClosed() )
      {
        multiPoint->addGeometry( new QgsPointV2( curve->startPoint() ) );
        multiPoint->addGeometry( new QgsPointV2( curve->endPoint() ) );
      }
    }
  }
  if ( multiPoint->numGeometries() == 0 )
  {
    delete multiPoint;
    return nullptr;
  }
  return multiPoint;
}
