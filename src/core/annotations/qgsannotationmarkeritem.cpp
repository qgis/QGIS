/***************************************************************************
    qgsannotationmarkeritem.cpp
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

#include "qgsannotationmarkeritem.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

QgsAnnotationMarkerItem::QgsAnnotationMarkerItem( QgsPointXY point, const QgsCoordinateReferenceSystem &crs )
  : QgsAnnotationItem( crs )
  , mPoint( point )
  , mSymbol( qgis::make_unique< QgsMarkerSymbol >() )
{

}

QgsAnnotationMarkerItem::~QgsAnnotationMarkerItem() = default;

QString QgsAnnotationMarkerItem::type() const
{
  return QStringLiteral( "marker" );
}

void QgsAnnotationMarkerItem::render( QgsRenderContext &context, QgsFeedback * )
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

bool QgsAnnotationMarkerItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "x" ), qgsDoubleToString( mPoint.x() ) );
  element.setAttribute( QStringLiteral( "y" ), qgsDoubleToString( mPoint.y() ) );
  crs().writeXml( element, document );

  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "markerSymbol" ), mSymbol.get(), document, context ) );

  return true;
}

QgsAnnotationMarkerItem *QgsAnnotationMarkerItem::create()
{
  return new QgsAnnotationMarkerItem( QgsPointXY(), QgsCoordinateReferenceSystem() );
}

bool QgsAnnotationMarkerItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const double x = element.attribute( QStringLiteral( "x" ) ).toDouble();
  const double y = element.attribute( QStringLiteral( "y" ) ).toDouble();
  mPoint = QgsPointXY( x, y );

  QgsCoordinateReferenceSystem crs;
  crs.readXml( element );
  setCrs( crs );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElem, context ) );

  return true;
}

QgsAnnotationMarkerItem *QgsAnnotationMarkerItem::clone()
{
  std::unique_ptr< QgsAnnotationMarkerItem > item = qgis::make_unique< QgsAnnotationMarkerItem >( mPoint, crs() );
  item->setSymbol( mSymbol->clone() );
  return item.release();
}

const QgsMarkerSymbol *QgsAnnotationMarkerItem::symbol() const
{
  return mSymbol.get();
}

void QgsAnnotationMarkerItem::setSymbol( QgsMarkerSymbol *symbol )
{
  mSymbol.reset( symbol );
}
