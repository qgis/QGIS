/***************************************************************************
  qgslayertreemodel.cpp
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMimeData>
#include <QTextStream>

#include "qgslayertreemodel.h"

#include "qgsapplication.h"
#include "qgslayertree.h"
#include "qgslayertreemodellegendnode.h"
#include "qgsproject.h"
#include "qgsmaphittest.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerlegend.h"
#include "qgsvectorlayer.h"
#include "qgslayerdefinition.h"
#include "qgsiconutils.h"
#include "qgsmimedatautils.h"
#include "qgssettingsregistrycore.h"
#include "qgsmaplayerstyle.h"
#include "qgsrendercontext.h"
#include "qgslayertreefiltersettings.h"

#include <QPalette>

QgsLayerTreeModel::QgsLayerTreeModel( QgsLayerTree *rootNode, QObject *parent )
  : QAbstractItemModel( parent )
  , mRootNode( rootNode )
  , mFlags( ShowLegend | AllowLegendChangeState | DeferredLegendInvalidation )
{
  if ( rootNode )
  {
    connectToRootNode();
  }

  mFontLayer.setBold( true );

  connect( &mDeferLegendInvalidationTimer, &QTimer::timeout, this, &QgsLayerTreeModel::invalidateLegendMapBasedData );
  mDeferLegendInvalidationTimer.setSingleShot( true );
}

QgsLayerTreeModel::~QgsLayerTreeModel()
{
  legendCleanup();
}

QgsLayerTreeNode *QgsLayerTreeModel::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode;

  QObject *obj = reinterpret_cast<QObject *>( index.internalPointer() );
  return qobject_cast<QgsLayerTreeNode *>( obj );
}


int QgsLayerTreeModel::rowCount( const QModelIndex &parent ) const
{
  if ( QgsLayerTreeModelLegendNode *nodeLegend = index2legendNode( parent ) )
    return legendNodeRowCount( nodeLegend );

  QgsLayerTreeNode *n = index2node( parent );
  if ( !n )
    return 0;

  if ( QgsLayerTree::isLayer( n ) )
  {
    if ( !testFlag( ShowLegend ) )
      return 0;

    return legendRootRowCount( QgsLayerTree::toLayer( n ) );
  }

  return n->children().count();
}

int QgsLayerTreeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QModelIndex QgsLayerTreeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount( parent ) ||
       row < 0 || row >= rowCount( parent ) )
    return QModelIndex();

  if ( QgsLayerTreeModelLegendNode *nodeLegend = index2legendNode( parent ) )
    return legendNodeIndex( row, column, nodeLegend );

  QgsLayerTreeNode *n = index2node( parent );
  if ( !n )
    return QModelIndex(); // have no children

  if ( testFlag( ShowLegend ) && QgsLayerTree::isLayer( n ) )
  {
    return legendRootIndex( row, column, QgsLayerTree::toLayer( n ) );
  }

  return createIndex( row, column, static_cast<QObject *>( n->children().at( row ) ) );
}


QModelIndex QgsLayerTreeModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsLayerTreeNode *n = index2node( child ) )
  {
    return indexOfParentLayerTreeNode( n->parent() ); // must not be null
  }
  else if ( QgsLayerTreeModelLegendNode *legendNode = index2legendNode( child ) )
  {
    return legendParent( legendNode );
  }
  else
  {
    Q_ASSERT( false ); // no other node types!
    return QModelIndex();
  }

}


QModelIndex QgsLayerTreeModel::indexOfParentLayerTreeNode( QgsLayerTreeNode *parentNode ) const
{
  Q_ASSERT( parentNode );

  QgsLayerTreeNode *grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex();  // root node -> invalid index

  int row = grandParentNode->children().indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, static_cast<QObject *>( parentNode ) );
}


QVariant QgsLayerTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.column() > 1 )
    return QVariant();

  if ( QgsLayerTreeModelLegendNode *sym = index2legendNode( index ) )
    return legendNodeData( sym, role );

  QgsLayerTreeNode *node = index2node( index );
  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    if ( QgsLayerTree::isGroup( node ) )
      return QgsLayerTree::toGroup( node )->name();

    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
      QString name = nodeLayer->name();
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( nodeLayer->layer() );
      if ( vlayer && nodeLayer->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toInt() && role == Qt::DisplayRole )
      {
        const bool estimatedCount = vlayer->dataProvider() ? QgsDataSourceUri( vlayer->dataProvider()->dataSourceUri() ).useEstimatedMetadata() : false;
        const qlonglong count = vlayer->featureCount();

        // if you modify this line, please update QgsSymbolLegendNode::updateLabel
        name += QStringLiteral( " [%1%2]" ).arg(
                  estimatedCount ? QStringLiteral( "≈" ) : QString(),
                  count != -1 ? QLocale().toString( count ) : tr( "N/A" ) );
      }
      return name;
    }
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 )
  {
    if ( QgsLayerTree::isGroup( node ) )
      return iconGroup();

    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );

      QgsMapLayer *layer = nodeLayer->layer();
      if ( !layer )
        return QVariant();

      // icons possibly overriding default icon
      QIcon icon = QgsIconUtils::iconForLayer( layer );

      // if there's just on legend entry that should be embedded in layer - do that!
      if ( testFlag( ShowLegend ) && legendEmbeddedInParent( nodeLayer ) )
      {
        icon = legendIconEmbeddedInParent( nodeLayer );
      }

      if ( !icon.isNull() && layer->isEditable() && !( layer->properties() & Qgis::MapLayerProperty::UsersCannotToggleEditing ) && testFlag( UseTextFormatting ) )
      {
        const int iconSize = scaleIconSize( 16 );
        QPixmap pixmap( icon.pixmap( iconSize, iconSize ) );

        QPainter painter( &pixmap );
        painter.drawPixmap( 0, 0, iconSize, iconSize, QgsApplication::getThemePixmap( layer->isModified() ? QStringLiteral( "/mIconEditableEdits.svg" ) : QStringLiteral( "/mActionToggleEditing.svg" ) ) );
        painter.end();

        icon = QIcon( pixmap );
      }

      return icon;
    }
  }
  else if ( role == Qt::CheckStateRole )
  {
    if ( !testFlag( AllowNodeChangeVisibility ) )
      return QVariant();

    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );

      if ( nodeLayer->layer() && !nodeLayer->layer()->isSpatial() )
        return QVariant(); // do not show checkbox for non-spatial tables

      return nodeLayer->itemVisibilityChecked() ? Qt::Checked : Qt::Unchecked;
    }
    else if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup *nodeGroup = QgsLayerTree::toGroup( node );
      return nodeGroup->itemVisibilityChecked() ? Qt::Checked : Qt::Unchecked;
    }
  }
  else if ( role == Qt::FontRole && testFlag( UseTextFormatting ) )
  {
    QFont f( QgsLayerTree::isLayer( node ) ? mFontLayer : ( QgsLayerTree::isGroup( node ) ? mFontGroup : QFont() ) );
    if ( index == mCurrentIndex )
      f.setUnderline( true );
    if ( QgsLayerTree::isLayer( node ) )
    {
      const QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
      if ( ( !node->isVisible() && ( !layer || layer->isSpatial() ) ) || ( layer && !layer->isInScaleRange( mLegendMapViewScale ) ) )
      {
        f.setItalic( !f.italic() );
      }
    }
    return f;
  }
  else if ( role == Qt::ForegroundRole && testFlag( UseTextFormatting ) )
  {
    QBrush brush( qApp->palette().color( QPalette::Text ), Qt::SolidPattern );
    if ( QgsLayerTree::isLayer( node ) )
    {
      const QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
      if ( ( !node->isVisible() && ( !layer || layer->isSpatial() ) ) || ( layer && !layer->isInScaleRange( mLegendMapViewScale ) ) )
      {
        QColor fadedTextColor = brush.color();
        fadedTextColor.setAlpha( 128 );
        brush.setColor( fadedTextColor );
      }
    }
    return brush;
  }
  else if ( role == Qt::ToolTipRole )
  {
    if ( QgsLayerTree::isLayer( node ) )
    {
      if ( QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer() )
      {
        QString title =
          !layer->title().isEmpty() ? layer->title() :
          !layer->shortName().isEmpty() ? layer->shortName() :
          layer->name();

        title = "<b>" + title.toHtmlEscaped() + "</b>";

        if ( layer->isSpatial() && layer->crs().isValid() )
        {
          if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
            title += tr( " (%1 - %2)" ).arg( QgsWkbTypes::displayString( vl->wkbType() ), layer->crs().authid() ).toHtmlEscaped();
          else
            title += tr( " (%1)" ).arg( layer->crs().authid() ).toHtmlEscaped();
        }

        QStringList parts;
        parts << title;

        if ( !layer->abstract().isEmpty() )
        {
          parts << QString();
          const QStringList abstractLines = layer->abstract().split( '\n' );
          for ( const auto &l : abstractLines )
          {
            parts << l.toHtmlEscaped();
          }
          parts << QString();
        }

        QString source( layer->publicSource() );
        if ( source.size() > 1024 )
        {
          source = source.left( 1023 ) + QString( QChar( 0x2026 ) );
        }

        parts << "<i>" + source.toHtmlEscaped() + "</i>";

        QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
        const bool showFeatureCount = nodeLayer->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toBool();
        const bool estimatedCount = layer->dataProvider() ? QgsDataSourceUri( layer->dataProvider()->dataSourceUri() ).useEstimatedMetadata() : false;
        if ( showFeatureCount && estimatedCount )
        {
          parts << tr( "<b>Feature count is estimated</b> : the feature count is determined by the database statistics" );
        }

        return parts.join( QLatin1String( "<br/>" ) );
      }
    }
  }

  return QVariant();
}


Qt::ItemFlags QgsLayerTreeModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    Qt::ItemFlags rootFlags = Qt::ItemFlags();
    if ( testFlag( AllowNodeReorder ) )
      rootFlags |= Qt::ItemIsDropEnabled;
    return rootFlags;
  }

  if ( QgsLayerTreeModelLegendNode *symn = index2legendNode( index ) )
    return legendNodeFlags( symn );

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if ( testFlag( AllowNodeRename ) )
    f |= Qt::ItemIsEditable;

  QgsLayerTreeNode *node = index2node( index );
  bool isEmbedded = node->customProperty( QStringLiteral( "embedded" ) ).toInt();

  if ( testFlag( AllowNodeReorder ) )
  {
    // only root embedded nodes can be reordered
    if ( !isEmbedded || ( isEmbedded && node->parent() && !node->parent()->customProperty( QStringLiteral( "embedded" ) ).toInt() ) )
      f |= Qt::ItemIsDragEnabled;
  }

  if ( testFlag( AllowNodeChangeVisibility ) && ( QgsLayerTree::isLayer( node ) || QgsLayerTree::isGroup( node ) ) )
    f |= Qt::ItemIsUserCheckable;

  if ( testFlag( AllowNodeReorder ) && QgsLayerTree::isGroup( node ) && !isEmbedded )
    f |= Qt::ItemIsDropEnabled;

  return f;
}

bool QgsLayerTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  QgsLayerTreeModelLegendNode *sym = index2legendNode( index );
  if ( sym )
  {
    if ( role == Qt::CheckStateRole && !testFlag( AllowLegendChangeState ) )
      return false;
    bool res = sym->setData( value, role );
    if ( res )
      emit dataChanged( index, index );
    return res;
  }

  QgsLayerTreeNode *node = index2node( index );
  if ( !node )
    return QAbstractItemModel::setData( index, value, role );

  if ( role == Qt::CheckStateRole )
  {
    if ( !testFlag( AllowNodeChangeVisibility ) )
      return false;

    bool checked = static_cast< Qt::CheckState >( value.toInt() ) == Qt::Checked;
    if ( checked &&  node->children().isEmpty() )
    {
      node->setItemVisibilityCheckedParentRecursive( checked );
    }
    else if ( testFlag( ActionHierarchical ) )
    {
      if ( node->children().isEmpty() )
        node->setItemVisibilityCheckedParentRecursive( checked );
      else
        node->setItemVisibilityCheckedRecursive( checked );
    }
    else
    {
      node->setItemVisibilityChecked( checked );
    }

    recursivelyEmitDataChanged( index );

    return true;
  }
  else if ( role == Qt::EditRole )
  {
    if ( !testFlag( AllowNodeRename ) )
      return false;

    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *layer = QgsLayerTree::toLayer( node );
      layer->setName( value.toString() );
      emit dataChanged( index, index );
    }
    else if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTree::toGroup( node )->setName( value.toString() );
      emit dataChanged( index, index );
    }
  }

  return QAbstractItemModel::setData( index, value, role );
}

QModelIndex QgsLayerTreeModel::node2index( QgsLayerTreeNode *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->children().indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}


static bool _isChildOfNode( QgsLayerTreeNode *child, QgsLayerTreeNode *node )
{
  if ( !child->parent() )
    return false;

  if ( child->parent() == node )
    return true;

  return _isChildOfNode( child->parent(), node );
}

static bool _isChildOfNodes( QgsLayerTreeNode *child, const QList<QgsLayerTreeNode *> &nodes )
{
  for ( QgsLayerTreeNode *n : nodes )
  {
    if ( _isChildOfNode( child, n ) )
      return true;
  }

  return false;
}


QList<QgsLayerTreeNode *> QgsLayerTreeModel::indexes2nodes( const QModelIndexList &list, bool skipInternal ) const
{
  QList<QgsLayerTreeNode *> nodes;
  const auto constList = list;
  for ( const QModelIndex &index : constList )
  {
    QgsLayerTreeNode *node = index2node( index );
    if ( !node )
      continue;

    nodes << node;
  }

  if ( !skipInternal )
    return nodes;

  // remove any children of nodes if both parent node and children are selected
  QList<QgsLayerTreeNode *> nodesFinal;
  for ( QgsLayerTreeNode *node : std::as_const( nodes ) )
  {
    if ( !_isChildOfNodes( node, nodes ) )
      nodesFinal << node;
  }

  return nodesFinal;
}

QgsLayerTree *QgsLayerTreeModel::rootGroup() const
{
  return mRootNode;
}

void QgsLayerTreeModel::setRootGroup( QgsLayerTree *newRootGroup )
{
  beginResetModel();

  disconnectFromRootNode();

  Q_ASSERT( mLegend.isEmpty() );

  mRootNode = newRootGroup;

  endResetModel();

  connectToRootNode();
}

void QgsLayerTreeModel::refreshLayerLegend( QgsLayerTreeLayer *nodeLayer )
{
  // update title
  QModelIndex idx = node2index( nodeLayer );
  emit dataChanged( idx, idx );

  // update children
  int oldNodeCount = rowCount( idx );
  if ( oldNodeCount > 0 )
  {
    beginRemoveRows( idx, 0, oldNodeCount - 1 );
    removeLegendFromLayer( nodeLayer );
    endRemoveRows();
  }

  addLegendToLayer( nodeLayer );
  int newNodeCount = rowCount( idx );

  // automatic collapse of legend nodes - useful if a layer has many legend nodes
  if ( mAutoCollapseLegendNodesCount != -1 && oldNodeCount != newNodeCount && newNodeCount >= mAutoCollapseLegendNodesCount )
    nodeLayer->setExpanded( false );
}

QModelIndex QgsLayerTreeModel::currentIndex() const
{
  return mCurrentIndex;
}

void QgsLayerTreeModel::setCurrentIndex( const QModelIndex &currentIndex )
{
  mCurrentIndex = currentIndex;
}


void QgsLayerTreeModel::setLayerTreeNodeFont( int nodeType, const QFont &font )
{
  if ( nodeType == QgsLayerTreeNode::NodeGroup )
  {
    if ( mFontGroup != font )
    {
      mFontGroup = font;
      recursivelyEmitDataChanged();
    }
  }
  else if ( nodeType == QgsLayerTreeNode::NodeLayer )
  {
    if ( mFontLayer != font )
    {
      mFontLayer = font;
      recursivelyEmitDataChanged();
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "invalid node type" ), 4 );
  }
}


QFont QgsLayerTreeModel::layerTreeNodeFont( int nodeType ) const
{
  if ( nodeType == QgsLayerTreeNode::NodeGroup )
    return mFontGroup;
  else if ( nodeType == QgsLayerTreeNode::NodeLayer )
    return mFontLayer;
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "invalid node type" ), 4 );
    return QFont();
  }
}

void QgsLayerTreeModel::setLegendFilterByScale( double scale )
{
  mLegendFilterByScale = scale;

  // this could be later done in more efficient way
  // by just updating active legend nodes, without refreshing original legend nodes
  const auto layers = mRootNode->findLayers();
  for ( QgsLayerTreeLayer *nodeLayer : layers )
    refreshLayerLegend( nodeLayer );
}

void QgsLayerTreeModel::setLegendFilterByMap( const QgsMapSettings *settings )
{
  Q_NOWARN_DEPRECATED_PUSH
  setLegendFilter( settings, /* useExtent = */ true );
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayerTreeModel::setLegendFilter( const QgsMapSettings *settings, bool useExtent, const QgsGeometry &polygon, bool useExpressions )
{
  if ( settings && settings->hasValidSettings() )
  {
    std::unique_ptr< QgsLayerTreeFilterSettings > filterSettings = std::make_unique< QgsLayerTreeFilterSettings >( *settings );

    if ( !useExtent ) // only expressions
    {
      filterSettings->setFlags( Qgis::LayerTreeFilterFlag::SkipVisibilityCheck );
    }
    else if ( polygon.type() == Qgis::GeometryType::Polygon )
    {
      filterSettings->setFilterPolygon( polygon );
    }

    if ( useExpressions )
    {
      filterSettings->setLayerFilterExpressionsFromLayerTree( mRootNode );
    }

    setFilterSettings( filterSettings.get() );
  }
  else
  {
    setFilterSettings( nullptr );
  }
}

