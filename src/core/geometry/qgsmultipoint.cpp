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
{
  mWkbType = QgsWkbTypes::MultiPoint;
}

QString QgsMultiPointV2::geometryType() const
{
  return QStringLiteral( "MultiPoint" );
}

QgsMultiPointV2 *QgsMultiPointV2::createEmptyWithSameType() const
{
  auto result = qgis::make_unique< QgsMultiPointV2 >();
  result->mWkbType = mWkbType;
  return result.release();
}

QgsMultiPointV2 *QgsMultiPointV2::clone() const
{
  return new QgsMultiPointV2( *this );
}

QgsMultiPointV2 *QgsMultiPointV2::toCurveType() const
{
  return clone();
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

void QgsMultiPointV2::clear()
{
  QgsGeometryCollection::clear();
  mWkbType = QgsWkbTypes::MultiPoint;
}

QDomElement QgsMultiPointV2::asGML2( QDomDocument &doc, int precision, const QString &ns ) const
{
  QDomElement elemMultiPoint = doc.createElementNS( ns, QStringLiteral( "MultiPoint" ) );

  if ( isEmpty() )
    return elemMultiPoint;

  for ( const QgsAbstractGeometry *geom : mGeometries )
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

  if ( isEmpty() )
    return elemMultiPoint;

  for ( const QgsAbstractGeometry *geom : mGeometries )
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
  for ( const QgsAbstractGeometry *geom : mGeometries )
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

int QgsMultiPointV2::nCoordinates() const
{
  return mGeometries.size();
}

bool QgsMultiPointV2::addGeometry( QgsAbstractGeometry *g )
{
  if ( !qgsgeometry_cast<QgsPoint *>( g ) )
  {
    delete g;
    return false;
  }
  if ( mGeometries.empty() )
  {
    setZMTypeFromSubGeometry( g, QgsWkbTypes::MultiPoint );
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

bool QgsMultiPointV2::insertGeometry( QgsAbstractGeometry *g, int index )
{
  if ( !g || QgsWkbTypes::flatType( g->wkbType() ) != QgsWkbTypes::Point )
  {
    delete g;
    return false;
  }

  return QgsGeometryCollection::insertGeometry( g, index );
}

QgsAbstractGeometry *QgsMultiPointV2::boundary() const
{
  return nullptr;
}

int QgsMultiPointV2::vertexNumberFromVertexId( QgsVertexId id ) const
{
  if ( id.part < 0 || id.part >= mGeometries.count() || id.vertex != 0 || id.ring != 0 )
    return -1;

  return id.part; // can shortcut the calculation, since each part will have 1 vertex
}

bool QgsMultiPointV2::wktOmitChildType() const
{
  return true;
}
