/***************************************************************************
  qgslayertreeviewdefaultactions.cpp
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

#include "qgslayertreeviewdefaultactions.h"
#include "qgsguiutils.h"
#include "qgsapplication.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeview.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

#include <QAction>

QgsLayerTreeViewDefaultActions::QgsLayerTreeViewDefaultActions( QgsLayerTreeView *view )
  : QObject( view )
  , mView( view )
{
}

QAction *QgsLayerTreeViewDefaultActions::actionAddGroup( QObject *parent )
{
  QAction *a = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddGroup.svg" ) ), tr( "&Add Group" ), parent );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::addGroup );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionRemoveGroupOrLayer( QObject *parent )
{
  QAction *a = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemoveLayer.svg" ) ), tr( "&Remove" ), parent );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::removeGroupOrLayer );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionShowInOverview( QObject *parent )
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node )
    return nullptr;

  QAction *a = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionInOverview.svg" ) ), tr( "Show in &Overview" ), parent );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::showInOverview );
  a->setCheckable( true );
  a->setChecked( node->customProperty( QStringLiteral( "overview" ), 0 ).toInt() );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionRenameGroupOrLayer( QObject *parent )
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node )
    return nullptr;

  QString text;
  if ( QgsLayerTree::isGroup( node ) )
    text = tr( "Re&name Group" );
  else
    text = tr( "Re&name Layer" );

  QAction *a = new QAction( text, parent );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::renameGroupOrLayer );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionShowFeatureCount( QObject *parent )
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node )
    return nullptr;

  QAction *a = new QAction( tr( "Show Feature &Count" ), parent );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::showFeatureCount );
  a->setCheckable( true );
  a->setChecked( node->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toInt() );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionZoomToLayer( QgsMapCanvas *canvas, QObject *parent )
{
  QAction *a = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToLayer.svg" ) ),
                            tr( "&Zoom to Layer" ), parent );
  a->setData( QVariant::fromValue( reinterpret_cast<void *>( canvas ) ) );
  Q_NOWARN_DEPRECATED_PUSH
  connect( a, &QAction::triggered, this, static_cast<void ( QgsLayerTreeViewDefaultActions::* )()>( &QgsLayerTreeViewDefaultActions::zoomToLayer ) );
  Q_NOWARN_DEPRECATED_POP
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionZoomToLayers( QgsMapCanvas *canvas, QObject *parent )
{
  QAction *a = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToLayer.svg" ) ),
                            tr( "&Zoom to Layer(s)" ), parent );
  a->setData( QVariant::fromValue( canvas ) );
  connect( a, &QAction::triggered, this, static_cast<void ( QgsLayerTreeViewDefaultActions::* )()>( &QgsLayerTreeViewDefaultActions::zoomToLayers ) );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionZoomToSelection( QgsMapCanvas *canvas, QObject *parent )
{
  QAction *a = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToSelected.svg" ) ),
                            tr( "Zoom to &Selection" ), parent );
  a->setData( QVariant::fromValue( reinterpret_cast<void *>( canvas ) ) );
  connect( a, &QAction::triggered, this, static_cast<void ( QgsLayerTreeViewDefaultActions::* )()>( &QgsLayerTreeViewDefaultActions::zoomToSelection ) );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionZoomToGroup( QgsMapCanvas *canvas, QObject *parent )
{
  QAction *a = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToLayer.svg" ) ),
                            tr( "&Zoom to Group" ), parent );
  a->setData( QVariant::fromValue( reinterpret_cast<void *>( canvas ) ) );
  connect( a, &QAction::triggered, this, static_cast<void ( QgsLayerTreeViewDefaultActions::* )()>( &QgsLayerTreeViewDefaultActions::zoomToGroup ) );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionMakeTopLevel( QObject *parent )
{
  QAction *a = new QAction( tr( "&Move to Top-level" ), parent );
  Q_NOWARN_DEPRECATED_PUSH
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::makeTopLevel );
  Q_NOWARN_DEPRECATED_POP
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionMoveOutOfGroup( QObject *parent )
{
  QAction *a = new QAction( tr( "Move O&ut of Group" ), parent );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::moveOutOfGroup );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionMoveToTop( QObject *parent )
{
  QAction *a = new QAction( tr( "Move to &Top" ), parent );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::moveToTop );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionMoveToBottom( QObject *parent )
{
  QAction *a = new QAction( tr( "Move to &Bottom" ), parent );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::moveToBottom );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionGroupSelected( QObject *parent )
{
  QAction *a = new QAction( tr( "&Group Selected" ), parent );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::groupSelected );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionMutuallyExclusiveGroup( QObject *parent )
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node || !QgsLayerTree::isGroup( node ) )
    return nullptr;

  QAction *a = new QAction( tr( "&Mutually Exclusive Group" ), parent );
  a->setCheckable( true );
  a->setChecked( QgsLayerTree::toGroup( node )->isMutuallyExclusive() );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::mutuallyExclusiveGroup );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionCheckAndAllChildren( QObject *parent )
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node || !QgsLayerTree::isGroup( node ) || node->isItemVisibilityCheckedRecursive() )
    return nullptr;
#ifdef Q_OS_MACX
  QAction *a = new QAction( tr( "Check and All its Children (⌘-click)" ), parent );
#else
  QAction *a = new QAction( tr( "Check and All its Children (Ctrl-click)" ), parent );
#endif
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::checkAndAllChildren );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionUncheckAndAllChildren( QObject *parent )
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node || !QgsLayerTree::isGroup( node ) || node->isItemVisibilityUncheckedRecursive() )
    return nullptr;
#ifdef Q_OS_MACX
  QAction *a = new QAction( tr( "Uncheck and All its Children (⌘-click)" ), parent );
#else
  QAction *a = new QAction( tr( "Uncheck and All its Children (Ctrl-click)" ), parent );
#endif
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::uncheckAndAllChildren );
  return a;
}

QAction *QgsLayerTreeViewDefaultActions::actionCheckAndAllParents( QObject *parent )
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node || !QgsLayerTree::isLayer( node ) || node->isVisible() )
    return nullptr;
  QAction *a = new QAction( tr( "Chec&k and All its Parents" ), parent );
  connect( a, &QAction::triggered, this, &QgsLayerTreeViewDefaultActions::checkAndAllParents );
  return a;
}

void QgsLayerTreeViewDefaultActions::checkAndAllChildren()
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node )
    return;
  node->setItemVisibilityCheckedRecursive( true );
}

void QgsLayerTreeViewDefaultActions::uncheckAndAllChildren()
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node )
    return;
  node->setItemVisibilityCheckedRecursive( false );
}

void QgsLayerTreeViewDefaultActions::checkAndAllParents()
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node )
    return;
  node->setItemVisibilityCheckedParentRecursive( true );
}

void QgsLayerTreeViewDefaultActions::addGroup()
{
  int nodeCount = mView->selectedNodes( true ).count();
  if ( nodeCount > 1 || ( nodeCount == 1 && mView->currentLayer() ) )
  {
    groupSelected();
    return;
  }
  QgsLayerTreeGroup *group = mView->currentGroupNode();
  if ( !group )
    group = mView->layerTreeModel()->rootGroup();

  QgsLayerTreeGroup *newGroup = group->addGroup( uniqueGroupName( group ) );
  mView->edit( mView->node2index( newGroup ) );
}

void QgsLayerTreeViewDefaultActions::removeGroupOrLayer()
{
  const auto constSelectedNodes = mView->selectedNodes( true );
  for ( QgsLayerTreeNode *node : constSelectedNodes )
  {
    // could be more efficient if working directly with ranges instead of individual nodes
    qobject_cast<QgsLayerTreeGroup *>( node->parent() )->removeChildNode( node );
  }
}

void QgsLayerTreeViewDefaultActions::renameGroupOrLayer()
{
  mView->edit( mView->currentIndex() );
}

void QgsLayerTreeViewDefaultActions::showInOverview()
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node )
    return;
  int newValue = node->customProperty( QStringLiteral( "overview" ), 0 ).toInt();
  const auto constSelectedLayerNodes = mView->selectedLayerNodes();
  for ( QgsLayerTreeLayer *l : constSelectedLayerNodes )
    l->setCustomProperty( QStringLiteral( "overview" ), newValue ? 0 : 1 );
}

void QgsLayerTreeViewDefaultActions::showFeatureCount()
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  int newValue = node->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toInt();
  const auto constSelectedLayerNodes = mView->selectedLayerNodes();
  for ( QgsLayerTreeLayer *l : constSelectedLayerNodes )
    l->setCustomProperty( QStringLiteral( "showFeatureCount" ), newValue ? 0 : 1 );
}

void QgsLayerTreeViewDefaultActions::zoomToLayer( QgsMapCanvas *canvas )
{
  QgsMapLayer *layer = mView->currentLayer();
  if ( !layer )
    return;

  const QList<QgsMapLayer *> layers { layer };
  zoomToLayers( canvas, layers );
}

void QgsLayerTreeViewDefaultActions::zoomToLayers( QgsMapCanvas *canvas )
{
  const QList<QgsMapLayer *> layers = mView->selectedLayers();

  zoomToLayers( canvas, layers );
}

void QgsLayerTreeViewDefaultActions::zoomToSelection( QgsMapCanvas *canvas )
{
  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( mView->currentLayer() );

  const QList<QgsMapLayer *> layers = mView->selectedLayers();

  if ( layers.size() > 1 )
    canvas->zoomToSelected( layers );
  else if ( layers.size() <= 1 && layer )
    canvas->zoomToSelected( layer );

}

void QgsLayerTreeViewDefaultActions::zoomToGroup( QgsMapCanvas *canvas )
{
  QgsLayerTreeGroup *groupNode = mView->currentGroupNode();
  if ( !groupNode )
    return;

  QList<QgsMapLayer *> layers;
  const QStringList findLayerIds = groupNode->findLayerIds();
  for ( const QString &layerId : findLayerIds )
    layers << QgsProject::instance()->mapLayer( layerId );

  zoomToLayers( canvas, layers );
}

void QgsLayerTreeViewDefaultActions::zoomToLayer()
{
  Q_NOWARN_DEPRECATED_PUSH
  QAction *s = qobject_cast<QAction *>( sender() );
  QgsMapCanvas *canvas = reinterpret_cast<QgsMapCanvas *>( s->data().value<void *>() );

  zoomToLayer( canvas );
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayerTreeViewDefaultActions::zoomToLayers()
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QgsMapCanvas *canvas = s->data().value<QgsMapCanvas *>();
  zoomToLayers( canvas );
}

void QgsLayerTreeViewDefaultActions::zoomToSelection()
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QgsMapCanvas *canvas = reinterpret_cast<QgsMapCanvas *>( s->data().value<void *>() );
  zoomToSelection( canvas );
}

void QgsLayerTreeViewDefaultActions::zoomToGroup()
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QgsMapCanvas *canvas = reinterpret_cast<QgsMapCanvas *>( s->data().value<void *>() );
  zoomToGroup( canvas );
}

void QgsLayerTreeViewDefaultActions::zoomToLayers( QgsMapCanvas *canvas, const QList<QgsMapLayer *> &layers )
{
  QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );

  QgsRectangle extent;
  extent.setMinimal();

  if ( !layers.empty() )
  {
    for ( int i = 0; i < layers.size(); ++i )
    {
      QgsMapLayer *layer = layers.at( i );
      QgsRectangle layerExtent = layer->extent();

      QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( vLayer )
      {
        if ( vLayer->geometryType() == QgsWkbTypes::NullGeometry )
          continue;

        if ( layerExtent.isEmpty() )
        {
          vLayer->updateExtents();
          layerExtent = vLayer->extent();
        }
      }

      if ( layerExtent.isNull() )
        continue;

      //transform extent
      layerExtent = canvas->mapSettings().layerExtentToOutputExtent( layer, layerExtent );

      extent.combineExtentWith( layerExtent );
    }
  }

  // If no layer is selected, use current layer
  else if ( mView->currentLayer() )
  {
    QgsRectangle layerExtent = mView->currentLayer()->extent();
    layerExtent = canvas->mapSettings().layerExtentToOutputExtent( mView->currentLayer(), layerExtent );
    extent.combineExtentWith( layerExtent );
  }

  if ( extent.isNull() )
    return;

  // Increase bounding box with 5%, so that layer is a bit inside the borders
  extent.scale( 1.05 );

  //zoom to bounding box
  canvas->setExtent( extent, true );
  canvas->refresh();
}


QString QgsLayerTreeViewDefaultActions::uniqueGroupName( QgsLayerTreeGroup *parentGroup )
{
  QString prefix = parentGroup == mView->layerTreeModel()->rootGroup() ? "group" : "sub-group";
  QString newName = prefix + '1';
  for ( int i = 2; parentGroup->findGroup( newName ); ++i )
    newName = prefix + QString::number( i );
  return newName;
}


void QgsLayerTreeViewDefaultActions::makeTopLevel()
{
  const auto constSelectedLayerNodes = mView->selectedLayerNodes();
  for ( QgsLayerTreeLayer *l : constSelectedLayerNodes )
  {
    QgsLayerTreeGroup *rootGroup = mView->layerTreeModel()->rootGroup();
    QgsLayerTreeGroup *parentGroup = qobject_cast<QgsLayerTreeGroup *>( l->parent() );
    if ( !parentGroup || parentGroup == rootGroup )
      continue;
    QgsLayerTreeLayer *clonedLayer = l->clone();
    rootGroup->addChildNode( clonedLayer );
    parentGroup->removeChildNode( l );
  }
}


void QgsLayerTreeViewDefaultActions::moveOutOfGroup()
{
  const QList< QgsLayerTreeLayer * >  selectedLayerNodes = mView->selectedLayerNodes();
  for ( QgsLayerTreeLayer *l : selectedLayerNodes )
  {
    QgsLayerTreeGroup *rootGroup = mView->layerTreeModel()->rootGroup();
    QgsLayerTreeGroup *parentGroup = qobject_cast<QgsLayerTreeGroup *>( l->parent() );
    if ( !parentGroup || parentGroup == rootGroup )
      continue;
    QgsLayerTreeGroup *tempGroup = parentGroup;
    while ( tempGroup->parent() != rootGroup )
    {
      tempGroup = qobject_cast<QgsLayerTreeGroup *>( tempGroup->parent() );
    }
    QgsLayerTreeLayer *clonedLayer = l->clone();
    int insertIdx = rootGroup->children().indexOf( tempGroup );
    rootGroup->insertChildNode( insertIdx, clonedLayer );
    parentGroup->removeChildNode( l );
  }
}


void QgsLayerTreeViewDefaultActions::moveToTop()
{
  QList< QgsLayerTreeNode * >  selectedNodes = mView->selectedNodes();
  std::reverse( selectedNodes.begin(), selectedNodes.end() );
  // sort the nodes by depth first to avoid moving a group before its contents
  std::stable_sort( selectedNodes.begin(), selectedNodes.end(), []( const QgsLayerTreeNode * a, const QgsLayerTreeNode * b )
  {
    return a->depth() > b->depth();
  } );
  for ( QgsLayerTreeNode *n : std::as_const( selectedNodes ) )
  {
    QgsLayerTreeGroup *parentGroup = qobject_cast<QgsLayerTreeGroup *>( n->parent() );
    QgsLayerTreeNode *clonedNode = n->clone();
    parentGroup->insertChildNode( 0, clonedNode );
    parentGroup->removeChildNode( n );
  }
}


void QgsLayerTreeViewDefaultActions::moveToBottom()
{
  QList< QgsLayerTreeNode * > selectedNodes = mView->selectedNodes();
  // sort the nodes by depth first to avoid moving a group before its contents
  std::stable_sort( selectedNodes.begin(), selectedNodes.end(), []( const QgsLayerTreeNode * a, const QgsLayerTreeNode * b )
  {
    return a->depth() > b->depth();
  } );
  for ( QgsLayerTreeNode *n : std::as_const( selectedNodes ) )
  {
    QgsLayerTreeGroup *parentGroup = qobject_cast<QgsLayerTreeGroup *>( n->parent() );
    QgsLayerTreeNode *clonedNode = n->clone();
    parentGroup->insertChildNode( -1, clonedNode );
    parentGroup->removeChildNode( n );
  }
}


void QgsLayerTreeViewDefaultActions::groupSelected()
{
  const QList<QgsLayerTreeNode *> nodes = mView->selectedNodes( true );
  if ( nodes.empty() || ! QgsLayerTree::isGroup( nodes[0]->parent() ) )
    return;

  QgsLayerTreeGroup *parentGroup = QgsLayerTree::toGroup( nodes[0]->parent() );
  int insertIdx = parentGroup->children().indexOf( nodes[0] );

  QgsLayerTreeGroup *newGroup = new QgsLayerTreeGroup( uniqueGroupName( parentGroup ) );
  for ( QgsLayerTreeNode *node : nodes )
    newGroup->addChildNode( node->clone() );

  parentGroup->insertChildNode( insertIdx, newGroup );

  for ( QgsLayerTreeNode *node : nodes )
  {
    QgsLayerTreeGroup *group = qobject_cast<QgsLayerTreeGroup *>( node->parent() );
    if ( group )
      group->removeChildNode( node );
  }

  mView->setCurrentIndex( mView->node2index( newGroup ) );
  mView->edit( mView->node2index( newGroup ) );
}

void QgsLayerTreeViewDefaultActions::mutuallyExclusiveGroup()
{
  QgsLayerTreeNode *node = mView->currentNode();
  if ( !node || !QgsLayerTree::isGroup( node ) )
    return;

  QgsLayerTree::toGroup( node )->setIsMutuallyExclusive( !QgsLayerTree::toGroup( node )->isMutuallyExclusive() );
}
