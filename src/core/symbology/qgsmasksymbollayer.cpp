/***************************************************************************
 qgsmasksymbollayer.cpp
 ---------------------
 begin                : July 2019
 copyright            : (C) 2019 by Hugo Mercier
 email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmasksymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgspainteffect.h"
#include "qgspainterswapper.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgssymbollayerreference.h"
#include "qgsmaskpaintdevice.h"

QgsMaskMarkerSymbolLayer::QgsMaskMarkerSymbolLayer()
{
  mSymbol.reset( static_cast<QgsMarkerSymbol *>( QgsMarkerSymbol::createSimple( QVariantMap() ) ) );
}

QgsMaskMarkerSymbolLayer::~QgsMaskMarkerSymbolLayer() = default;

bool QgsMaskMarkerSymbolLayer::enabled() const
{
  return !mMaskedSymbolLayers.isEmpty();
}

bool QgsMaskMarkerSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( symbol && symbol->type() == Qgis::SymbolType::Marker )
  {
    mSymbol.reset( static_cast<QgsMarkerSymbol *>( symbol ) );
    return true;
  }
  delete symbol;
  return false;
}

QgsSymbolLayer *QgsMaskMarkerSymbolLayer::create( const QVariantMap &props )
{
  QgsMaskMarkerSymbolLayer *l = new QgsMaskMarkerSymbolLayer();

  l->setSubSymbol( QgsMarkerSymbol::createSimple( props ) );

  if ( props.contains( QStringLiteral( "mask_symbollayers" ) ) )
  {
    l->setMasks( stringToSymbolLayerReferenceList( props[QStringLiteral( "mask_symbollayers" )].toString() ) );
  }
  return l;
}

QgsMaskMarkerSymbolLayer *QgsMaskMarkerSymbolLayer::clone() const
{
  QgsMaskMarkerSymbolLayer *l = static_cast<QgsMaskMarkerSymbolLayer *>( create( properties() ) );
  l->setSubSymbol( mSymbol->clone() );
  l->setMasks( mMaskedSymbolLayers );
  copyDataDefinedProperties( l );
  copyPaintEffect( l );
  return l;
}

QgsSymbol *QgsMaskMarkerSymbolLayer::subSymbol()
{
  return mSymbol.get();
}

QString QgsMaskMarkerSymbolLayer::layerType() const
{
  return QStringLiteral( "MaskMarker" );
}

QVariantMap QgsMaskMarkerSymbolLayer::properties() const
{
  QVariantMap props;
  props[QStringLiteral( "mask_symbollayers" )] = symbolLayerReferenceListToString( masks() );
  return props;
}

QSet<QString> QgsMaskMarkerSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes = QgsMarkerSymbolLayer::usedAttributes( context );

  attributes.unite( mSymbol->usedAttributes( context ) );

  return attributes;
}

bool QgsMaskMarkerSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mSymbol && mSymbol->hasDataDefinedProperties() )
    return true;
  return false;
}

void QgsMaskMarkerSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  // since we need to swap the regular painter with the mask painter during rendering,
  // effects won't work. So we cheat by handling effects ourselves in renderPoint
  if ( auto *lPaintEffect = paintEffect() )
  {
    mEffect.reset( lPaintEffect->clone() );
    setPaintEffect( nullptr );
  }
  mSymbol->startRender( context.renderContext() );
}

void QgsMaskMarkerSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  mSymbol->stopRender( context.renderContext() );
  if ( mEffect )
  {
    setPaintEffect( mEffect.release() );
  }
}

void QgsMaskMarkerSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext &context, QSize size )
{
  QgsMarkerSymbolLayer::drawPreviewIcon( context, size );
}

QList<QgsSymbolLayerReference> QgsMaskMarkerSymbolLayer::masks() const
{
  return mMaskedSymbolLayers;
}

void QgsMaskMarkerSymbolLayer::setMasks( const QList<QgsSymbolLayerReference> &maskedLayers )
{
  mMaskedSymbolLayers = maskedLayers;
}

QRectF QgsMaskMarkerSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  return mSymbol->bounds( point, context.renderContext() );
}

bool QgsMaskMarkerSymbolLayer::usesMapUnits() const
{
  return mSizeUnit == QgsUnitTypes::RenderMapUnits || mSizeUnit == QgsUnitTypes::RenderMetersInMapUnits
         || ( mSymbol && mSymbol->usesMapUnits() );
}

void QgsMaskMarkerSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsMarkerSymbolLayer::setOutputUnit( unit );
  if ( mSymbol )
    mSymbol->setOutputUnit( unit );
}

QColor QgsMaskMarkerSymbolLayer::color() const
{
  return QColor();
}

void QgsMaskMarkerSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  QgsRenderContext &renderContext = context.renderContext();
  if ( !renderContext.painter() )
    return;

  if ( renderContext.isGuiPreview() )
  {
    mSymbol->renderPoint( point, context.feature(), renderContext, /* layer = */ -1, /* selected = */ false );
    return;
  }

  if ( !renderContext.maskPainter() )
    return;

  if ( mMaskedSymbolLayers.isEmpty() )
    return;

  // Otherwise switch to the mask painter before rendering
  const QgsPainterSwapper swapper( renderContext, renderContext.maskPainter() );

  // Special case when an effect is defined on this mask symbol layer
  // (effects defined on sub symbol's layers do not need special handling)
  if ( mEffect && mEffect->enabled() )
  {
    QgsEffectPainter p( renderContext );
    // translate operates on the mask painter, which is what we want
    p->translate( point );
    p.setEffect( mEffect.get() );
    mSymbol->renderPoint( QPointF( 0, 0 ), context.feature(), renderContext, /* layer = */ -1, /* selected = */ false );
    // the translation will be canceled at the end of scope here
  }
  else
  {
    mSymbol->renderPoint( point, context.feature(), renderContext, /* layer = */ -1, /* selected = */ false );
  }
}