const QgsMapSettings *QgsLayerTreeModel::legendFilterMapSettings() const
{
  return mFilterSettings ? &mFilterSettings->mapSettings() : nullptr;
}

void QgsLayerTreeModel::setFilterSettings( const QgsLayerTreeFilterSettings *settings )
{
  if ( settings )
  {
    mFilterSettings = std::make_unique< QgsLayerTreeFilterSettings >( *settings );
    mFilterSettings->mapSettings().setLayerStyleOverrides( mLayerStyleOverrides );

    bool hitTestWasRunning = false;
    if ( mHitTestTask )
    {
      // cancel outdated task -- this is owned by the task manager and will get automatically deleted accordingly
      disconnect( mHitTestTask, &QgsTask::taskCompleted, this, &QgsLayerTreeModel::hitTestTaskCompleted );
      mHitTestTask->cancel();
      mHitTestTask = nullptr;
      hitTestWasRunning = true;
    }

    std::unique_ptr< QgsMapHitTest > blockingHitTest;
    if ( mFlags & QgsLayerTreeModel::Flag::UseThreadedHitTest )
      mHitTestTask = new QgsMapHitTestTask( *mFilterSettings );
    else
      blockingHitTest = std::make_unique< QgsMapHitTest >( *mFilterSettings );

    if ( mHitTestTask )
    {
      connect( mHitTestTask, &QgsTask::taskCompleted, this, &QgsLayerTreeModel::hitTestTaskCompleted );
      QgsApplication::taskManager()->addTask( mHitTestTask );

      if ( !hitTestWasRunning )
        emit hitTestStarted();
    }
    else
    {
      blockingHitTest->run();
      mHitTestResults = blockingHitTest->results();
      handleHitTestResults();
    }
  }
  else
  {
    if ( !mFilterSettings )
      return;

    mFilterSettings.reset();
    handleHitTestResults();
  }
}

