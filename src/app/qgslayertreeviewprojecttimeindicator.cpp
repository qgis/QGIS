/***************************************************************************
                         qgslayertreeviewprojecttimeindicator.cpp
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewprojecttimeindicator.h"
#include "qgslayertreeview.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgsrasterlayer.h"
#include "qgisapp.h"

QgsLayerTreeViewProjectTimeIndicatorProvider::QgsLayerTreeViewProjectTimeIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewProjectTimeIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  if ( !( qobject_cast<QgsVectorLayer *>( layer ) || qobject_cast<QgsRasterLayer *>( layer ) ) )
    return;
  QgsMapLayer *mapLayer = layer;
  connect( mapLayer, &QgsMapLayer::dataSourceChanged, this, &QgsLayerTreeViewProjectTimeIndicatorProvider::onLayerChanged );

  if ( mapLayer && mapLayer->dataProvider() && mapLayer->dataProvider()->temporalCapabilities() )
  {
    mLayer = mapLayer;
    QgsRasterDataProvider *provider = qobject_cast<QgsRasterDataProvider *>( mapLayer->dataProvider() );
    connect( provider, &QgsRasterDataProvider::statusChanged,
             this, &QgsLayerTreeViewProjectTimeIndicatorProvider::onLayerChanged );
  }
}

void QgsLayerTreeViewProjectTimeIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  Q_UNUSED( index )
}

bool QgsLayerTreeViewProjectTimeIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  if ( !layer )
    return false;
  if ( layer->temporalProperties() &&
       layer->temporalProperties()->temporalSource() ==
       QgsMapLayerTemporalProperties::TemporalSource::Project )
    return true;
  return false;
}

QString QgsLayerTreeViewProjectTimeIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return QStringLiteral( "/mIndicatorTimeFromProject.svg" );
}

QString QgsLayerTreeViewProjectTimeIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return tr( "<b>Temporal layer using Project time </b>" );
}

void QgsLayerTreeViewProjectTimeIndicatorProvider::onLayerChanged()
{
  QgsMapLayer *mapLayer = qobject_cast<QgsMapLayer *>( sender() );
  QgsMapLayer *mapLayerFromIndicator = qobject_cast<QgsMapLayer *>( mLayer );

  if ( !mapLayer )
  {
    if ( !mapLayerFromIndicator )
      return;
    else
      updateLayerIndicator( mapLayerFromIndicator );
  }
  else
    updateLayerIndicator( mapLayer );
}

