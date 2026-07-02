/***************************************************************************
  qgslayerstylewatcher.cpp
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Oslandia
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayerstylewatcher.h"

#include "qgs3dmapsettings.h"

#include "moc_qgslayerstylewatcher.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE
///
QgsLayerStyleWatcher::QgsLayerStyleWatcher( Qgs3DMapSettings *mapSettings )
  : QObject()
  , mMapSettings( mapSettings )
{
  onLayersChanged();
  connect( mMapSettings, &Qgs3DMapSettings::layersChanged, this, &QgsLayerStyleWatcher::onLayersChanged );
}

void QgsLayerStyleWatcher::onLayersChanged()
{
  // disconnect all watched layers
  const QList<QgsMapLayer *> keys = mLayers.keys();
  for ( QgsMapLayer *layer : keys )
  {
    disconnect( layer, &QgsMapLayer::renderer3DChanged, this, &QgsLayerStyleWatcher::onLayer3DRendererChanged );
    disconnect( layer, &QgsMapLayer::repaintRequested, this, &QgsLayerStyleWatcher::onLayerRepaintRequested );
    disconnect( layer, &QgsMapLayer::destroyed, this, &QgsLayerStyleWatcher::onLayerDestroyed );
  }

  mLayers.clear();

  // connect on all layer renderer3DChanged and styleChanged signals
  // then keep if they have or not a 3D renderer
  const QList<QgsMapLayer *> layers = mMapSettings->layers();
  for ( QgsMapLayer *layer : layers )
  {
    mLayers[layer] = static_cast< bool >( layer->renderer3D() );
    connect( layer, &QgsMapLayer::renderer3DChanged, this, &QgsLayerStyleWatcher::onLayer3DRendererChanged );
    connect( layer, &QgsMapLayer::repaintRequested, this, &QgsLayerStyleWatcher::onLayerRepaintRequested );
    connect( layer, &QgsMapLayer::destroyed, this, &QgsLayerStyleWatcher::onLayerDestroyed );
  }

  emit styleChanged();
}

void QgsLayerStyleWatcher::onLayerRepaintRequested()
{
  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() ) )
  {
    // if layer has no 3D renderer and its 2D style changed, we must invalidate the map images.
    if ( mLayers.contains( layer ) && !static_cast< bool >( layer->renderer3D() ) )
    {
      emit styleChanged();
    }
  }
}

void QgsLayerStyleWatcher::onLayer3DRendererChanged()
{
  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() ) )
  {
    // if layer has gone from having a 3d renderer to not having one, or vice versa, we must invalidate the map images.
    if ( mLayers.contains( layer ) && mLayers[layer] != static_cast< bool >( layer->renderer3D() ) )
    {
      mLayers[layer] = static_cast< bool >( layer->renderer3D() );
      emit styleChanged();
    }
  }
}
void QgsLayerStyleWatcher::onLayerDestroyed()
{
  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() ) )
  {
    mLayers.remove( layer );
  }
}

/// @endcond
