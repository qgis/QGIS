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

QgsMultiPoint::QgsMultiPoint()
{
  mWkbType = QgsWkbTypes::MultiPoint;
}

QString QgsMultiPoint::geometryType() const
{
  return QStringLiteral( "MultiPoint" );
}

QgsMultiPoint *QgsMultiPoint::createEmptyWithSameType() const
{
  auto result = qgis::make_unique< QgsMultiPoint >();
  result->mWkbType = mWkbType;
  return result.release();
}

QgsMultiPoint *QgsMultiPoint::clone() const
{
  return new QgsMultiPoint( *this );
}

QgsMultiPoint *QgsMultiPoint::toCurveType() const
{
  return clone();
}

bool QgsMultiPoint::fromWkt( const QString &wkt )
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

  return fromCollectionWkt( collectionWkt, QVector<QgsAbstractGeometry *>() << new QgsPoint, QStringLiteral( "Point" ) );
}

void QgsMultiPoint::clear()
{
  QgsGeometryCollection::clear();
  mWkbType = QgsWkbTypes::MultiPoint;
}

QDomElement QgsMultiPoint::asGml2( QDomDocument &doc, int precision, const QString &ns, const AxisOrder axisOrder ) const
{
  QDomElement elemMultiPoint = doc.createElementNS( ns, QStringLiteral( "MultiPoint" ) );

  if ( isEmpty() )
    return elemMultiPoint;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsPoint *>( geom ) )
    {
      QDomElement elemPointMember = doc.createElementNS( ns, QStringLiteral( "pointMember" ) );
      elemPointMember.appendChild( geom->asGml2( doc, precision, ns, axisOrder ) );
      elemMultiPoint.appendChild( elemPointMember );
    }
  }

  return elemMultiPoint;
}

QDomElement QgsMultiPoint::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement elemMultiPoint = doc.createElementNS( ns, QStringLiteral( "MultiPoint" ) );

  if ( isEmpty() )
    return elemMultiPoint;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsPoint *>( geom ) )
    {
      QDomElement elemPointMember = doc.createElementNS( ns, QStringLiteral( "pointMember" ) );
      elemPointMember.appendChild( geom->asGml3( doc, precision, ns, axisOrder ) );
      elemMultiPoint.appendChild( elemPointMember );
    }
  }

  return elemMultiPoint;
}

QString QgsMultiPoint::asJson( int precision ) const
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

int QgsMultiPoint::nCoordinates() const
{
  return mGeometries.size();
}

bool QgsMultiPoint::addGeometry( QgsAbstractGeometry *g )
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

bool QgsMultiPoint::insertGeometry( QgsAbstractGeometry *g, int index )
{
  if ( !g || QgsWkbTypes::flatType( g->wkbType() ) != QgsWkbTypes::Point )
  {
    delete g;
    return false;
  }

  return QgsGeometryCollection::insertGeometry( g, index );
}

QgsAbstractGeometry *QgsMultiPoint::boundary() const
{
  return nullptr;
}

int QgsMultiPoint::vertexNumberFromVertexId( QgsVertexId id ) const
{
  if ( id.part < 0 || id.part >= mGeometries.count() || id.vertex != 0 || id.ring != 0 )
    return -1;

  return id.part; // can shortcut the calculation, since each part will have 1 vertex
}

double QgsMultiPoint::segmentLength( QgsVertexId ) const
{
  return 0.0;
}

void QgsMultiPoint::filterVertices( const std::function<bool ( const QgsPoint & )> &filter )
{
  mGeometries.erase( std::remove_if( mGeometries.begin(), mGeometries.end(), // clazy:exclude=detaching-member
                                     [&filter]( const QgsAbstractGeometry * part )
  {
    if ( const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( part ) )
    {
      if ( !filter( *point ) )
      {
        delete point;
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      delete part;
      return true;
    }
  } ), mGeometries.end() ); // clazy:exclude=detaching-member
}

bool QgsMultiPoint::wktOmitChildType() const
{
  return true;
}