const QgsLayerTreeFilterSettings *QgsLayerTreeModel::filterSettings() const
{
  return mFilterSettings.get();
}

void QgsLayerTreeModel::handleHitTestResults()
{
  // temporarily disable autocollapse so that legend nodes stay visible
  int bkAutoCollapse = autoCollapseLegendNodes();
  setAutoCollapseLegendNodes( -1 );

  // this could be later done in more efficient way
  // by just updating active legend nodes, without refreshing original legend nodes
  const auto layers = mRootNode->findLayers();
  for ( QgsLayerTreeLayer *nodeLayer : layers )
    refreshLayerLegend( nodeLayer );

  setAutoCollapseLegendNodes( bkAutoCollapse );
}

void QgsLayerTreeModel::setLegendMapViewData( double mapUnitsPerPixel, int dpi, double scale )
{
  if ( mLegendMapViewDpi == dpi && qgsDoubleNear( mLegendMapViewMupp, mapUnitsPerPixel ) && qgsDoubleNear( mLegendMapViewScale, scale ) )
    return;

  double previousScale = mLegendMapViewScale;
  mLegendMapViewScale = scale;
  mLegendMapViewMupp = mapUnitsPerPixel;
  mLegendMapViewDpi = dpi;

  // now invalidate legend nodes!
  legendInvalidateMapBasedData();

  if ( scale != previousScale )
    refreshScaleBasedLayers( QModelIndex(), previousScale );
}

void QgsLayerTreeModel::legendMapViewData( double *mapUnitsPerPixel, int *dpi, double *scale ) const
{
  if ( mapUnitsPerPixel ) *mapUnitsPerPixel = mLegendMapViewMupp;
  if ( dpi ) *dpi = mLegendMapViewDpi;
  if ( scale ) *scale = mLegendMapViewScale;
}

QMap<QString, QString> QgsLayerTreeModel::layerStyleOverrides() const
{
  return mLayerStyleOverrides;
}

void QgsLayerTreeModel::setLayerStyleOverrides( const QMap<QString, QString> &overrides )
{
  mLayerStyleOverrides = overrides;
}

void QgsLayerTreeModel::addTargetScreenProperties( const QgsScreenProperties &properties )
{
  if ( mTargetScreenProperties.contains( properties ) )
    return;

  mTargetScreenProperties.insert( properties );
}

QSet<QgsScreenProperties> QgsLayerTreeModel::targetScreenProperties() const
{
  return mTargetScreenProperties;
}

int QgsLayerTreeModel::scaleIconSize( int standardSize )
{
  return QgsApplication::scaleIconSize( standardSize, true );
}

void QgsLayerTreeModel::waitForHitTestBlocking()
{
  if ( mHitTestTask )
    mHitTestTask->waitForFinished();
}

