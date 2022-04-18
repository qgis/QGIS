/***************************************************************************
                          qgselevationprofilelayertreemodel.cpp
                          -----------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgselevationprofilelayertreemodel.h"
#include "qgslayertreenode.h"
#include "qgslayertree.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayerelevationproperties.h"
#include "qgsmeshlayerelevationproperties.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgsvectorlayer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"

QgsElevationProfileLayerTreeModel::QgsElevationProfileLayerTreeModel( QgsLayerTree *rootNode, QObject *parent )
  : QgsLayerTreeModel( rootNode, parent )
{
  setFlag( QgsLayerTreeModel::AllowNodeReorder );
  setFlag( QgsLayerTreeModel::AllowNodeChangeVisibility );
  setFlag( QgsLayerTreeModel::ShowLegendAsTree );
  setFlag( QgsLayerTreeModel::AllowLegendChangeState, false );
}

QVariant QgsElevationProfileLayerTreeModel::data( const QModelIndex &index, int role ) const
{
  switch ( role )
  {
    case Qt::DecorationRole:
    {
      QgsLayerTreeNode *node = index2node( index );

      if ( node && node->nodeType() == QgsLayerTreeNode::NodeLayer )
      {
        if ( QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer() )
        {
          std::unique_ptr<QgsRenderContext> context( createTemporaryRenderContext() );

          const int iconSize = scaleIconSize( 16 );
          std::unique_ptr< QgsSymbol > symbol;
          switch ( layer->type() )
          {
            case QgsMapLayerType::VectorLayer:
            {
              QgsVectorLayerElevationProperties *elevationProperties = qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() );
              QgsVectorLayer *vLayer = qobject_cast< QgsVectorLayer * >( layer );

              if ( ( vLayer->geometryType() == QgsWkbTypes::PointGeometry && !elevationProperties->extrusionEnabled() )
                   || ( vLayer->geometryType() == QgsWkbTypes::LineGeometry && !elevationProperties->extrusionEnabled() )
                 )
              {
                if ( QgsMarkerSymbol *markerSymbol = elevationProperties->profileMarkerSymbol() )
                {
                  symbol.reset( markerSymbol->clone() );
                }
              }

              if ( !symbol && vLayer->geometryType() == QgsWkbTypes::PolygonGeometry && elevationProperties->extrusionEnabled() )
              {
                if ( QgsFillSymbol *fillSymbol = elevationProperties->profileFillSymbol() )
                {
                  symbol.reset( fillSymbol->clone() );
                }
              }

              if ( !symbol )
              {
                if ( QgsLineSymbol *lineSymbol = elevationProperties->profileLineSymbol() )
                {
                  symbol.reset( lineSymbol->clone() );
                }
              }

              if ( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->respectLayerSymbology() )
              {
                if ( QgsSingleSymbolRenderer *renderer = dynamic_cast< QgsSingleSymbolRenderer * >( qobject_cast< QgsVectorLayer * >( layer )->renderer() ) )
                {
                  symbol->setColor( renderer->symbol()->color() );
                  symbol->setOpacity( renderer->symbol()->opacity() );
                }
                else
                {
                  // just use default layer icon
                  return QgsLayerTreeModel::data( index, role );
                }
              }
              break;
            }

            case QgsMapLayerType::RasterLayer:
              if ( QgsLineSymbol *lineSymbol = qgis::down_cast< QgsRasterLayerElevationProperties * >( layer->elevationProperties() )->profileLineSymbol() )
              {
                symbol.reset( lineSymbol->clone() );
              }
              break;

            case QgsMapLayerType::MeshLayer:
              if ( QgsLineSymbol *lineSymbol = qgis::down_cast< QgsMeshLayerElevationProperties * >( layer->elevationProperties() )->profileLineSymbol() )
              {
                symbol.reset( lineSymbol->clone() );
              }
              break;

            case QgsMapLayerType::PluginLayer:
            case QgsMapLayerType::VectorTileLayer:
            case QgsMapLayerType::AnnotationLayer:
            case QgsMapLayerType::PointCloudLayer:
            case QgsMapLayerType::GroupLayer:
              break;
          }
          if ( !symbol )
            break;

          const QPixmap pix = QgsSymbolLayerUtils::symbolPreviewPixmap( symbol.get(), QSize( iconSize, iconSize ), 0, context.get() );
          return QIcon( pix );
        }
      }
      break;
    }
    default:
      break;
  }
  return QgsLayerTreeModel::data( index, role );
}


//
// QgsElevationProfileLayerTreeProxyModel
//

QgsElevationProfileLayerTreeProxyModel::QgsElevationProfileLayerTreeProxyModel( QgsElevationProfileLayerTreeModel *model, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mModel( model )
{
  setSourceModel( mModel );
  setDynamicSortFilter( true );
}

bool QgsElevationProfileLayerTreeProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  const QModelIndex sourceIndex = mModel->index( sourceRow, 0, sourceParent );
  if ( QgsLayerTreeNode *node = mModel->index2node( sourceIndex ) )
  {
    switch ( node->nodeType() )
    {
      case QgsLayerTreeNode::NodeLayer:
      {
        if ( QgsLayerTreeLayer *layerTreeLayer = QgsLayerTree::toLayer( node ) )
        {
          if ( QgsMapLayer *layer = layerTreeLayer->layer() )
          {
            // hide layers which don't have elevation
            if ( !layer->elevationProperties() || !layer->elevationProperties()->hasElevation() )
              return false;
          }
        }
        break;
      }

      case QgsLayerTreeNode::NodeGroup:
        break;
    }
    return true;
  }
  else if ( QgsLayerTreeModelLegendNode *legendNode = mModel->index2legendNode( sourceIndex ) )
  {
    // we only show legend nodes for vector layers where the profile symbol is set to follow the layer's symbology
    // (and the layer's renderer isn't a single symbol renderer)
    if ( QgsLayerTreeLayer *layerTreeLayer = QgsLayerTree::toLayer( legendNode->layerNode() ) )
    {
      if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( layerTreeLayer->layer() ) )
      {
        if ( !qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->respectLayerSymbology() )
        {
          return false;
        }
        else if ( dynamic_cast< QgsSingleSymbolRenderer * >( qobject_cast< QgsVectorLayer * >( layer )->renderer() ) )
        {
          return false;
        }
        else
        {
          return true;
        }
      }
    }
    return false;
  }
  else
  {
    return false;
  }
}
