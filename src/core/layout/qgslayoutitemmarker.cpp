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
#include "qgslayoutitemmap.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoututils.h"
#include "qgsmarkersymbol.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbollayerutils.h"

#include <QPainter>

#include "moc_qgslayoutitemmarker.cpp"

QgsLayoutItemMarker::QgsLayoutItemMarker( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mNorthArrowHandler( new QgsLayoutNorthArrowHandler( this ) )
{
  setBackgroundEnabled( false );
  setFrameEnabled( false );
  setReferencePoint( QgsLayoutItem::Middle );
  QVariantMap properties;
  properties.insert( u"size"_s, u"4"_s );
  mShapeStyleSymbol = QgsMarkerSymbol::createSimple( properties );
  refreshSymbol();

  connect( this, &QgsLayoutItemMarker::sizePositionChanged, this, [this]
  {
    updateBoundingRect();
    update();
  } );

  connect( mNorthArrowHandler, &QgsLayoutNorthArrowHandler::arrowRotationChanged, this, &QgsLayoutItemMarker::northArrowRotationChanged );
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
  return QgsApplication::getThemeIcon( u"/mLayoutItemMarker.svg"_s );
}

void QgsLayoutItemMarker::refreshSymbol()
{
  if ( auto *lLayout = layout() )
  {
    QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( lLayout, nullptr, lLayout->renderContext().dpi() );

    std::unique_ptr< QgsMarkerSymbol > sym( mShapeStyleSymbol->clone() );
    sym->setAngle( sym->angle() + mNorthArrowRotation );
    sym->startRender( rc );
    QRectF bounds = sym->bounds( QPointF( 0, 0 ), rc );
    sym->stopRender( rc );
    mPoint = QPointF( -bounds.left() * 25.4 / lLayout->renderContext().dpi(),
                      -bounds.top() * 25.4 / lLayout->renderContext().dpi() );
    bounds.translate( mPoint );

    const QgsLayoutSize newSizeMm = QgsLayoutSize( bounds.size()  * 25.4 / lLayout->renderContext().dpi(), Qgis::LayoutUnit::Millimeters );
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

void QgsLayoutItemMarker::northArrowRotationChanged( double rotation )
{
  mNorthArrowRotation = rotation;
  refreshSymbol();
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

void QgsLayoutItemMarker::setLinkedMap( QgsLayoutItemMap *map )
{
  mNorthArrowHandler->setLinkedMap( map );
}

QgsLayoutItemMap *QgsLayoutItemMarker::linkedMap() const
{
  return mNorthArrowHandler->linkedMap();
}

QgsLayoutNorthArrowHandler::NorthMode QgsLayoutItemMarker::northMode() const
{
  return mNorthArrowHandler->northMode();

}

void QgsLayoutItemMarker::setNorthMode( QgsLayoutNorthArrowHandler::NorthMode mode )
{
  mNorthArrowHandler->setNorthMode( mode );

}

double QgsLayoutItemMarker::northOffset() const
{
  return mNorthArrowHandler->northOffset();
}

void QgsLayoutItemMarker::setNorthOffset( double offset )
{
  mNorthArrowHandler->setNorthOffset( offset );
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

  const double scale = context.renderContext().convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );

  const QPointF shapePoint = mPoint * scale;

  std::unique_ptr< QgsMarkerSymbol > sym( mShapeStyleSymbol->clone() );
  sym->setAngle( sym->angle() + mNorthArrowRotation );
  sym->startRender( context.renderContext() );
  sym->renderPoint( shapePoint, nullptr, context.renderContext() );
  sym->stopRender( context.renderContext() );
}

bool QgsLayoutItemMarker::writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  const QDomElement shapeStyleElem = QgsSymbolLayerUtils::saveSymbol( QString(), mShapeStyleSymbol.get(), document, context );
  element.appendChild( shapeStyleElem );

  //rotation
  element.setAttribute( u"arrowRotation"_s, QString::number( mNorthArrowRotation ) );
  if ( !mNorthArrowHandler->linkedMap() )
  {
    element.setAttribute( u"mapUuid"_s, QString() );
  }
  else
  {
    element.setAttribute( u"mapUuid"_s, mNorthArrowHandler->linkedMap()->uuid() );
  }
  element.setAttribute( u"northMode"_s, mNorthArrowHandler->northMode() );
  element.setAttribute( u"northOffset"_s, mNorthArrowHandler->northOffset() );

  return true;
}

bool QgsLayoutItemMarker::readPropertiesFromElement( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext &context )
{
  const QDomElement shapeStyleSymbolElem = element.firstChildElement( u"symbol"_s );
  if ( !shapeStyleSymbolElem.isNull() )
  {
    mShapeStyleSymbol = QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( shapeStyleSymbolElem, context );
  }

  //picture rotation
  if ( !qgsDoubleNear( element.attribute( u"arrowRotation"_s, u"0"_s ).toDouble(), 0.0 ) )
  {
    mNorthArrowRotation = element.attribute( u"arrowRotation"_s, u"0"_s ).toDouble();
  }

  //rotation map
  mNorthArrowHandler->setNorthMode( static_cast< QgsLayoutNorthArrowHandler::NorthMode >( element.attribute( u"northMode"_s, u"0"_s ).toInt() ) );
  mNorthArrowHandler->setNorthOffset( element.attribute( u"northOffset"_s, u"0"_s ).toDouble() );

  mNorthArrowHandler->setLinkedMap( nullptr );
  mRotationMapUuid = element.attribute( u"mapUuid"_s );

  refreshSymbol();

  return true;
}

void QgsLayoutItemMarker::finalizeRestoreFromXml()
{
  if ( !mLayout || mRotationMapUuid.isEmpty() )
  {
    mNorthArrowHandler->setLinkedMap( nullptr );
  }
  else
  {
    mNorthArrowHandler->setLinkedMap( qobject_cast< QgsLayoutItemMap * >( mLayout->itemByUuid( mRotationMapUuid, true ) ) );
  }
  emit changed();
}
