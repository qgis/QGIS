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
#include "qgsmarkersymbol.h"

QgsAnnotationMarkerItem::QgsAnnotationMarkerItem( const QgsPoint &point )
  : QgsAnnotationItem()
  , mPoint( point )
  , mSymbol( std::make_unique< QgsMarkerSymbol >() )
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
  element.setAttribute( QStringLiteral( "zIndex" ), zIndex() );

  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "markerSymbol" ), mSymbol.get(), document, context ) );

  return true;
}

QgsAnnotationMarkerItem *QgsAnnotationMarkerItem::create()
{
  return new QgsAnnotationMarkerItem( QgsPoint() );
}

bool QgsAnnotationMarkerItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const double x = element.attribute( QStringLiteral( "x" ) ).toDouble();
  const double y = element.attribute( QStringLiteral( "y" ) ).toDouble();
  mPoint = QgsPoint( x, y );

  setZIndex( element.attribute( QStringLiteral( "zIndex" ) ).toInt() );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElem, context ) );

  return true;
}

QgsAnnotationMarkerItem *QgsAnnotationMarkerItem::clone()
{
  std::unique_ptr< QgsAnnotationMarkerItem > item = std::make_unique< QgsAnnotationMarkerItem >( mPoint );
  item->setSymbol( mSymbol->clone() );
  item->setZIndex( zIndex() );
  return item.release();
}

QgsRectangle QgsAnnotationMarkerItem::boundingBox() const
{
  return QgsRectangle( mPoint.x(), mPoint.y(), mPoint.x(), mPoint.y() );
}

const QgsMarkerSymbol *QgsAnnotationMarkerItem::symbol() const
{
  return mSymbol.get();
}

void QgsAnnotationMarkerItem::setSymbol( QgsMarkerSymbol *symbol )
{
  mSymbol.reset( symbol );
}