bool QgsLayerTreeModel::hitTestInProgress() const
{
  return static_cast< bool >( mHitTestTask );
}

void QgsLayerTreeModel::nodeWillAddChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  beginInsertRows( node2index( node ), indexFrom, indexTo );
}

static QList<QgsLayerTreeLayer *> _layerNodesInSubtree( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  QList<QgsLayerTreeNode *> children = node->children();
  QList<QgsLayerTreeLayer *> newLayerNodes;
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode *child = children.at( i );
    if ( QgsLayerTree::isLayer( child ) )
      newLayerNodes << QgsLayerTree::toLayer( child );
    else if ( QgsLayerTree::isGroup( child ) )
      newLayerNodes << QgsLayerTree::toGroup( child )->findLayers();
  }
  return newLayerNodes;
}

void QgsLayerTreeModel::nodeAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  Q_ASSERT( node );

  endInsertRows();

  const auto subNodes = _layerNodesInSubtree( node, indexFrom, indexTo );
  for ( QgsLayerTreeLayer *newLayerNode : subNodes )
    connectToLayer( newLayerNode );
}

void QgsLayerTreeModel::nodeWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  Q_ASSERT( node );

  beginRemoveRows( node2index( node ), indexFrom, indexTo );

  // disconnect from layers and remove their legend
  const auto subNodes = _layerNodesInSubtree( node, indexFrom, indexTo );
  for ( QgsLayerTreeLayer *nodeLayer : subNodes )
    disconnectFromLayer( nodeLayer );
}

void QgsLayerTreeModel::nodeRemovedChildren()
{
  endRemoveRows();
}

void QgsLayerTreeModel::nodeVisibilityChanged( QgsLayerTreeNode *node )
{
  Q_ASSERT( node );

  const QModelIndex index = node2index( node );
  emit dataChanged( index, index );
}

void QgsLayerTreeModel::nodeNameChanged( QgsLayerTreeNode *node, const QString &name )
{
  Q_UNUSED( name )
  Q_ASSERT( node );

  const QModelIndex index = node2index( node );
  emit dataChanged( index, index );
}


void QgsLayerTreeModel::nodeCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key )
{
  if ( QgsLayerTree::isLayer( node ) && key == QLatin1String( "showFeatureCount" ) )
    refreshLayerLegend( QgsLayerTree::toLayer( node ) );
}


void QgsLayerTreeModel::nodeLayerLoaded()
{
  QgsLayerTreeLayer *nodeLayer = qobject_cast<QgsLayerTreeLayer *>( sender() );
  if ( !nodeLayer )
    return;

  // deferred connection to the layer
  connectToLayer( nodeLayer );
}

void QgsLayerTreeModel::nodeLayerWillBeUnloaded()
{
  QgsLayerTreeLayer *nodeLayer = qobject_cast<QgsLayerTreeLayer *>( sender() );
  if ( !nodeLayer )
    return;

  disconnectFromLayer( nodeLayer );

  // wait for the layer to appear again
  connect( nodeLayer, &QgsLayerTreeLayer::layerLoaded, this, &QgsLayerTreeModel::nodeLayerLoaded );
}

void QgsLayerTreeModel::layerLegendChanged()
{
  if ( !mRootNode )
    return;

  if ( !testFlag( ShowLegend ) )
    return;

  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  if ( !layer )
    return;

  QgsLayerTreeLayer *nodeLayer = mRootNode->findLayer( layer->id() );
  if ( !nodeLayer )
    return;

  refreshLayerLegend( nodeLayer );
}

void QgsLayerTreeModel::layerFlagsChanged()
{
  if ( !mRootNode )
    return;

  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  if ( !layer )
    return;

  QgsLayerTreeLayer *nodeLayer = mRootNode->findLayer( layer->id() );
  if ( !nodeLayer )
    return;

  const QModelIndex index = node2index( nodeLayer );
  emit dataChanged( index, index );
}

void QgsLayerTreeModel::layerNeedsUpdate()
{
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  if ( !layer )
    return;

  QgsLayerTreeLayer *nodeLayer = mRootNode->findLayer( layer->id() );
  if ( !nodeLayer )
    return;

  QModelIndex index = node2index( nodeLayer );
  emit dataChanged( index, index );

  if ( nodeLayer->customProperty( QStringLiteral( "showFeatureCount" ) ).toInt() )
    refreshLayerLegend( nodeLayer );
}


void QgsLayerTreeModel::legendNodeDataChanged()
{
  QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( sender() );
  if ( !legendNode )
    return;

  QModelIndex index = legendNode2index( legendNode );
  if ( index.isValid() )
    emit dataChanged( index, index );
}

void QgsLayerTreeModel::legendNodeSizeChanged()
{
  QgsLayerTreeModelLegendNode *legendNode = qobject_cast<QgsLayerTreeModelLegendNode *>( sender() );
  if ( !legendNode )
    return;

  QModelIndex index = legendNode2index( legendNode );
  if ( index.isValid() )
    emit dataChanged( index, index, QVector<int> { Qt::SizeHintRole } );
}

void QgsLayerTreeModel::hitTestTaskCompleted()
{
  if ( mHitTestTask )
  {
    mHitTestResults = mHitTestTask->results();
    handleHitTestResults();
    emit hitTestCompleted();
  }
}

void QgsLayerTreeModel::connectToLayer( QgsLayerTreeLayer *nodeLayer )
{
  if ( !nodeLayer->layer() )
  {
    // in order to connect to layer, we need to have it loaded.
    // keep an eye on the layer ID: once loaded, we will use it
    connect( nodeLayer, &QgsLayerTreeLayer::layerLoaded, this, &QgsLayerTreeModel::nodeLayerLoaded, Qt::UniqueConnection );
    return;
  }

  // watch if the layer is getting removed
  connect( nodeLayer, &QgsLayerTreeLayer::layerWillBeUnloaded, this, &QgsLayerTreeModel::nodeLayerWillBeUnloaded, Qt::UniqueConnection );

  if ( testFlag( ShowLegend ) )
  {
    addLegendToLayer( nodeLayer );

    // if we aren't loading a layer from a project, setup some nice default settings
    if ( !mRootNode->customProperty( QStringLiteral( "loading" ) ).toBool() )
    {
      // automatic collapse of legend nodes - useful if a layer has many legend nodes
      if ( mAutoCollapseLegendNodesCount != -1 && rowCount( node2index( nodeLayer ) )  >= mAutoCollapseLegendNodesCount )
        nodeLayer->setExpanded( false );

      if ( nodeLayer->layer()->type() == Qgis::LayerType::Vector && QgsSettingsRegistryCore::settingsLayerTreeShowFeatureCountForNewLayers->value() )
      {
        nodeLayer->setCustomProperty( QStringLiteral( "showFeatureCount" ), true );
      }
    }
  }

  QgsMapLayer *layer = nodeLayer->layer();
  connect( layer, &QgsMapLayer::legendChanged, this, &QgsLayerTreeModel::layerLegendChanged, Qt::UniqueConnection );
  connect( layer, &QgsMapLayer::flagsChanged, this, &QgsLayerTreeModel::layerFlagsChanged, Qt::UniqueConnection );

  // using unique connection because there may be temporarily more nodes for a layer than just one
  // which would create multiple connections, however disconnect() would disconnect all multiple connections
  // even if we wanted to disconnect just one connection in each call.
  connect( layer, &QgsMapLayer::editingStarted, this, &QgsLayerTreeModel::layerNeedsUpdate, Qt::UniqueConnection );
  connect( layer, &QgsMapLayer::editingStopped, this, &QgsLayerTreeModel::layerNeedsUpdate, Qt::UniqueConnection );
  connect( layer, &QgsMapLayer::layerModified, this, &QgsLayerTreeModel::layerNeedsUpdate, Qt::UniqueConnection );

  emit dataChanged( node2index( nodeLayer ), node2index( nodeLayer ) );
}

