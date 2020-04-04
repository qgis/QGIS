/***************************************************************************
                              qgslayoutitemmarker.cpp
                             ---------------------
    begin                : April 2020
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

#include "qgslayoutitemmarker.h"
#include "qgslayout.h"
#include "qgslayoututils.h"
#include "qgssymbollayerutils.h"
#include "qgslayoutmodel.h"
#include "qgsstyleentityvisitor.h"

#include <QPainter>

QgsLayoutItemMarker::QgsLayoutItemMarker( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  setBackgroundEnabled( false );
  setFrameEnabled( false );
  setReferencePoint( QgsLayoutItem::Middle );
  QgsStringMap properties;
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "4" ) );
  mShapeStyleSymbol.reset( QgsMarkerSymbol::createSimple( properties ) );
  refreshSymbol();

  connect( this, &QgsLayoutItemMarker::sizePositionChanged, this, [ = ]
  {
    updateBoundingRect();
    update();
  } );
}

QgsLayoutItemMarker::~QgsLayoutItemMarker() = default;

QgsLayoutItemMarker *QgsLayoutItemMarker::create( QgsLayout *layout )
{
  return new QgsLayoutItemMarker( layout );
}

int QgsLayoutItemMarker::type() const
{
  return QgsLayoutItemRegistry::LayoutMarker;
}

QIcon QgsLayoutItemMarker::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemMarker.svg" ) );
}

void QgsLayoutItemMarker::refreshSymbol()
{
  if ( layout() )
  {
    QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( layout(), nullptr, layout()->renderContext().dpi() );

    mShapeStyleSymbol->startRender( rc );
    QRectF bounds = mShapeStyleSymbol->bounds( QPointF( 0, 0 ), rc );
    mShapeStyleSymbol->stopRender( rc );
    mPoint = QPointF( -bounds.left() * 25.4 / layout()->renderContext().dpi(),
                      -bounds.top() * 25.4 / layout()->renderContext().dpi() );
    bounds.translate( mPoint );

    QgsLayoutSize newSizeMm = QgsLayoutSize( bounds.size()  * 25.4 / layout()->renderContext().dpi(), QgsUnitTypes::LayoutMillimeters );
    mFixedSize = mLayout->renderContext().measurementConverter().convert( newSizeMm, sizeWithUnits().units() );

    attemptResize( mFixedSize );
  }

  updateBoundingRect();

  update();
  emit frameChanged();
}

void QgsLayoutItemMarker::updateBoundingRect()
{
  QRectF rectangle = rect();

  // add a bit, to account for antialiasing on stroke and miter effects on stroke
  rectangle.adjust( -5, -5, 5, 5 );
  if ( rectangle != mCurrentRectangle )
  {
    prepareGeometryChange();
    mCurrentRectangle = rectangle;
  }
}

void QgsLayoutItemMarker::setSymbol( QgsMarkerSymbol *symbol )
{
  if ( !symbol )
    return;

  mShapeStyleSymbol.reset( symbol );
  refreshSymbol();
}

QgsMarkerSymbol *QgsLayoutItemMarker::symbol()
{
  return mShapeStyleSymbol.get();
}

QRectF QgsLayoutItemMarker::boundingRect() const
{
  return mCurrentRectangle;
}

QgsLayoutSize QgsLayoutItemMarker::fixedSize() const
{
  return mFixedSize;
}

bool QgsLayoutItemMarker::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mShapeStyleSymbol )
  {
    QgsStyleSymbolEntity entity( mShapeStyleSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, uuid(), displayName() ) ) )
      return false;
  }

  return true;
}

void QgsLayoutItemMarker::draw( QgsLayoutItemRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();
  painter->setPen( Qt::NoPen );
  painter->setBrush( Qt::NoBrush );

  double scale = context.renderContext().convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  QPointF shapePoint = mPoint * scale;

  symbol()->startRender( context.renderContext() );
  symbol()->renderPoint( shapePoint, nullptr, context.renderContext() );
  symbol()->stopRender( context.renderContext() );
}

bool QgsLayoutItemMarker::writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement shapeStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mShapeStyleSymbol.get(), document, context );
  element.appendChild( shapeStyleElem );

  return true;
}

bool QgsLayoutItemMarker::readPropertiesFromElement( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext &context )
{
  QDomElement shapeStyleSymbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !shapeStyleSymbolElem.isNull() )
  {
    mShapeStyleSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( shapeStyleSymbolElem, context ) );
    refreshSymbol();
  }

  return true;
}
