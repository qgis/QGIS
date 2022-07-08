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

#include <QMimeData>

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

              switch ( elevationProperties->type() )
              {
                case Qgis::VectorProfileType::IndividualFeatures:
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

                  if ( elevationProperties->respectLayerSymbology() )
                  {
                    if ( QgsSingleSymbolRenderer *renderer = dynamic_cast< QgsSingleSymbolRenderer * >( qobject_cast< QgsVectorLayer * >( layer )->renderer() ) )
                    {
                      if ( renderer->symbol()->type() == symbol->type() )
                      {
                        // take the whole renderer symbol if we can
                        symbol.reset( renderer->symbol()->clone() );
                      }
                      else
                      {
                        // otherwise emulate what happens when rendering the actual chart and just copy the color and opacity
                        symbol->setColor( renderer->symbol()->color() );
                        symbol->setOpacity( renderer->symbol()->opacity() );
                      }
                    }
                    else
                    {
                      // just use default layer icon
                      return QgsLayerTreeModel::data( index, role );
                    }
                  }
                  break;

                case Qgis::VectorProfileType::ContinuousSurface:
                  switch ( elevationProperties->profileSymbology() )
                  {
                    case Qgis::ProfileSurfaceSymbology::Line:
                      if ( QgsLineSymbol *lineSymbol = elevationProperties->profileLineSymbol() )
                      {
                        symbol.reset( lineSymbol->clone() );
                      }
                      break;
                    case Qgis::ProfileSurfaceSymbology::FillBelow:
                      if ( QgsFillSymbol *fillSymbol = elevationProperties->profileFillSymbol() )
                      {
                        symbol.reset( fillSymbol->clone() );
                      }
                      break;
                  }
                  break;

              }
              break;
            }

            case QgsMapLayerType::RasterLayer:
            {
              QgsRasterLayerElevationProperties *rlProps = qgis::down_cast< QgsRasterLayerElevationProperties * >( layer->elevationProperties() );
              switch ( rlProps->profileSymbology() )
              {
                case Qgis::ProfileSurfaceSymbology::Line:
                  if ( QgsLineSymbol *lineSymbol = rlProps->profileLineSymbol() )
                  {
                    symbol.reset( lineSymbol->clone() );
                  }
                  break;
                case Qgis::ProfileSurfaceSymbology::FillBelow:
                  if ( QgsFillSymbol *fillSymbol = rlProps->profileFillSymbol() )
                  {
                    symbol.reset( fillSymbol->clone() );
                  }
                  break;
              }
              break;
            }

            case QgsMapLayerType::MeshLayer:
            {
              QgsMeshLayerElevationProperties *mlProps = qgis::down_cast< QgsMeshLayerElevationProperties * >( layer->elevationProperties() );
              switch ( mlProps->profileSymbology() )
              {
                case Qgis::ProfileSurfaceSymbology::Line:
                  if ( QgsLineSymbol *lineSymbol = mlProps->profileLineSymbol() )
                  {
                    symbol.reset( lineSymbol->clone() );
                  }
                  break;
                case Qgis::ProfileSurfaceSymbology::FillBelow:
                  if ( QgsFillSymbol *fillSymbol = mlProps->profileFillSymbol() )
                  {
                    symbol.reset( fillSymbol->clone() );
                  }
                  break;
              }
              break;
            }

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

    case Qt::ToolTipRole:
    {
      // override default tooltip with elevation specific one
      QgsLayerTreeNode *node = index2node( index );
      if ( node && node->nodeType() == QgsLayerTreeNode::NodeLayer )
      {
        if ( QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer() )
        {
          QString title =
            !layer->title().isEmpty() ? layer->title() :
            !layer->shortName().isEmpty() ? layer->shortName() :
            layer->name();

          title = "<b>" + title.toHtmlEscaped() + "</b>";

          QStringList parts;
          parts << title;

          const QString elevationPropertiesSummary = layer->elevationProperties() ? layer->elevationProperties()->htmlSummary() : QString();
          if ( !elevationPropertiesSummary.isEmpty( ) )
            parts << elevationPropertiesSummary;

          return parts.join( QLatin1String( "<br/>" ) );
        }
      }
      break;
    }

    default:
      break;
  }
  return QgsLayerTreeModel::data( index, role );
}

bool QgsElevationProfileLayerTreeModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) )
    return false;

  // don't accept drags from other layer trees -- only allow internal drag
  const QString source = data->data( QStringLiteral( "application/qgis.layertree.source" ) );
  if ( source.isEmpty() || source != QStringLiteral( ":0x%1" ).arg( reinterpret_cast<quintptr>( this ), 2 * QT_POINTER_SIZE, 16, QLatin1Char( '0' ) ) )
    return false;

  return QgsLayerTreeModel::dropMimeData( data, action, row, column, parent );
}

QMimeData *QgsElevationProfileLayerTreeModel::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData *mimeData = QgsLayerTreeModel::mimeData( indexes );
  if ( mimeData )
  {
    mimeData->setData( QStringLiteral( "application/qgis.restrictlayertreemodelsubclass" ), "QgsElevationProfileLayerTreeModel" );
  }
  return mimeData;
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
