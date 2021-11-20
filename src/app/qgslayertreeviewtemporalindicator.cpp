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
#include "qgsmeshlayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgisapp.h"

QgsLayerTreeViewTemporalIndicatorProvider::QgsLayerTreeViewTemporalIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewTemporalIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  if ( !layer || !layer->temporalProperties() )
    return;

  connect( layer->temporalProperties(), &QgsMapLayerTemporalProperties::changed, this, [ this, layer ]( ) { this->onLayerChanged( layer ); } );
}

void QgsLayerTreeViewTemporalIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
  if ( !layer )
    return;

  switch ( layer->type() )
  {
    case QgsMapLayerType::RasterLayer:
      QgisApp::instance()->showLayerProperties( layer, QStringLiteral( "mOptsPage_Temporal" ) );
      break;
    case QgsMapLayerType::MeshLayer:
      QgisApp::instance()->showLayerProperties( layer, QStringLiteral( "mOptsPage_Temporal" ) );
      break;
    case QgsMapLayerType::VectorLayer:
      QgisApp::instance()->showLayerProperties( layer, QStringLiteral( "mOptsPage_Temporal" ) );
      break;
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::VectorTileLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::GroupLayer:
      break;
  }
}

bool QgsLayerTreeViewTemporalIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  if ( !layer )
    return false;
  if ( layer->temporalProperties() &&
       layer->temporalProperties()->isActive() )
    return true;
  return false;
}

QString QgsLayerTreeViewTemporalIndicatorProvider::iconName( QgsMapLayer * )
{
  return QStringLiteral( "/mIndicatorTemporal.svg" );
}

QString QgsLayerTreeViewTemporalIndicatorProvider::tooltipText( QgsMapLayer * )
{
  return tr( "<b>Temporal layer</b>" );
}

void QgsLayerTreeViewTemporalIndicatorProvider::onLayerChanged( QgsMapLayer *layer )
{
  if ( !layer )
    return;
  updateLayerIndicator( layer );
}
