/***************************************************************************
  qgslayertreeviewfilterindicator.cpp
  --------------------------------------
  Date                 : January 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewfilterindicator.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeview.h"
#include "qgsquerybuilder.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgspointcloudlayer.h"
#include "qgisapp.h"
#include "qgsstringutils.h"

QgsLayerTreeViewFilterIndicatorProvider::QgsLayerTreeViewFilterIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewFilterIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgisApp::instance()->layerSubsetString( QgsLayerTree::toLayer( node )->layer() );

}

QString QgsLayerTreeViewFilterIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer )
  return QStringLiteral( "/mIndicatorFilter.svg" );
}

QString QgsLayerTreeViewFilterIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  QString filter;
  if ( vlayer )
    filter = vlayer->subsetString();

  // PG raster
  QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( rlayer && rlayer->dataProvider() && rlayer->dataProvider()->supportsSubsetString() )
    filter = rlayer->subsetString();

  QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( layer );
  if ( pclayer && pclayer->dataProvider() && pclayer->dataProvider()->supportsSubsetString() )
    filter = pclayer->subsetString();

  if ( filter.isEmpty() )
    return QString();

  if ( filter.size() > 1024 )
  {
    filter = QgsStringUtils::truncateMiddleOfString( filter, 1024 );
  }

  return QStringLiteral( "<b>%1:</b><br>%2" ).arg( tr( "Filter" ), filter.toHtmlEscaped() );
}

void QgsLayerTreeViewFilterIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::connectSignals( layer );
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      connect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case QgsMapLayerType::RasterLayer:
    {
      // PG raster
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
      connect( rlayer, &QgsRasterLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case QgsMapLayerType::PointCloudLayer:
    {
      QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( layer );
      connect( pclayer, &QgsPointCloudLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::GroupLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::VectorTileLayer:
      break;
  }
}

void QgsLayerTreeViewFilterIndicatorProvider::disconnectSignals( QgsMapLayer *layer )
{
  if ( !layer )
    return;

  QgsLayerTreeViewIndicatorProvider::disconnectSignals( layer );
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      disconnect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case QgsMapLayerType::RasterLayer:
    {
      // PG raster
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
      disconnect( rlayer, &QgsRasterLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case QgsMapLayerType::PointCloudLayer:
    {
      QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( layer );
      disconnect( pclayer, &QgsPointCloudLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::GroupLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::VectorTileLayer:
      break;
  }
}

bool QgsLayerTreeViewFilterIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      return ! vlayer->subsetString().isEmpty();
    }
    case QgsMapLayerType::RasterLayer:
    {
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
      return ! rlayer->subsetString().isEmpty();
    }
    case QgsMapLayerType::PointCloudLayer:
    {
      QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( layer );
      return ! pclayer->subsetString().isEmpty();
    }
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::GroupLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::VectorTileLayer:
      break;
  }
  return false;
}
