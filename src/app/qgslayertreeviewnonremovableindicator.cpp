/***************************************************************************
  qgslayertreeviewnonremovableindicator.cpp
  --------------------------------------
  Date                 : Sep 2018
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

#include "qgslayertreeviewnonremovableindicator.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeview.h"


QgsLayerTreeViewNonRemovableIndicatorProvider::QgsLayerTreeViewNonRemovableIndicatorProvider( QgsLayerTreeView *view )
  : QObject( view )
  , mLayerTreeView( view )
{
  mIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIndicatorNonRemovable.svg" ) );

  QgsLayerTree *tree = mLayerTreeView->layerTreeModel()->rootGroup();
  onAddedChildren( tree, 0, tree->children().count() - 1 );

  connect( tree, &QgsLayerTree::addedChildren, this, &QgsLayerTreeViewNonRemovableIndicatorProvider::onAddedChildren );
  connect( tree, &QgsLayerTree::willRemoveChildren, this, &QgsLayerTreeViewNonRemovableIndicatorProvider::onWillRemoveChildren );
}

void QgsLayerTreeViewNonRemovableIndicatorProvider::onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  // recursively connect to providers' dataChanged() signal

  QList<QgsLayerTreeNode *> children = node->children();
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode *childNode = children[i];

    if ( QgsLayerTree::isGroup( childNode ) )
    {
      onAddedChildren( childNode, 0, childNode->children().count() - 1 );
    }
    else if ( QgsLayerTree::isLayer( childNode ) )
    {
      QgsLayerTreeLayer *childLayerNode = QgsLayerTree::toLayer( childNode );
      if ( QgsMapLayer *layer = childLayerNode->layer() )
      {
        if ( QgsLayerTreeUtils::countMapLayerInTree( mLayerTreeView->layerTreeModel()->rootGroup(), layer ) == 1 )
          connect( layer, &QgsMapLayer::flagsChanged, this, &QgsLayerTreeViewNonRemovableIndicatorProvider::onFlagsChanged );
        addOrRemoveIndicator( childLayerNode, layer );
      }
      else if ( !childLayerNode->layer() )
      {
        // wait for layer to be loaded (e.g. when loading project, first the tree is loaded, afterwards the references to layers are resolved)
        connect( childLayerNode, &QgsLayerTreeLayer::layerLoaded, this, &QgsLayerTreeViewNonRemovableIndicatorProvider::onLayerLoaded );
      }
    }
  }
}


void QgsLayerTreeViewNonRemovableIndicatorProvider::onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  // recursively disconnect from providers' dataChanged() signal

  QList<QgsLayerTreeNode *> children = node->children();
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode *childNode = children[i];

    if ( QgsLayerTree::isGroup( childNode ) )
    {
      onWillRemoveChildren( childNode, 0, childNode->children().count() - 1 );
    }
    else if ( QgsLayerTree::isLayer( childNode ) )
    {
      QgsLayerTreeLayer *childLayerNode = QgsLayerTree::toLayer( childNode );
      if ( childLayerNode->layer() )
      {
        if ( QgsLayerTreeUtils::countMapLayerInTree( mLayerTreeView->layerTreeModel()->rootGroup(), childLayerNode->layer() ) == 1 )
          disconnect( childLayerNode->layer(), &QgsMapLayer::flagsChanged, this, &QgsLayerTreeViewNonRemovableIndicatorProvider::onFlagsChanged );
      }
    }
  }
}


void QgsLayerTreeViewNonRemovableIndicatorProvider::onLayerLoaded()
{
  QgsLayerTreeLayer *nodeLayer = qobject_cast<QgsLayerTreeLayer *>( sender() );
  if ( !nodeLayer )
    return;

  if ( QgsMapLayer *layer = nodeLayer->layer() )
  {
    connect( layer, &QgsMapLayer::flagsChanged, this, &QgsLayerTreeViewNonRemovableIndicatorProvider::onFlagsChanged );
    addOrRemoveIndicator( nodeLayer, layer );
  }
}

void QgsLayerTreeViewNonRemovableIndicatorProvider::onFlagsChanged()
{
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  if ( !layer )
    return;

  // walk the tree and find layer node that needs to be updated
  const QList<QgsLayerTreeLayer *> layerNodes = mLayerTreeView->layerTreeModel()->rootGroup()->findLayers();
  for ( QgsLayerTreeLayer *node : layerNodes )
  {
    if ( node->layer() && node->layer() == layer )
    {
      addOrRemoveIndicator( node, layer );
      break;
    }
  }
}


std::unique_ptr<QgsLayerTreeViewIndicator> QgsLayerTreeViewNonRemovableIndicatorProvider::newIndicator()
{
  std::unique_ptr< QgsLayerTreeViewIndicator > indicator = qgis::make_unique< QgsLayerTreeViewIndicator >( this );
  indicator->setIcon( mIcon );
  indicator->setToolTip( tr( "Layer required by the project" ) );
  mIndicators.insert( indicator.get() );
  return indicator;
}

void QgsLayerTreeViewNonRemovableIndicatorProvider::addOrRemoveIndicator( QgsLayerTreeNode *node, QgsMapLayer *layer )
{
  bool removable = layer->flags() & QgsMapLayer::Removable;
  if ( !removable )
  {
    const QList<QgsLayerTreeViewIndicator *> nodeIndicators = mLayerTreeView->indicators( node );

    // maybe the indicator exists already
    for ( QgsLayerTreeViewIndicator *indicator : nodeIndicators )
    {
      if ( mIndicators.contains( indicator ) )
        return;
    }

    // it does not exist: need to create a new one
    mLayerTreeView->addIndicator( node, newIndicator().release() );
  }
  else
  {
    const QList<QgsLayerTreeViewIndicator *> nodeIndicators = mLayerTreeView->indicators( node );

    // there may be existing indicator we need to get rid of
    for ( QgsLayerTreeViewIndicator *indicator : nodeIndicators )
    {
      if ( mIndicators.contains( indicator ) )
      {
        mLayerTreeView->removeIndicator( node, indicator );
        indicator->deleteLater();
        return;
      }
    }

    // no indicator was there before, nothing to do
  }
}
