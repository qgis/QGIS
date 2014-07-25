/***************************************************************************
    qgspluginlayer.cpp
    ---------------------
    begin                : January 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspluginlayer.h"

#include "qgsmaplayerlegend.h"
#include "qgsmaplayerrenderer.h"

QgsPluginLayer::QgsPluginLayer( QString layerType, QString layerName )
    : QgsMapLayer( PluginLayer, layerName ), mPluginLayerType( layerType )
{
  setLegend( QgsMapLayerLegend::defaultPluginLegend( this ) );
}

QString QgsPluginLayer::pluginLayerType()
{
  return mPluginLayerType;
}

void QgsPluginLayer::setExtent( const QgsRectangle &extent )
{
  mExtent = extent;
}

QgsLegendSymbologyList QgsPluginLayer::legendSymbologyItems( const QSize& iconSize )
{
  Q_UNUSED( iconSize );
  return QgsLegendSymbologyList();
}

/** Fallback layer renderer implementation for layer that do not support map renderer yet.
 *
 * @note added in 2.4
 */
class QgsPluginLayerRenderer : public QgsMapLayerRenderer
{
  public:
    QgsPluginLayerRenderer( QgsPluginLayer* layer, QgsRenderContext& rendererContext )
        : QgsMapLayerRenderer( layer->id() )
        , mLayer( layer )
        , mRendererContext( rendererContext )
    {}

    virtual bool render()
    {
      return mLayer->draw( mRendererContext );
    }

  protected:
    QgsPluginLayer* mLayer;
    QgsRenderContext& mRendererContext;
};

QgsMapLayerRenderer* QgsPluginLayer::createMapRenderer( QgsRenderContext& rendererContext )
{
  return new QgsPluginLayerRenderer( this, rendererContext );
}
