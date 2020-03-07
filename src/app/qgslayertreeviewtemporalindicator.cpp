/***************************************************************************
                         qgslayertreeviewtemporalindicator.cpp
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

#include "qgslayertreeviewtemporalindicator.h"
#include "qgslayertreeview.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgsrasterlayer.h"
#include "qgisapp.h"

QgsLayerTreeViewTemporalIndicatorProvider::QgsLayerTreeViewTemporalIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewTemporalIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  if ( !( qobject_cast<QgsVectorLayer *>( layer ) || qobject_cast<QgsRasterLayer *>( layer ) ) )
    return;
  QgsMapLayer *mapLayer = layer;
  connect( mapLayer, &QgsMapLayer::dataSourceChanged, this, &QgsLayerTreeViewTemporalIndicatorProvider::onLayerChanged );

  if ( mapLayer && mapLayer->dataProvider() )
  {
    mLayer = mapLayer;
    QgsRasterDataProvider *provider = qobject_cast<QgsRasterDataProvider *>( mapLayer->dataProvider() );
    connect( provider, &QgsRasterDataProvider::statusChanged,
             this, &QgsLayerTreeViewTemporalIndicatorProvider::onLayerChanged );
  }
}

void QgsLayerTreeViewTemporalIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  Q_UNUSED( index )
}

bool QgsLayerTreeViewTemporalIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  if ( !layer )
    return false;
  if ( layer->temporalProperties() )
    return true;
  return false;
}

QString QgsLayerTreeViewTemporalIndicatorProvider::iconName( QgsMapLayer *layer )
{
  if ( layer->temporalProperties()->temporalSource() ==
       QgsMapLayerTemporalProperties::TemporalSource::Project )
    return QStringLiteral( "/mIndicatorTimeFromProject.svg" );

  return QStringLiteral( "/mIndicatorTemporal.svg" );
}

QString QgsLayerTreeViewTemporalIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  if ( layer->temporalProperties()->temporalSource() ==
       QgsMapLayerTemporalProperties::TemporalSource::Project )
    return tr( "<b>Temporal layer using Project time </b>" );

  return tr( "<b>Temporal layer</b>" );
}

void QgsLayerTreeViewTemporalIndicatorProvider::onLayerChanged()
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
