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
#include "moc_qgslayertreeviewfilterindicator.cpp"

#include "qgslayertree.h"
#include "qgslayertreeview.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgspointcloudlayer.h"
#include "qgisapp.h"
#include "qgsstringutils.h"
#include "qgsmessagebar.h"

QgsLayerTreeViewFilterIndicatorProvider::QgsLayerTreeViewFilterIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewFilterIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
  if ( vl && vl->isEditable() )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Edit filter" ), tr( "Cannot edit filter when layer is in edit mode" ) );
    return;
  }

  QgisApp::instance()->layerSubsetString( layer );
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
    case Qgis::LayerType::Vector:
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      connect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case Qgis::LayerType::Raster:
    {
      // PG raster
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
      connect( rlayer, &QgsRasterLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case Qgis::LayerType::PointCloud:
    {
      QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( layer );
      connect( pclayer, &QgsPointCloudLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::TiledScene:
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
    case Qgis::LayerType::Vector:
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      disconnect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case Qgis::LayerType::Raster:
    {
      // PG raster
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
      disconnect( rlayer, &QgsRasterLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case Qgis::LayerType::PointCloud:
    {
      QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( layer );
      disconnect( pclayer, &QgsPointCloudLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerChanged );
      break;
    }
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::TiledScene:
      break;
  }
}

bool QgsLayerTreeViewFilterIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case Qgis::LayerType::Vector:
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      return !vlayer->subsetString().isEmpty();
    }
    case Qgis::LayerType::Raster:
    {
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
      return !rlayer->subsetString().isEmpty();
    }
    case Qgis::LayerType::PointCloud:
    {
      QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( layer );
      return !pclayer->subsetString().isEmpty();
    }
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::TiledScene:
      break;
  }
  return false;
}
