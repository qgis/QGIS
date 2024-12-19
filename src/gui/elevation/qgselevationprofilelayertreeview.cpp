/***************************************************************************
                          qgselevationprofilelayertreeview.cpp
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

#include "qgselevationprofilelayertreeview.h"
#include "moc_qgselevationprofilelayertreeview.cpp"
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
#include "qgsmaplayerutils.h"

#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>
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
          std::unique_ptr<QgsSymbol> symbol;
          switch ( layer->type() )
          {
            case Qgis::LayerType::Vector:
            {
              QgsVectorLayerElevationProperties *elevationProperties = qgis::down_cast<QgsVectorLayerElevationProperties *>( layer->elevationProperties() );
              QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );

              switch ( elevationProperties->type() )
              {
                case Qgis::VectorProfileType::IndividualFeatures:
                  if ( ( vLayer->geometryType() == Qgis::GeometryType::Point && !elevationProperties->extrusionEnabled() )
                       || ( vLayer->geometryType() == Qgis::GeometryType::Line && !elevationProperties->extrusionEnabled() ) )
                  {
                    if ( QgsMarkerSymbol *markerSymbol = elevationProperties->profileMarkerSymbol() )
                    {
                      symbol.reset( markerSymbol->clone() );
                    }
                  }

                  if ( !symbol && vLayer->geometryType() == Qgis::GeometryType::Polygon && elevationProperties->extrusionEnabled() )
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
                    if ( QgsSingleSymbolRenderer *renderer = dynamic_cast<QgsSingleSymbolRenderer *>( qobject_cast<QgsVectorLayer *>( layer )->renderer() ) )
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
                    case Qgis::ProfileSurfaceSymbology::FillAbove:
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

            case Qgis::LayerType::Raster:
            {
              QgsRasterLayerElevationProperties *rlProps = qgis::down_cast<QgsRasterLayerElevationProperties *>( layer->elevationProperties() );
              switch ( rlProps->profileSymbology() )
              {
                case Qgis::ProfileSurfaceSymbology::Line:
                  if ( QgsLineSymbol *lineSymbol = rlProps->profileLineSymbol() )
                  {
                    symbol.reset( lineSymbol->clone() );
                  }
                  break;
                case Qgis::ProfileSurfaceSymbology::FillBelow:
                case Qgis::ProfileSurfaceSymbology::FillAbove:
                  if ( QgsFillSymbol *fillSymbol = rlProps->profileFillSymbol() )
                  {
                    symbol.reset( fillSymbol->clone() );
                  }
                  break;
              }
              break;
            }

            case Qgis::LayerType::Mesh:
            {
              QgsMeshLayerElevationProperties *mlProps = qgis::down_cast<QgsMeshLayerElevationProperties *>( layer->elevationProperties() );
              switch ( mlProps->profileSymbology() )
              {
                case Qgis::ProfileSurfaceSymbology::Line:
                  if ( QgsLineSymbol *lineSymbol = mlProps->profileLineSymbol() )
                  {
                    symbol.reset( lineSymbol->clone() );
                  }
                  break;
                case Qgis::ProfileSurfaceSymbology::FillBelow:
                case Qgis::ProfileSurfaceSymbology::FillAbove:
                  if ( QgsFillSymbol *fillSymbol = mlProps->profileFillSymbol() )
                  {
                    symbol.reset( fillSymbol->clone() );
                  }
                  break;
              }
              break;
            }

            case Qgis::LayerType::Plugin:
            case Qgis::LayerType::VectorTile:
            case Qgis::LayerType::Annotation:
            case Qgis::LayerType::PointCloud:
            case Qgis::LayerType::Group:
            case Qgis::LayerType::TiledScene:
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
          QString title = !layer->metadata().title().isEmpty() ? layer->metadata().title() : !layer->serverProperties()->title().isEmpty()     ? layer->serverProperties()->title()
                                                                                           : !layer->serverProperties()->shortName().isEmpty() ? layer->serverProperties()->shortName()
                                                                                                                                               : layer->name();

          title = "<b>" + title.toHtmlEscaped() + "</b>";

          QStringList parts;
          parts << title;

          const QString elevationPropertiesSummary = layer->elevationProperties() ? layer->elevationProperties()->htmlSummary() : QString();
          if ( !elevationPropertiesSummary.isEmpty() )
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

bool QgsElevationProfileLayerTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( QgsLayerTreeLayer *layerNode = qobject_cast<QgsLayerTreeLayer *>( index2node( index ) ) )
  {
    if ( role == Qt::CheckStateRole )
    {
      const bool checked = static_cast<Qt::CheckState>( value.toInt() ) == Qt::Checked;
      if ( QgsMapLayer *layer = layerNode->layer() )
      {
        layer->setCustomProperty( QStringLiteral( "_include_in_elevation_profiles" ), checked );
      }
    }
  }

  return QgsLayerTreeModel::setData( index, value, role );
}

bool QgsElevationProfileLayerTreeModel::canDropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) const
{
  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) )
    return false;

  // don't accept moves from other layer trees -- only allow internal drag
  if ( action == Qt::MoveAction )
  {
    const QString source = data->data( QStringLiteral( "application/qgis.layertree.source" ) );
    if ( source.isEmpty() || source != QStringLiteral( ":0x%1" ).arg( reinterpret_cast<quintptr>( this ), 2 * QT_POINTER_SIZE, 16, QLatin1Char( '0' ) ) )
    {
      return false;
    }
  }

  return QgsLayerTreeModel::canDropMimeData( data, action, row, column, parent );
}

bool QgsElevationProfileLayerTreeModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) )
    return false;

  // don't accept moves from other layer trees -- only allow internal drag
  const QString source = data->data( QStringLiteral( "application/qgis.layertree.source" ) );
  if ( source.isEmpty() || source != QStringLiteral( ":0x%1" ).arg( reinterpret_cast<quintptr>( this ), 2 * QT_POINTER_SIZE, 16, QLatin1Char( '0' ) ) )
  {
    if ( action == Qt::CopyAction )
    {
      QByteArray encodedLayerTreeData = data->data( QStringLiteral( "application/qgis.layertreemodeldata" ) );

      QDomDocument layerTreeDoc;
      if ( !layerTreeDoc.setContent( QString::fromUtf8( encodedLayerTreeData ) ) )
        return false;

      QDomElement rootLayerTreeElem = layerTreeDoc.documentElement();
      if ( rootLayerTreeElem.tagName() != QLatin1String( "layer_tree_model_data" ) )
        return false;

      QList<QgsMapLayer *> layersToAdd;

      QDomElement elem = rootLayerTreeElem.firstChildElement();
      while ( !elem.isNull() )
      {
        std::unique_ptr<QgsLayerTreeNode> node( QgsLayerTreeNode::readXml( elem, QgsProject::instance() ) );
        if ( node && QgsLayerTree::isLayer( node.get() ) )
        {
          if ( QgsMapLayer *layer = qobject_cast<QgsLayerTreeLayer *>( node.get() )->layer() )
          {
            layersToAdd << layer;
          }
        }
        elem = elem.nextSiblingElement();
      }

      if ( !layersToAdd.empty() )
        emit addLayers( layersToAdd );

      return true;
    }
    else
    {
      return false;
    }
  }

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
      if ( QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( layerTreeLayer->layer() ) )
      {
        if ( !qgis::down_cast<QgsVectorLayerElevationProperties *>( layer->elevationProperties() )->respectLayerSymbology() )
        {
          return false;
        }
        else if ( dynamic_cast<QgsSingleSymbolRenderer *>( qobject_cast<QgsVectorLayer *>( layer )->renderer() ) )
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


QgsElevationProfileLayerTreeView::QgsElevationProfileLayerTreeView( QgsLayerTree *rootNode, QWidget *parent )
  : QTreeView( parent )
  , mLayerTree( rootNode )
{
  mModel = new QgsElevationProfileLayerTreeModel( rootNode, this );
  connect( mModel, &QgsElevationProfileLayerTreeModel::addLayers, this, &QgsElevationProfileLayerTreeView::addLayers );
  mProxyModel = new QgsElevationProfileLayerTreeProxyModel( mModel, this );

  setHeaderHidden( true );

  setDragEnabled( true );
  setAcceptDrops( true );
  setDropIndicatorShown( true );
  setExpandsOnDoubleClick( false );

  // Ensure legend graphics are scrollable
  header()->setStretchLastSection( false );
  header()->setSectionResizeMode( QHeaderView::ResizeToContents );

  // If vertically scrolling by item, legend graphics can get clipped
  setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

  setDefaultDropAction( Qt::MoveAction );

  setModel( mProxyModel );
}

QgsMapLayer *QgsElevationProfileLayerTreeView::indexToLayer( const QModelIndex &index )
{
  if ( QgsLayerTreeNode *node = mModel->index2node( mProxyModel->mapToSource( index ) ) )
  {
    if ( QgsLayerTreeLayer *layerTreeLayerNode = mLayerTree->toLayer( node ) )
    {
      return layerTreeLayerNode->layer();
    }
  }
  return nullptr;
}

void QgsElevationProfileLayerTreeView::populateInitialLayers( QgsProject *project )
{
  const QList<QgsMapLayer *> layers = project->layers<QgsMapLayer *>().toList();

  // sort layers so that types which are more likely to obscure others are rendered below
  // e.g. vector features should be drawn above raster DEMS, or the DEM line may completely obscure
  // the vector feature
  QList<QgsMapLayer *> sortedLayers = QgsMapLayerUtils::sortLayersByType( layers, { Qgis::LayerType::Raster, Qgis::LayerType::Mesh, Qgis::LayerType::Vector, Qgis::LayerType::PointCloud } );

  std::reverse( sortedLayers.begin(), sortedLayers.end() );
  for ( QgsMapLayer *layer : std::as_const( sortedLayers ) )
  {
    QgsLayerTreeLayer *node = mLayerTree->addLayer( layer );

    if ( layer->customProperty( QStringLiteral( "_include_in_elevation_profiles" ) ).isValid() )
    {
      node->setItemVisibilityChecked( layer->customProperty( QStringLiteral( "_include_in_elevation_profiles" ) ).toBool() );
    }
    else
    {
      node->setItemVisibilityChecked( layer->elevationProperties() && layer->elevationProperties()->showByDefaultInElevationProfilePlots() );
    }
  }
}

QgsElevationProfileLayerTreeProxyModel *QgsElevationProfileLayerTreeView::proxyModel()
{
  return mProxyModel;
}

void QgsElevationProfileLayerTreeView::resizeEvent( QResizeEvent *event )
{
  header()->setMinimumSectionSize( viewport()->width() );
  QTreeView::resizeEvent( event );
}
