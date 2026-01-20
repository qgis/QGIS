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

#include "qgsapplication.h"
#include "qgsfillsymbol.h"
#include "qgslayertree.h"
#include "qgslayertreenode.h"
#include "qgsmaplayerutils.h"
#include "qgsmarkersymbol.h"
#include "qgsmeshlayerelevationproperties.h"
#include "qgsprofilesourceregistry.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerelevationproperties.h"

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>
#include <QMimeData>

#include "moc_qgselevationprofilelayertreeview.cpp"

const QString QgsElevationProfileLayerTreeView::CUSTOM_NODE_ELEVATION_PROFILE_SOURCE = u"elevationProfileRegistry"_s;

QgsElevationProfileLayerTreeModel::QgsElevationProfileLayerTreeModel( QgsLayerTree *rootNode, QObject *parent )
  : QgsLayerTreeModel( rootNode, parent )
{
  setAllowModifications( true );

  setFlag( QgsLayerTreeModel::AllowLegendChangeState, false );
  setFlag( QgsLayerTreeModel::ShowLegendAsTree );
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
                    if ( !symbol )
                    {
                      // just use default layer icon
                      return QgsLayerTreeModel::data( index, role );
                    }

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

          return parts.join( "<br/>"_L1 );
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
  if ( !mAllowModifications )
    return false;

  if ( QgsLayerTreeLayer *layerNode = qobject_cast<QgsLayerTreeLayer *>( index2node( index ) ) )
  {
    if ( role == Qt::CheckStateRole )
    {
      const bool checked = static_cast<Qt::CheckState>( value.toInt() ) == Qt::Checked;
      if ( QgsMapLayer *layer = layerNode->layer() )
      {
        layer->setCustomProperty( u"_include_in_elevation_profiles"_s, checked );
      }
    }
  }

  return QgsLayerTreeModel::setData( index, value, role );
}

Qt::ItemFlags QgsElevationProfileLayerTreeModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags f = QgsLayerTreeModel::flags( index );

  // the elevation tree model only supports group renames, not layer renames (otherwise
  // we'd be renaming the actual layer, which is likely NOT what users expect)
  QgsLayerTreeNode *node = index2node( index );
  if ( !mAllowModifications || !QgsLayerTree::isGroup( node ) )
  {
    f.setFlag( Qt::ItemFlag::ItemIsEditable, false );
  }
  return f;
}

bool QgsElevationProfileLayerTreeModel::canDropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) const
{
  if ( !mAllowModifications )
    return false;

  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( u"application/qgis.layertreemodeldata"_s ) )
    return false;

  // don't accept moves from other layer trees -- only allow internal drag
  if ( action == Qt::MoveAction )
  {
    const QString source = data->data( u"application/qgis.layertree.source"_s );
    if ( source.isEmpty() || source != u":0x%1"_s.arg( reinterpret_cast<quintptr>( this ), 2 * QT_POINTER_SIZE, 16, '0'_L1 ) )
    {
      return false;
    }
  }

  return QgsLayerTreeModel::canDropMimeData( data, action, row, column, parent );
}

bool QgsElevationProfileLayerTreeModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  if ( !mAllowModifications )
    return false;

  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( u"application/qgis.layertreemodeldata"_s ) )
    return false;

  // don't accept moves from other layer trees -- only allow internal drag
  const QString source = data->data( u"application/qgis.layertree.source"_s );
  if ( source.isEmpty() || source != u":0x%1"_s.arg( reinterpret_cast<quintptr>( this ), 2 * QT_POINTER_SIZE, 16, '0'_L1 ) )
  {
    if ( action == Qt::CopyAction )
    {
      QByteArray encodedLayerTreeData = data->data( u"application/qgis.layertreemodeldata"_s );

      QDomDocument layerTreeDoc;
      if ( !layerTreeDoc.setContent( QString::fromUtf8( encodedLayerTreeData ) ) )
        return false;

      QDomElement rootLayerTreeElem = layerTreeDoc.documentElement();
      if ( rootLayerTreeElem.tagName() != "layer_tree_model_data"_L1 )
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
  if ( !mAllowModifications )
    return nullptr;

  QMimeData *mimeData = QgsLayerTreeModel::mimeData( indexes );
  if ( mimeData )
  {
    mimeData->setData( u"application/qgis.restrictlayertreemodelsubclass"_s, "QgsElevationProfileLayerTreeModel" );
  }
  return mimeData;
}

void QgsElevationProfileLayerTreeModel::setAllowModifications( bool allow )
{
  setFlag( QgsLayerTreeModel::AllowNodeReorder, allow );
  setFlag( QgsLayerTreeModel::AllowNodeChangeVisibility, allow );
  setFlag( QgsLayerTreeModel::AllowNodeRename, allow );
  mAllowModifications = allow;
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
      case QgsLayerTreeNode::NodeCustom:
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
  : QgsLayerTreeViewBase( parent )
{
  setLayerTree( rootNode );
}

void QgsElevationProfileLayerTreeView::setLayerTree( QgsLayerTree *rootNode )
{
  QgsElevationProfileLayerTreeModel *oldModel = mModel;

  mLayerTree = rootNode;

  mModel = new QgsElevationProfileLayerTreeModel( mLayerTree, this );

  connect( mModel, &QgsElevationProfileLayerTreeModel::addLayers, this, &QgsElevationProfileLayerTreeView::addLayers );
  mProxyModel = new QgsElevationProfileLayerTreeProxyModel( mModel, this );

  setModel( mProxyModel );
  setLayerTreeModel( mModel );

  delete oldModel;
}

void QgsElevationProfileLayerTreeView::populateMissingLayers( QgsProject *project )
{
  const QList<QgsMapLayer *> layers = project->layers<QgsMapLayer *>().toList();

  // sort layers so that types which are more likely to obscure others are rendered below
  // e.g. vector features should be drawn above raster DEMS, or the DEM line may completely obscure
  // the vector feature
  QList<QgsMapLayer *> sortedLayers = QgsMapLayerUtils::sortLayersByType( layers, { Qgis::LayerType::Raster, Qgis::LayerType::Mesh, Qgis::LayerType::Vector, Qgis::LayerType::PointCloud } );

  std::reverse( sortedLayers.begin(), sortedLayers.end() );
  for ( QgsMapLayer *layer : std::as_const( sortedLayers ) )
  {
    // don't re-add existing layers
    if ( mLayerTree->findLayer( layer ) )
      continue;

    QgsLayerTreeLayer *node = mLayerTree->addLayer( layer );

    if ( layer->customProperty( u"_include_in_elevation_profiles"_s ).isValid() )
    {
      node->setItemVisibilityChecked( layer->customProperty( u"_include_in_elevation_profiles"_s ).toBool() );
    }
    else
    {
      node->setItemVisibilityChecked( layer->elevationProperties() && layer->elevationProperties()->showByDefaultInElevationProfilePlots() );
    }
  }
}

void QgsElevationProfileLayerTreeView::populateInitialSources( QgsProject *project )
{
  const QList< QgsAbstractProfileSource * > sources = QgsApplication::profileSourceRegistry()->profileSources();
  for ( QgsAbstractProfileSource *source : sources )
  {
    addNodeForRegisteredSource( source->profileSourceId(), source->profileSourceName() );
  }

  populateMissingLayers( project );
}

void QgsElevationProfileLayerTreeView::addNodeForRegisteredSource( const QString &sourceId, const QString &sourceName )
{
  auto customNode = std::make_unique< QgsLayerTreeCustomNode >( sourceId, sourceName.isEmpty() ? sourceId : sourceName );
  customNode->setItemVisibilityChecked( true );
  // Mark the node so that we know which custom nodes correspond to elevation profile sources
  customNode->setCustomProperty( u"source"_s, QgsElevationProfileLayerTreeView::CUSTOM_NODE_ELEVATION_PROFILE_SOURCE );

  QgsLayerTreeCustomNode *node = mLayerTree->insertCustomNode( -1, customNode.release() );
  if ( !node )
    QgsDebugError( QString( "The custom node with id '%1' could not be added!" ).arg( sourceId ) );
}

void QgsElevationProfileLayerTreeView::removeNodeForUnregisteredSource( const QString &sourceId )
{
  mLayerTree->removeCustomNode( sourceId );
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
