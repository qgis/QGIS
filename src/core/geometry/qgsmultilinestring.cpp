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

QgsMultiLineString::QgsMultiLineString()
    : QgsMultiCurve()
{
  mWkbType = QgsWkbTypes::MultiLineString;
}

QgsMultiLineString* QgsMultiLineString::clone() const
{
  return new QgsMultiLineString( *this );
}

bool QgsMultiLineString::fromWkt( const QString& wkt )
{
  return fromCollectionWkt( wkt, QList<QgsAbstractGeometry*>() << new QgsLineString, "LineString" );
}

QDomElement QgsMultiLineString::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiLineString = doc.createElementNS( ns, "MultiLineString" );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsLineString*>( geom ) )
    {
      const QgsLineString* lineString = static_cast<const QgsLineString*>( geom );

      QDomElement elemLineStringMember = doc.createElementNS( ns, "lineStringMember" );
      elemLineStringMember.appendChild( lineString->asGML2( doc, precision, ns ) );
      elemMultiLineString.appendChild( elemLineStringMember );

      delete lineString;
    }
  }

  return elemMultiLineString;
}

QDomElement QgsMultiLineString::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiCurve = doc.createElementNS( ns, "MultiLineString" );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsLineString*>( geom ) )
    {
      const QgsLineString* lineString = static_cast<const QgsLineString*>( geom );

      QDomElement elemCurveMember = doc.createElementNS( ns, "curveMember" );
      elemCurveMember.appendChild( lineString->asGML3( doc, precision, ns ) );
      elemMultiCurve.appendChild( elemCurveMember );
    }
  }

  return elemMultiCurve;
}

QString QgsMultiLineString::asJSON( int precision ) const
{
  QString json = "{\"type\": \"MultiLineString\", \"coordinates\": [";
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsCurve*>( geom ) )
    {
      const QgsLineString* lineString = static_cast<const QgsLineString*>( geom );
      QgsPointSequence pts;
      lineString->points( pts );
      json += QgsGeometryUtils::pointsToJSON( pts, precision ) + ", ";
    }
  }
  if ( json.endsWith( ", " ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += "] }";
  return json;
}

bool QgsMultiLineString::addGeometry( QgsAbstractGeometry* g )
{
  if ( !dynamic_cast<QgsLineString*>( g ) )
  {
    delete g;
    return false;
  }

  setZMTypeFromSubGeometry( g, QgsWkbTypes::MultiLineString );
  return QgsGeometryCollection::addGeometry( g );
}

QgsAbstractGeometry* QgsMultiLineString::toCurveType() const
{
  QgsMultiCurve* multiCurve = new QgsMultiCurve();
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    multiCurve->addGeometry( mGeometries.at( i )->clone() );
  }
  return multiCurve;
}