// try to find out if the layer ID is present in the tree multiple times
static int _numLayerCount( QgsLayerTreeGroup *group, const QString &layerId )
{
  int count = 0;
  const auto constChildren = group->children();
  for ( QgsLayerTreeNode *child : constChildren )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      if ( QgsLayerTree::toLayer( child )->layerId() == layerId )
        count++;
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      count += _numLayerCount( QgsLayerTree::toGroup( child ), layerId );
    }
  }
  return count;
}

void QgsLayerTreeModel::disconnectFromLayer( QgsLayerTreeLayer *nodeLayer )
{
  disconnect( nodeLayer, nullptr, this, nullptr ); // disconnect from delayed load of layer

  if ( !nodeLayer->layer() )
    return; // we were never connected

  if ( testFlag( ShowLegend ) )
  {
    removeLegendFromLayer( nodeLayer );
  }

  if ( _numLayerCount( mRootNode, nodeLayer->layerId() ) == 1 )
  {
    // last instance of the layer in the tree: disconnect from all signals from layer!
    disconnect( nodeLayer->layer(), nullptr, this, nullptr );
  }
}

void QgsLayerTreeModel::connectToLayers( QgsLayerTreeGroup *parentGroup )
{
  const auto constChildren = parentGroup->children();
  for ( QgsLayerTreeNode *node : constChildren )
  {
    if ( QgsLayerTree::isGroup( node ) )
      connectToLayers( QgsLayerTree::toGroup( node ) );
    else if ( QgsLayerTree::isLayer( node ) )
      connectToLayer( QgsLayerTree::toLayer( node ) );
  }
}

void QgsLayerTreeModel::disconnectFromLayers( QgsLayerTreeGroup *parentGroup )
{
  const auto constChildren = parentGroup->children();
  for ( QgsLayerTreeNode *node : constChildren )
  {
    if ( QgsLayerTree::isGroup( node ) )
      disconnectFromLayers( QgsLayerTree::toGroup( node ) );
    else if ( QgsLayerTree::isLayer( node ) )
      disconnectFromLayer( QgsLayerTree::toLayer( node ) );
  }
}

void QgsLayerTreeModel::connectToRootNode()
{
  Q_ASSERT( mRootNode );

  connect( mRootNode, &QgsLayerTreeNode::willAddChildren, this, &QgsLayerTreeModel::nodeWillAddChildren, Qt::ConnectionType::UniqueConnection );
  connect( mRootNode, &QgsLayerTreeNode::addedChildren, this, &QgsLayerTreeModel::nodeAddedChildren, Qt::ConnectionType::UniqueConnection );
  connect( mRootNode, &QgsLayerTreeNode::willRemoveChildren, this, &QgsLayerTreeModel::nodeWillRemoveChildren, Qt::ConnectionType::UniqueConnection );
  connect( mRootNode, &QgsLayerTreeNode::removedChildren, this, &QgsLayerTreeModel::nodeRemovedChildren, Qt::ConnectionType::UniqueConnection );
  connect( mRootNode, &QgsLayerTreeNode::visibilityChanged, this, &QgsLayerTreeModel::nodeVisibilityChanged, Qt::ConnectionType::UniqueConnection );
  connect( mRootNode, &QgsLayerTreeNode::nameChanged, this, &QgsLayerTreeModel::nodeNameChanged, Qt::ConnectionType::UniqueConnection );
  connect( mRootNode, &QgsLayerTreeNode::customPropertyChanged, this, &QgsLayerTreeModel::nodeCustomPropertyChanged, Qt::ConnectionType::UniqueConnection );

  connectToLayers( mRootNode );
}

void QgsLayerTreeModel::disconnectFromRootNode()
{
  if ( mRootNode )
  {
    disconnect( mRootNode, nullptr, this, nullptr );
    disconnectFromLayers( mRootNode );
  }
}

void QgsLayerTreeModel::recursivelyEmitDataChanged( const QModelIndex &idx )
{
  QgsLayerTreeNode *node = index2node( idx );
  if ( !node )
    return;

  int count = node->children().count();
  if ( count == 0 )
    return;
  emit dataChanged( index( 0, 0, idx ), index( count - 1, 0, idx ) );
  for ( int i = 0; i < count; ++i )
    recursivelyEmitDataChanged( index( i, 0, idx ) );
}

void QgsLayerTreeModel::refreshScaleBasedLayers( const QModelIndex &idx, double previousScale )
{
  QgsLayerTreeNode *node = index2node( idx );
  if ( !node )
    return;

  if ( node->nodeType() == QgsLayerTreeNode::NodeLayer )
  {
    const QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
    if ( layer && layer->hasScaleBasedVisibility() )
    {
      if ( layer->isInScaleRange( mLegendMapViewScale ) != layer->isInScaleRange( previousScale ) )
        emit dataChanged( idx, idx, QVector<int>() << Qt::FontRole << Qt::ForegroundRole );
    }
  }
  int count = node->children().count();
  for ( int i = 0; i < count; ++i )
    refreshScaleBasedLayers( index( i, 0, idx ), previousScale );
}

Qt::DropActions QgsLayerTreeModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

QStringList QgsLayerTreeModel::mimeTypes() const
{
  QStringList types;
  types << QStringLiteral( "application/qgis.layertreemodeldata" );
  return types;
}


QMimeData *QgsLayerTreeModel::mimeData( const QModelIndexList &indexes ) const
{
  // Sort the indexes. Depending on how the user selected the items, the indexes may be unsorted.
  QModelIndexList sortedIndexes = indexes;
  std::sort( sortedIndexes.begin(), sortedIndexes.end(), std::less<QModelIndex>() );

  QList<QgsLayerTreeNode *> nodesFinal = indexes2nodes( sortedIndexes, true );

  if ( nodesFinal.isEmpty() )
    return nullptr;

  QMimeData *mimeData = new QMimeData();

  QDomDocument layerTreeDoc;
  QDomElement rootLayerTreeElem = layerTreeDoc.createElement( QStringLiteral( "layer_tree_model_data" ) );

  for ( QgsLayerTreeNode *node : std::as_const( nodesFinal ) )
  {
    node->writeXml( rootLayerTreeElem, QgsReadWriteContext() );
  }
  layerTreeDoc.appendChild( rootLayerTreeElem );

  QString errorMessage;
  QgsReadWriteContext readWriteContext;
  QDomDocument layerDefinitionsDoc( QStringLiteral( "qgis-layer-definition" ) );
  QgsLayerDefinition::exportLayerDefinition( layerDefinitionsDoc, nodesFinal, errorMessage, QgsReadWriteContext() );

  QString txt = layerDefinitionsDoc.toString();

  mimeData->setData( QStringLiteral( "application/qgis.layertreemodeldata" ), layerTreeDoc.toString().toUtf8() );
  mimeData->setData( QStringLiteral( "application/qgis.application.pid" ), QString::number( QCoreApplication::applicationPid() ).toUtf8() );
  mimeData->setData( QStringLiteral( "application/qgis.layertree.source" ), QStringLiteral( ":0x%1" ).arg( reinterpret_cast<quintptr>( this ), 2 * QT_POINTER_SIZE, 16, QLatin1Char( '0' ) ).toUtf8() );
  mimeData->setData( QStringLiteral( "application/qgis.layertree.layerdefinitions" ), txt.toUtf8() );
  mimeData->setData( QStringLiteral( "application/x-vnd.qgis.qgis.uri" ), QgsMimeDataUtils::layerTreeNodesToUriList( nodesFinal ) );

  return mimeData;
}

