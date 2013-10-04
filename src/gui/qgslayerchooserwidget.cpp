/***************************************************************************
                             QgsLayerChooserwidget.cpp
                             -------------------------
    begin                : September 2013
    copyright            : (C) 2013 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "qgslayerchooserwidget.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"

QgsLayerChooserWidget::DisplayStatus QgsLayerChooserWidget::VectorLayerFilter::acceptLayer( QgsMapLayer* layer )
{
  if ( layer->type() == QgsMapLayer::VectorLayer )
    return enabled;
  else
    return hidden;
}

QgsLayerChooserWidget::DisplayStatus QgsLayerChooserWidget::RasterLayerFilter::acceptLayer( QgsMapLayer* layer )
{
  if ( layer->type() == QgsMapLayer::RasterLayer )
    return enabled;
  else
    return hidden;
}


QgsLayerChooserWidget::QgsLayerChooserWidget( QObject *parent )
    : QObject( parent )
    , mFilter( new LayerFilter() )
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( populateLayers( QList<QgsMapLayer*> ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersRemoved( QStringList ) ), this, SLOT( populateLayers( QStringList ) ) );
}

void QgsLayerChooserWidget::setFilter( QgsLayerChooserWidget::LayerFilter* filter )
{
  mFilter = filter;
}

bool QgsLayerChooserWidget::initWidget( QWidget* widget )
{
  Q_UNUSED( widget );
  return false;
}

void QgsLayerChooserWidget::setLayer( QString layerid )
{
  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerid );
  setLayer( layer );
}

QString QgsLayerChooserWidget::getLayerId() const
{
  QgsMapLayer* layer = getLayer();
  if ( !layer )
    return QString();

  return layer->id();
}

void QgsLayerChooserWidget::populateLayers()
{
  clearWidget();

  QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
  QStringList layerIds = layers.keys();
  layerIds.sort();
  for ( QStringList::const_iterator layerId = layerIds.begin(); layerId != layerIds.end(); ++layerId )
  {
    QgsMapLayer* layer = layers.value( *layerId );
    DisplayStatus display = mFilter->acceptLayer( layer );
    if ( display != hidden )
      addLayer( layer, display );
  }
}



