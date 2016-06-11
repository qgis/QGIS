/***************************************************************************
                        qgsmultipointv2.cpp
  -------------------------------------------------------------------
Date                 : 29 Oct 2014
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

#include "qgsmultipointv2.h"
#include "qgsapplication.h"
#include "qgsgeometryutils.h"
#include "qgspointv2.h"
#include "qgswkbptr.h"

QgsMultiPointV2::QgsMultiPointV2()
    : QgsGeometryCollectionV2()
{
  mWkbType = QgsWKBTypes::MultiPoint;
}

QgsMultiPointV2 *QgsMultiPointV2::clone() const
{
  return new QgsMultiPointV2( *this );
}

bool QgsMultiPointV2::fromWkt( const QString& wkt )
{
  QString collectionWkt( wkt );
  //test for non-standard MultiPoint(x1 y1, x2 y2) format
  QRegExp regex( "^\\s*MultiPoint\\s*[ZM]*\\s*\\(\\s*\\d" );
  regex.setCaseSensitivity( Qt::CaseInsensitive );
  if ( regex.indexIn( collectionWkt ) >= 0 )
  {
    //alternate style without extra brackets, upgrade to standard
    collectionWkt.replace( '(', "((" ).replace( ')', "))" ).replace( ',', "),(" );
  }

  return fromCollectionWkt( collectionWkt, QList<QgsAbstractGeometryV2*>() << new QgsPointV2, "Point" );
}

QDomElement QgsMultiPointV2::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiPoint = doc.createElementNS( ns, "MultiPoint" );
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsPointV2*>( geom ) )
    {
      QDomElement elemPointMember = doc.createElementNS( ns, "pointMember" );
      elemPointMember.appendChild( geom->asGML2( doc, precision, ns ) );
      elemMultiPoint.appendChild( elemPointMember );
    }
  }

  return elemMultiPoint;
}

QDomElement QgsMultiPointV2::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiPoint = doc.createElementNS( ns, "MultiPoint" );
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsPointV2*>( geom ) )
    {
      QDomElement elemPointMember = doc.createElementNS( ns, "pointMember" );
      elemPointMember.appendChild( geom->asGML3( doc, precision, ns ) );
      elemMultiPoint.appendChild( elemPointMember );
    }
  }

  return elemMultiPoint;
}

QString QgsMultiPointV2::asJSON( int precision ) const
{
  QString json = "{\"type\": \"MultiPoint\", \"coordinates\": ";

  QgsPointSequenceV2 pts;
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( dynamic_cast<const QgsPointV2*>( geom ) )
    {
      const QgsPointV2* point = static_cast<const QgsPointV2*>( geom );
      pts << *point;
    }
  }
  json += QgsGeometryUtils::pointsToJSON( pts, precision );
  json += " }";
  return json;
}

bool QgsMultiPointV2::addGeometry( QgsAbstractGeometryV2* g )
{
  if ( !dynamic_cast<QgsPointV2*>( g ) )
  {
    delete g;
    return false;
  }
  setZMTypeFromSubGeometry( g, QgsWKBTypes::MultiPoint );
  return QgsGeometryCollectionV2::addGeometry( g );
}