bool QgsLayerTreeModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) )
    return false;

  if ( column >= columnCount( parent ) )
    return false;

  // don't accept drops from some layer tree subclasses to non-matching subclasses
  const QString restrictTypes( data->data( QStringLiteral( "application/qgis.restrictlayertreemodelsubclass" ) ) );
  if ( !restrictTypes.isEmpty() && restrictTypes != QString( metaObject()->className() ) )
    return false;

  QgsLayerTreeNode *nodeParent = index2node( parent );
  if ( !QgsLayerTree::isGroup( nodeParent ) )
    return false;

  if ( parent.isValid() && row == -1 )
    row = 0; // if dropped directly onto group item, insert at first position

  // if we are coming from another QGIS instance, we need to add the layers too
  bool ok = false;
  // the application pid is only provided from QGIS 3.14, so do not check to OK before defaulting to moving in the legend
  qint64 qgisPid = data->data( QStringLiteral( "application/qgis.application.pid" ) ).toInt( &ok );

  if ( ok && qgisPid != QCoreApplication::applicationPid() )
  {
    QByteArray encodedLayerDefinitionData = data->data( QStringLiteral( "application/qgis.layertree.layerdefinitions" ) );
    QDomDocument layerDefinitionDoc;
    if ( !layerDefinitionDoc.setContent( QString::fromUtf8( encodedLayerDefinitionData ) ) )
      return false;
    QgsReadWriteContext context;
    QString errorMessage;
    QgsLayerDefinition::loadLayerDefinition( layerDefinitionDoc, QgsProject::instance(), QgsLayerTree::toGroup( nodeParent ), errorMessage, context );
    emit messageEmitted( tr( "New layers added from another QGIS instance" ) );
  }
  else
  {
    QByteArray encodedLayerTreeData = data->data( QStringLiteral( "application/qgis.layertreemodeldata" ) );

    QDomDocument layerTreeDoc;
    if ( !layerTreeDoc.setContent( QString::fromUtf8( encodedLayerTreeData ) ) )
      return false;

    QDomElement rootLayerTreeElem = layerTreeDoc.documentElement();
    if ( rootLayerTreeElem.tagName() != QLatin1String( "layer_tree_model_data" ) )
      return false;

    QList<QgsLayerTreeNode *> nodes;

    QDomElement elem = rootLayerTreeElem.firstChildElement();
    while ( !elem.isNull() )
    {
      QgsLayerTreeNode *node = QgsLayerTreeNode::readXml( elem, QgsProject::instance() );
      if ( node )
        nodes << node;

      elem = elem.nextSiblingElement();
    }

    if ( nodes.isEmpty() )
      return false;

    QgsLayerTree::toGroup( nodeParent )->insertChildNodes( row, nodes );
  }
  return true;
}

bool QgsLayerTreeModel::removeRows( int row, int count, const QModelIndex &parent )
{
  QgsLayerTreeNode *parentNode = index2node( parent );
  if ( QgsLayerTree::isGroup( parentNode ) )
  {
    QgsLayerTree::toGroup( parentNode )->removeChildren( row, count );
    return true;
  }
  return false;
}

void QgsLayerTreeModel::setFlags( QgsLayerTreeModel::Flags f )
{
  mFlags = f;
}

void QgsLayerTreeModel::setFlag( QgsLayerTreeModel::Flag f, bool on )
{
  if ( on )
    mFlags |= f;
  else
    mFlags &= ~f;
}

QgsLayerTreeModel::Flags QgsLayerTreeModel::flags() const
{
  return mFlags;
}

bool QgsLayerTreeModel::testFlag( QgsLayerTreeModel::Flag f ) const
{
  return mFlags.testFlag( f );
}

QIcon QgsLayerTreeModel::iconGroup()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFolder.svg" ) );
}

QList<QgsLayerTreeModelLegendNode *> QgsLayerTreeModel::filterLegendNodes( const QList<QgsLayerTreeModelLegendNode *> &nodes )
{
  QList<QgsLayerTreeModelLegendNode *> filtered;

  if ( mLegendFilterByScale > 0 )
  {
    for ( QgsLayerTreeModelLegendNode *node : std::as_const( nodes ) )
    {
      if ( node->isScaleOK( mLegendFilterByScale ) )
        filtered << node;
    }
  }
  else if ( mFilterSettings )
  {
    if ( !nodes.isEmpty() && mFilterSettings->layers().contains( nodes.at( 0 )->layerNode()->layer() ) )
    {
      for ( QgsLayerTreeModelLegendNode *node : std::as_const( nodes ) )
      {
        switch ( node->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::NodeType ) ).value<QgsLayerTreeModelLegendNode::NodeTypes>() )
        {
          case QgsLayerTreeModelLegendNode::EmbeddedWidget:
            filtered << node;
            break;

          case QgsLayerTreeModelLegendNode::SimpleLegend:
          case QgsLayerTreeModelLegendNode::SymbolLegend:
          case QgsLayerTreeModelLegendNode::RasterSymbolLegend:
          case QgsLayerTreeModelLegendNode::ImageLegend:
          case QgsLayerTreeModelLegendNode::WmsLegend:
          case QgsLayerTreeModelLegendNode::DataDefinedSizeLegend:
          case QgsLayerTreeModelLegendNode::ColorRampLegend:
          {
            const QString ruleKey = node->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString();
            const bool checked = ( mFilterSettings && !( mFilterSettings->flags() & Qgis::LayerTreeFilterFlag::SkipVisibilityCheck ) )
                                 || node->data( Qt::CheckStateRole ).toInt() == Qt::Checked;

            if ( checked )
            {
              if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( node->layerNode()->layer() ) )
              {
                auto it = mHitTestResults.constFind( vl->id() );
                if ( it != mHitTestResults.constEnd() && it->contains( ruleKey ) )
                {
                  filtered << node;
                }
              }
              else
              {
                filtered << node;
              }
            }
            else  // unknown node type or unchecked
              filtered << node;
            break;
          }
        }
      }
    }
  }
  else
  {
    return nodes;
  }

  return filtered;
}



///////////////////////////////////////////////////////////////////////////////
// Legend nodes routines - start

void QgsLayerTreeModel::legendCleanup()
{
  const auto constMLegend = mLegend;
  for ( const LayerLegendData &data : constMLegend )
  {
    qDeleteAll( data.originalNodes );
    delete data.tree;
  }
  mLegend.clear();

  if ( mHitTestTask )
  {
    // cancel outdated task -- this is owned by the task manager and will get automatically deleted accordingly
    disconnect( mHitTestTask, &QgsTask::taskCompleted, this, &QgsLayerTreeModel::hitTestTaskCompleted );
    mHitTestTask->cancel();
    mHitTestTask = nullptr;
  }
}


void QgsLayerTreeModel::removeLegendFromLayer( QgsLayerTreeLayer *nodeLayer )
{
  if ( mLegend.contains( nodeLayer ) )
  {
    qDeleteAll( mLegend[nodeLayer].originalNodes );
    delete mLegend[nodeLayer].tree;
    mLegend.remove( nodeLayer );
  }
}


