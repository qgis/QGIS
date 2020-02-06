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

QgsMaskMarkerSymbolLayer::QgsMaskMarkerSymbolLayer()
{
  mSymbol.reset( static_cast<QgsMarkerSymbol *>( QgsMarkerSymbol::createSimple( QgsStringMap() ) ) );
}

bool QgsMaskMarkerSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( symbol && symbol->type() == QgsSymbol::Marker )
  {
    mSymbol.reset( static_cast<QgsMarkerSymbol *>( symbol ) );
    return true;
  }
  delete symbol;
  return false;
}

QgsSymbolLayer *QgsMaskMarkerSymbolLayer::create( const QgsStringMap &props )
{
  QgsMaskMarkerSymbolLayer *l = new QgsMaskMarkerSymbolLayer();

  l->setSubSymbol( QgsMarkerSymbol::createSimple( props ) );

  if ( props.contains( QStringLiteral( "mask_symbollayers" ) ) )
  {
    l->setMasks( stringToSymbolLayerReferenceList( props[QStringLiteral( "mask_symbollayers" )] ) );
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

QString QgsMaskMarkerSymbolLayer::layerType() const
{
  return QStringLiteral( "MaskMarker" );
}

QgsStringMap QgsMaskMarkerSymbolLayer::properties() const
{
  QgsStringMap props;
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
  if ( paintEffect() )
  {
    mEffect.reset( paintEffect()->clone() );
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

QRectF QgsMaskMarkerSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  return mSymbol->bounds( point, context.renderContext() );
}

void QgsMaskMarkerSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  if ( !context.renderContext().painter() )
    return;

  if ( context.renderContext().isGuiPreview() )
  {
    mSymbol->renderPoint( point, context.feature(), context.renderContext(), /* layer = */ -1, /* selected = */ false );
    return;
  }

  if ( ! context.renderContext().maskPainter() )
    return;

  if ( mMaskedSymbolLayers.isEmpty() )
    return;

  {
    // Otherwise switch to the mask painter before rendering
    QgsPainterSwapper swapper( context.renderContext(), context.renderContext().maskPainter() );

    // Special case when an effect is defined on this mask symbol layer
    // (effects defined on sub symbol's layers do not need special handling)
    if ( mEffect && mEffect->enabled() )
    {
      QgsEffectPainter p( context.renderContext() );
      // translate operates on the mask painter, which is what we want
      p->translate( point );
      p.setEffect( mEffect.get() );
      mSymbol->renderPoint( QPointF( 0, 0 ), context.feature(), context.renderContext(), /* layer = */ -1, /* selected = */ false );
      // the translation will be canceled at the end of scope here
    }
    else
    {
      mSymbol->renderPoint( point, context.feature(), context.renderContext(), /* layer = */ -1, /* selected = */ false );
    }
  }
}


