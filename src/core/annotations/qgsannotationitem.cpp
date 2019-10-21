/***************************************************************************
    qgsannotationitem.cpp
    ------------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationitem.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"


QgsAnnotationItem::QgsAnnotationItem( const QgsCoordinateReferenceSystem &crs )
  : mCrs( crs )
{

}

QgsMarkerItem::QgsMarkerItem( QgsPointXY point, const QgsCoordinateReferenceSystem &crs )
  : QgsAnnotationItem( crs )
  , mPoint( point )
  , mSymbol( qgis::make_unique< QgsMarkerSymbol >() )
{

}

QgsMarkerItem::~QgsMarkerItem() = default;

QString QgsMarkerItem::type() const
{
  return QStringLiteral( "marker" );
}

void QgsMarkerItem::render( QgsRenderContext &context, QgsFeedback * )
{
  QPointF pt;
  if ( context.coordinateTransform().isValid() )
  {
    double x = mPoint.x();
    double y = mPoint.y();
    double z = 0.0;
    context.coordinateTransform().transformInPlace( x, y, z );
    pt = QPointF( x, y );
  }
  else
    pt = mPoint.toQPointF();

  context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );

  mSymbol->startRender( context );
  mSymbol->renderPoint( pt, nullptr, context );
  mSymbol->stopRender( context );
}

bool QgsMarkerItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "x" ), qgsDoubleToString( mPoint.x() ) );
  element.setAttribute( QStringLiteral( "y" ), qgsDoubleToString( mPoint.y() ) );
  crs().writeXml( element, document );

  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "markerSymbol" ), mSymbol.get(), document, context ) );

  return true;
}

QgsMarkerItem *QgsMarkerItem::create()
{
  return new QgsMarkerItem( QgsPointXY(), QgsCoordinateReferenceSystem() );
}

QgsMarkerItem *QgsMarkerItem::createFromElement( const QDomElement &element, const QgsReadWriteContext &context )
{
  const double x = element.attribute( QStringLiteral( "x" ) ).toDouble();
  const double y = element.attribute( QStringLiteral( "y" ) ).toDouble();

  QgsCoordinateReferenceSystem crs;
  crs.readXml( element );

  std::unique_ptr< QgsMarkerItem > item = qgis::make_unique< QgsMarkerItem >( QgsPointXY( x, y ), crs );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    item->setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElem, context ) );

  return item.release();
}

QgsMarkerItem *QgsMarkerItem::clone()
{
  std::unique_ptr< QgsMarkerItem > item = qgis::make_unique< QgsMarkerItem >( mPoint, crs() );
  item->setSymbol( mSymbol->clone() );
  return item.release();
}

const QgsMarkerSymbol *QgsMarkerItem::symbol() const
{
  return mSymbol.get();
}

void QgsMarkerItem::setSymbol( QgsMarkerSymbol *symbol )
{
  mSymbol.reset( symbol );
}