void QgsLayerTreeModel::addLegendToLayer( QgsLayerTreeLayer *nodeL )
{
  if ( !nodeL || !nodeL->layer() )
    return;

  QgsMapLayer *ml = nodeL->layer();

  QgsMapLayerStyleOverride styleOverride( ml );
  if ( mLayerStyleOverrides.contains( ml->id() ) )
    styleOverride.setOverrideStyle( mLayerStyleOverrides.value( ml->id() ) );

  QgsMapLayerLegend *layerLegend = ml->legend();
  if ( !layerLegend )
    return;
  QList<QgsLayerTreeModelLegendNode *> lstNew = layerLegend->createLayerTreeModelLegendNodes( nodeL );

  // apply filtering defined in layer node's custom properties (reordering, filtering, custom labels)
  QgsMapLayerLegendUtils::applyLayerNodeProperties( nodeL, lstNew );

  if ( testFlag( UseEmbeddedWidgets ) )
  {
    // generate placeholder legend nodes that will be replaced by widgets in QgsLayerTreeView
    int widgetsCount = ml->customProperty( QStringLiteral( "embeddedWidgets/count" ), 0 ).toInt();
    while ( widgetsCount > 0 )
    {
      lstNew.insert( 0, new EmbeddedWidgetLegendNode( nodeL ) );
      --widgetsCount;
    }
  }

  QList<QgsLayerTreeModelLegendNode *> filteredLstNew = filterLegendNodes( lstNew );

  const auto constLstNew = lstNew;
  for ( QgsLayerTreeModelLegendNode *n : constLstNew )
  {
    n->setParent( this );
    connect( n, &QgsLayerTreeModelLegendNode::dataChanged, this, &QgsLayerTreeModel::legendNodeDataChanged );
    connect( n, &QgsLayerTreeModelLegendNode::sizeChanged, this, &QgsLayerTreeModel::legendNodeSizeChanged );
  }

  // See if we have an embedded node - if we do, we will not use it among active nodes.
  // Legend node embedded in parent does not have to be the first one,
  // there can be also nodes generated for embedded widgets
  QgsLayerTreeModelLegendNode *embeddedNode = nullptr;
  const auto constFilteredLstNew = filteredLstNew;
  for ( QgsLayerTreeModelLegendNode *legendNode : constFilteredLstNew )
  {
    if ( legendNode->isEmbeddedInParent() )
    {
      embeddedNode = legendNode;
      filteredLstNew.removeOne( legendNode );
      break;
    }
  }

  LayerLegendTree *legendTree = nullptr;

  // maybe the legend nodes form a tree - try to create a tree structure from the list
  if ( testFlag( ShowLegendAsTree ) )
    legendTree = tryBuildLegendTree( filteredLstNew );

  int count = legendTree ? legendTree->children[nullptr].count() : filteredLstNew.count();

  if ( !filteredLstNew.isEmpty() )
  {
    // Make sure it's clear
    const QModelIndex nodeIndex { node2index( nodeL ) };
    if ( rowCount( nodeIndex ) > 0 )
    {
      beginRemoveRows( node2index( nodeL ), 0, rowCount( nodeIndex ) - 1 );
      mLegend[nodeL] = LayerLegendData();
      endRemoveRows();
    }
    beginInsertRows( node2index( nodeL ), 0, count - 1 );
  }

  LayerLegendData data;
  data.originalNodes = lstNew;
  data.activeNodes = filteredLstNew;
  data.embeddedNodeInParent = embeddedNode;
  data.tree = legendTree;

  mLegend[nodeL] = data;

  if ( !filteredLstNew.isEmpty() )
  {
    endInsertRows();
  }

  // invalidate map based data even if the data is not map-based to make sure
  // the symbol sizes are computed at least once
  mInvalidatedNodes.insert( nodeL );
  legendInvalidateMapBasedData();
}


QgsLayerTreeModel::LayerLegendTree *QgsLayerTreeModel::tryBuildLegendTree( const QList<QgsLayerTreeModelLegendNode *> &nodes )
{
  // first check whether there are any legend nodes that are not top-level
  bool hasParentKeys = false;
  for ( QgsLayerTreeModelLegendNode *n : nodes )
  {
    if ( !n->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::ParentRuleKey ) ).toString().isEmpty() )
    {
      hasParentKeys = true;
      break;
    }
  }
  if ( !hasParentKeys )
    return nullptr; // all legend nodes are top-level => stick with list representation

  // make mapping from rules to nodes and do some sanity checks
  QHash<QString, QgsLayerTreeModelLegendNode *> rule2node;
  rule2node[QString()] = nullptr;
  for ( QgsLayerTreeModelLegendNode *n : nodes )
  {
    QString ruleKey = n->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString();
    if ( ruleKey.isEmpty() ) // in tree all nodes must have key
      return nullptr;
    if ( rule2node.contains( ruleKey ) ) // and they must be unique
      return nullptr;
    rule2node[ruleKey] = n;
  }

  // create the tree structure
  LayerLegendTree *tree = new LayerLegendTree;
  for ( QgsLayerTreeModelLegendNode *n : nodes )
  {
    QString parentRuleKey = n->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::ParentRuleKey ) ).toString();
    QgsLayerTreeModelLegendNode *parent = rule2node.value( parentRuleKey, nullptr );
    tree->parents[n] = parent;
    tree->children[parent] << n;
  }
  return tree;
}

QgsRenderContext *QgsLayerTreeModel::createTemporaryRenderContext() const
{
  double scale = 0.0;
  double mupp = 0.0;
  int dpi = 0;
  legendMapViewData( &mupp, &dpi, &scale );
  bool validData = !qgsDoubleNear( mupp, 0.0 ) && dpi != 0 && !qgsDoubleNear( scale, 0.0 );

  // setup temporary render context
  std::unique_ptr<QgsRenderContext> context( new QgsRenderContext );
  context->setScaleFactor( dpi / 25.4 );

  if ( !mTargetScreenProperties.isEmpty() )
  {
    mTargetScreenProperties.begin()->updateRenderContextForScreen( *context );
  }

  context->setRendererScale( scale );
  context->setMapToPixel( QgsMapToPixel( mupp ) );
  context->setFlag( Qgis::RenderContextFlag::RenderSymbolPreview );
  return validData ? context.release() : nullptr;
}


QgsLayerTreeModelLegendNode *QgsLayerTreeModel::index2legendNode( const QModelIndex &index )
{
  return qobject_cast<QgsLayerTreeModelLegendNode *>( reinterpret_cast<QObject *>( index.internalPointer() ) );
}


QModelIndex QgsLayerTreeModel::legendNode2index( QgsLayerTreeModelLegendNode *legendNode )
{
  const LayerLegendData &data = mLegend[legendNode->layerNode()];
  if ( data.tree )
  {
    if ( QgsLayerTreeModelLegendNode *parentLegendNode = data.tree->parents[legendNode] )
    {
      QModelIndex parentIndex = legendNode2index( parentLegendNode );
      int row = data.tree->children[parentLegendNode].indexOf( legendNode );
      return index( row, 0, parentIndex );
    }
    else
    {
      QModelIndex parentIndex = node2index( legendNode->layerNode() );
      int row = data.tree->children[nullptr].indexOf( legendNode );
      return index( row, 0, parentIndex );
    }
  }

  QModelIndex parentIndex = node2index( legendNode->layerNode() );
  Q_ASSERT( parentIndex.isValid() );
  int row = data.activeNodes.indexOf( legendNode );
  if ( row < 0 ) // legend node may be filtered (exists within the list of original nodes, but not in active nodes)
    return QModelIndex();

  return index( row, 0, parentIndex );
}


