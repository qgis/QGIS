/***************************************************************************
    qgsannotationlinestringitem.cpp
    ----------------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationlinestringitem.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

QgsAnnotationLineStringItem::QgsAnnotationLineStringItem( const QgsLineString &linestring, const QgsCoordinateReferenceSystem &crs )
  : QgsAnnotationItem( crs )
  , mLineString( linestring )
  , mSymbol( qgis::make_unique< QgsLineSymbol >() )
{

}

QgsAnnotationLineStringItem::~QgsAnnotationLineStringItem() = default;

QString QgsAnnotationLineStringItem::type() const
{
  return QStringLiteral( "linestring" );
}

void QgsAnnotationLineStringItem::render( QgsRenderContext &context, QgsFeedback * )
{
  QPolygonF pts = mLineString.asQPolygonF();

  //transform the QPolygonF to screen coordinates
  if ( context.coordinateTransform().isValid() )
  {
    try
    {
      context.coordinateTransform().transformPolygon( pts );
    }
    catch ( QgsCsException & )
    {
      // we don't abort the rendering here, instead we remove any invalid points and just plot those which ARE valid
    }
  }

  // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
  pts.erase( std::remove_if( pts.begin(), pts.end(),
                             []( const QPointF point )
  {
    return !std::isfinite( point.x() ) || !std::isfinite( point.y() );
  } ), pts.end() );

  QPointF *ptr = pts.data();
  for ( int i = 0; i < pts.size(); ++i, ++ptr )
  {
    context.mapToPixel().transformInPlace( ptr->rx(), ptr->ry() );
  }

  mSymbol->startRender( context );
  mSymbol->renderPolyline( pts, nullptr, context );
  mSymbol->stopRender( context );
}

bool QgsAnnotationLineStringItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "wkt" ), mLineString.asWkt() );
  crs().writeXml( element, document );
  element.setAttribute( QStringLiteral( "zIndex" ), zIndex() );

  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "lineSymbol" ), mSymbol.get(), document, context ) );

  return true;
}

QgsAnnotationLineStringItem *QgsAnnotationLineStringItem::create()
{
  return new QgsAnnotationLineStringItem( QgsLineString(), QgsCoordinateReferenceSystem() );
}

bool QgsAnnotationLineStringItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString wkt = element.attribute( QStringLiteral( "wkt" ) );
  QgsLineString ls;
  mLineString.fromWkt( wkt );

  QgsCoordinateReferenceSystem crs;
  crs.readXml( element );
  setCrs( crs );
  setZIndex( element.attribute( QStringLiteral( "zIndex" ) ).toInt() );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( symbolElem, context ) );

  return true;
}

QgsAnnotationLineStringItem *QgsAnnotationLineStringItem::clone()
{
  std::unique_ptr< QgsAnnotationLineStringItem > item = qgis::make_unique< QgsAnnotationLineStringItem >( mLineString, crs() );
  item->setSymbol( mSymbol->clone() );
  item->setZIndex( zIndex() );
  return item.release();
}

const QgsLineSymbol *QgsAnnotationLineStringItem::symbol() const
{
  return mSymbol.get();
}

void QgsAnnotationLineStringItem::setSymbol( QgsLineSymbol *symbol )
{
  mSymbol.reset( symbol );
}
