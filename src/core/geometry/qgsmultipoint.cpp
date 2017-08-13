/***************************************************************************
                        qgsmultipoint.cpp
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

#include "qgsmultipoint.h"
#include "qgsapplication.h"
#include "qgsgeometryutils.h"
#include "qgspoint.h"
#include "qgswkbptr.h"

QgsMultiPointV2::QgsMultiPointV2()
  : QgsGeometryCollection()
{
  mWkbType = QgsWkbTypes::MultiPoint;
}

QgsMultiPointV2 *QgsMultiPointV2::clone() const
{
  return new QgsMultiPointV2( *this );
}

bool QgsMultiPointV2::fromWkt( const QString &wkt )
{
  QString collectionWkt( wkt );
  //test for non-standard MultiPoint(x1 y1, x2 y2) format
  QRegExp regex( "^\\s*MultiPoint\\s*[ZM]*\\s*\\(\\s*[-\\d]" );
  regex.setCaseSensitivity( Qt::CaseInsensitive );
  if ( regex.indexIn( collectionWkt ) >= 0 )
  {
    //alternate style without extra brackets, upgrade to standard
    collectionWkt.replace( '(', QLatin1String( "((" ) ).replace( ')', QLatin1String( "))" ) ).replace( ',', QLatin1String( "),(" ) );
  }

  return fromCollectionWkt( collectionWkt, QList<QgsAbstractGeometry *>() << new QgsPoint, QStringLiteral( "Point" ) );
}

QDomElement QgsMultiPointV2::asGML2( QDomDocument &doc, int precision, const QString &ns ) const
{
  QDomElement elemMultiPoint = doc.createElementNS( ns, QStringLiteral( "MultiPoint" ) );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( qgsgeometry_cast<const QgsPoint *>( geom ) )
    {
      QDomElement elemPointMember = doc.createElementNS( ns, QStringLiteral( "pointMember" ) );
      elemPointMember.appendChild( geom->asGML2( doc, precision, ns ) );
      elemMultiPoint.appendChild( elemPointMember );
    }
  }

  return elemMultiPoint;
}

QDomElement QgsMultiPointV2::asGML3( QDomDocument &doc, int precision, const QString &ns ) const
{
  QDomElement elemMultiPoint = doc.createElementNS( ns, QStringLiteral( "MultiPoint" ) );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( qgsgeometry_cast<const QgsPoint *>( geom ) )
    {
      QDomElement elemPointMember = doc.createElementNS( ns, QStringLiteral( "pointMember" ) );
      elemPointMember.appendChild( geom->asGML3( doc, precision, ns ) );
      elemMultiPoint.appendChild( elemPointMember );
    }
  }

  return elemMultiPoint;
}

QString QgsMultiPointV2::asJSON( int precision ) const
{
  QString json = QStringLiteral( "{\"type\": \"MultiPoint\", \"coordinates\": " );

  QgsPointSequence pts;
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( qgsgeometry_cast<const QgsPoint *>( geom ) )
    {
      const QgsPoint *point = static_cast<const QgsPoint *>( geom );
      pts << *point;
    }
  }
  json += QgsGeometryUtils::pointsToJSON( pts, precision );
  json += QLatin1String( " }" );
  return json;
}

bool QgsMultiPointV2::addGeometry( QgsAbstractGeometry *g )
{
  if ( !qgsgeometry_cast<QgsPoint *>( g ) )
  {
    delete g;
    return false;
  }
  setZMTypeFromSubGeometry( g, QgsWkbTypes::MultiPoint );
  return QgsGeometryCollection::addGeometry( g );
}

QgsAbstractGeometry *QgsMultiPointV2::boundary() const
{
  return nullptr;
}