int QgsLayerTreeModel::legendNodeRowCount( QgsLayerTreeModelLegendNode *node ) const
{
  const LayerLegendData &data = mLegend[node->layerNode()];
  if ( data.tree )
    return data.tree->children[node].count();

  return 0; // they are leaves
}


int QgsLayerTreeModel::legendRootRowCount( QgsLayerTreeLayer *nL ) const
{
  if ( !mLegend.contains( nL ) )
    return 0;

  const LayerLegendData &data = mLegend[nL];
  if ( data.tree )
    return data.tree->children[nullptr].count();

  int count = data.activeNodes.count();
  return count;
}


QModelIndex QgsLayerTreeModel::legendRootIndex( int row, int column, QgsLayerTreeLayer *nL ) const
{
  Q_ASSERT( mLegend.contains( nL ) );
  const LayerLegendData &data = mLegend[nL];
  if ( data.tree )
    return createIndex( row, column, static_cast<QObject *>( data.tree->children[nullptr].at( row ) ) );

  return createIndex( row, column, static_cast<QObject *>( data.activeNodes.at( row ) ) );
}


QModelIndex QgsLayerTreeModel::legendNodeIndex( int row, int column, QgsLayerTreeModelLegendNode *node ) const
{
  const LayerLegendData &data = mLegend[node->layerNode()];
  if ( data.tree )
    return createIndex( row, column, static_cast<QObject *>( data.tree->children[node].at( row ) ) );

  return QModelIndex(); // have no children
}


QModelIndex QgsLayerTreeModel::legendParent( QgsLayerTreeModelLegendNode *legendNode ) const
{
  QgsLayerTreeLayer *layerNode = legendNode->layerNode();
  const LayerLegendData &data = mLegend[layerNode];
  if ( data.tree )
  {
    if ( QgsLayerTreeModelLegendNode *parentNode = data.tree->parents[legendNode] )
    {
      QgsLayerTreeModelLegendNode *grandParentNode = data.tree->parents[parentNode]; // may be null (not a problem)
      int row = data.tree->children[grandParentNode].indexOf( parentNode );
      return createIndex( row, 0, static_cast<QObject *>( parentNode ) );
    }
    else
      return indexOfParentLayerTreeNode( layerNode );
  }

  return indexOfParentLayerTreeNode( layerNode );
}


QVariant QgsLayerTreeModel::legendNodeData( QgsLayerTreeModelLegendNode *node, int role ) const
{
  if ( role == Qt::CheckStateRole && !testFlag( AllowLegendChangeState ) )
    return QVariant();
  return node->data( role );
}


Qt::ItemFlags QgsLayerTreeModel::legendNodeFlags( QgsLayerTreeModelLegendNode *node ) const
{
  Qt::ItemFlags f = node->flags();
  if ( !testFlag( AllowLegendChangeState ) )
    f &= ~Qt::ItemIsUserCheckable;
  return f;
}


bool QgsLayerTreeModel::legendEmbeddedInParent( QgsLayerTreeLayer *nodeLayer ) const
{
  return static_cast< bool >( mLegend[nodeLayer].embeddedNodeInParent );
}

QgsLayerTreeModelLegendNode *QgsLayerTreeModel::legendNodeEmbeddedInParent( QgsLayerTreeLayer *nodeLayer ) const
{
  return mLegend[nodeLayer].embeddedNodeInParent;
}


QIcon QgsLayerTreeModel::legendIconEmbeddedInParent( QgsLayerTreeLayer *nodeLayer ) const
{
  QgsLayerTreeModelLegendNode *legendNode = mLegend[nodeLayer].embeddedNodeInParent;
  if ( !legendNode )
    return QIcon();
  return QIcon( qvariant_cast<QPixmap>( legendNode->data( Qt::DecorationRole ) ) );
}


QList<QgsLayerTreeModelLegendNode *> QgsLayerTreeModel::layerLegendNodes( QgsLayerTreeLayer *nodeLayer, bool skipNodeEmbeddedInParent )
{
  if ( !mLegend.contains( nodeLayer ) )
    return QList<QgsLayerTreeModelLegendNode *>();

  const LayerLegendData &data = mLegend[nodeLayer];
  QList<QgsLayerTreeModelLegendNode *> lst( data.activeNodes );
  if ( !skipNodeEmbeddedInParent && data.embeddedNodeInParent )
    lst.prepend( data.embeddedNodeInParent );
  return lst;
}

QList<QgsLayerTreeModelLegendNode *> QgsLayerTreeModel::layerOriginalLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  return mLegend.value( nodeLayer ).originalNodes;
}

QgsLayerTreeModelLegendNode *QgsLayerTreeModel::findLegendNode( const QString &layerId, const QString &ruleKey ) const
{
  for ( auto it = mLegend.constBegin(); it != mLegend.constEnd(); ++it )
  {
    QgsLayerTreeLayer *layer = it.key();
    if ( layer->layerId() == layerId )
    {
      const auto activeNodes = mLegend.value( layer ).activeNodes;
      for ( QgsLayerTreeModelLegendNode *legendNode : activeNodes )
      {
        if ( legendNode->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString() == ruleKey )
        {
          //found it!
          return legendNode;
        }
      }
    }
  }

  return nullptr;
}

void QgsLayerTreeModel::legendInvalidateMapBasedData()
{
  if ( !testFlag( DeferredLegendInvalidation ) )
    invalidateLegendMapBasedData();
  else
    mDeferLegendInvalidationTimer.start( 10 );
}

void QgsLayerTreeModel::invalidateLegendMapBasedData()
{
  // we have varying icon sizes, and we want icon to be centered and
  // text to be left aligned, so we have to compute the max width of icons
  //
  // we do that for nodes which share a common parent
  //
  // we do that here because for symbols with size defined in map units
  // the symbol sizes changes depends on the zoom level

  std::unique_ptr<QgsRenderContext> context( createTemporaryRenderContext() );

  for ( QgsLayerTreeLayer *layerNode : std::as_const( mInvalidatedNodes ) )
  {
    const LayerLegendData &data = mLegend.value( layerNode );

    QList<QgsSymbolLegendNode *> symbolNodes;
    QMap<QString, int> widthMax;
    for ( QgsLayerTreeModelLegendNode *legendNode : std::as_const( data.originalNodes ) )
    {
      QgsSymbolLegendNode *n = qobject_cast<QgsSymbolLegendNode *>( legendNode );
      if ( n )
      {
        const QSize sz( n->minimumIconSize( context.get() ) );
        const QString parentKey( n->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::ParentRuleKey ) ).toString() );
        widthMax[parentKey] = std::max( sz.width(), widthMax.contains( parentKey ) ? widthMax[parentKey] : 0 );
        n->setIconSize( sz );
        symbolNodes.append( n );
      }
    }
    for ( QgsSymbolLegendNode *n : std::as_const( symbolNodes ) )
    {
      const QString parentKey( n->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::ParentRuleKey ) ).toString() );
      Q_ASSERT( widthMax[parentKey] > 0 );
      const int twiceMarginWidth = 2; // a one pixel margin avoids hugly rendering of icon
      n->setIconSize( QSize( widthMax[parentKey] + twiceMarginWidth, n->iconSize().rheight() + twiceMarginWidth ) );
    }
    for ( QgsLayerTreeModelLegendNode *legendNode : std::as_const( data.originalNodes ) )
      legendNode->invalidateMapBasedData();
  }

  mInvalidatedNodes.clear();
}

// Legend nodes routines - end
///////////////////////////////////////////////////////////////////////////////
