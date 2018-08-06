/***************************************************************************
  qgslayertreeviewmemoryindicator.h
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewmemoryindicator.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeview.h"
#include "qgsvectorlayer.h"

QgsLayerTreeViewMemoryIndicatorProvider::QgsLayerTreeViewMemoryIndicatorProvider( QgsLayerTreeView *view )
  : QObject( view )
  , mLayerTreeView( view )
{
  mIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIndicatorMemory.svg" ) );

  QgsLayerTree *tree = mLayerTreeView->layerTreeModel()->rootGroup();
  onAddedChildren( tree, 0, tree->children().count() - 1 );

  connect( tree, &QgsLayerTree::addedChildren, this, &QgsLayerTreeViewMemoryIndicatorProvider::onAddedChildren );
  connect( tree, &QgsLayerTree::willRemoveChildren, this, &QgsLayerTreeViewMemoryIndicatorProvider::onWillRemoveChildren );
}

void QgsLayerTreeViewMemoryIndicatorProvider::onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  // recursively populate indicators
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
      if ( QgsLayerTreeLayer *layerNode = dynamic_cast< QgsLayerTreeLayer * >( childNode ) )
      {
        if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layerNode->layer() ) )
        {
          connect( vlayer, &QgsVectorLayer::dataSourceChanged, this, &QgsLayerTreeViewMemoryIndicatorProvider::onDataSourceChanged );
          addOrRemoveIndicator( childNode, vlayer );
        }
        else if ( !layerNode->layer() )
        {
          // wait for layer to be loaded (e.g. when loading project, first the tree is loaded, afterwards the references to layers are resolved)
          connect( layerNode, &QgsLayerTreeLayer::layerLoaded, this, &QgsLayerTreeViewMemoryIndicatorProvider::onLayerLoaded );
        }
      }
    }
  }
}

void QgsLayerTreeViewMemoryIndicatorProvider::onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
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
      if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( childLayerNode->layer() ) )
      {
        if ( vlayer )
          disconnect( vlayer, &QgsVectorLayer::dataSourceChanged, this, &QgsLayerTreeViewMemoryIndicatorProvider::onDataSourceChanged );
      }
    }
  }
}

void QgsLayerTreeViewMemoryIndicatorProvider::onLayerLoaded()
{
  QgsLayerTreeLayer *layerNode = qobject_cast<QgsLayerTreeLayer *>( sender() );
  if ( !layerNode )
    return;

  if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layerNode->layer() ) )
  {
    if ( vlayer )
    {
      connect( vlayer, &QgsVectorLayer::dataSourceChanged, this, &QgsLayerTreeViewMemoryIndicatorProvider::onDataSourceChanged );
      addOrRemoveIndicator( layerNode, vlayer );
    }
  }
}

void QgsLayerTreeViewMemoryIndicatorProvider::onDataSourceChanged()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( sender() );
  if ( !vlayer )
    return;

  // walk the tree and find layer node that needs to be updated
  const QList<QgsLayerTreeLayer *> layerNodes = mLayerTreeView->layerTreeModel()->rootGroup()->findLayers();
  for ( QgsLayerTreeLayer *node : layerNodes )
  {
    if ( node->layer() && node->layer() == vlayer )
    {
      addOrRemoveIndicator( node, vlayer );
      break;
    }
  }
}

std::unique_ptr< QgsLayerTreeViewIndicator > QgsLayerTreeViewMemoryIndicatorProvider::newIndicator()
{
  std::unique_ptr< QgsLayerTreeViewIndicator > indicator = qgis::make_unique< QgsLayerTreeViewIndicator >( this );
  indicator->setIcon( mIcon );
  indicator->setToolTip( tr( "<b>Temporary scratch layer only!</b><br>Contents will be discarded after closing this project" ) );
  mIndicators.insert( indicator.get() );
  return indicator;
}

void QgsLayerTreeViewMemoryIndicatorProvider::addOrRemoveIndicator( QgsLayerTreeNode *node, QgsVectorLayer *vlayer )
{
  const bool isMemory = vlayer->dataProvider()->name() == QLatin1String( "memory" );

  if ( isMemory )
  {
    const QList<QgsLayerTreeViewIndicator *> nodeIndicators = mLayerTreeView->indicators( node );

    // maybe the indicator exists already
    for ( QgsLayerTreeViewIndicator *indicator : nodeIndicators )
    {
      if ( mIndicators.contains( indicator ) )
      {
        return;
      }
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
