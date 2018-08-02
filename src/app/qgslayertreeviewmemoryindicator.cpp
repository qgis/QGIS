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

QgsLayerTreeViewMemoryIndicatorProvider::QgsLayerTreeViewMemoryIndicatorProvider( QgsLayerTreeView *view )
  : QObject( view )
  , mLayerTreeView( view )
{
  mIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIndicatorMemory.svg" ) );

  QgsLayerTree *tree = mLayerTreeView->layerTreeModel()->rootGroup();
  onAddedChildren( tree, 0, tree->children().count() - 1 );

  connect( tree, &QgsLayerTree::addedChildren, this, &QgsLayerTreeViewMemoryIndicatorProvider::onAddedChildren );
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
        if ( layerNode->layer() && layerNode->layer()->dataProvider()->name() == QLatin1String( "memory" ) )
          addIndicatorForMemoryLayer( childNode );
        else if ( !layerNode->layer() )
        {
          // wait for layer to be loaded (e.g. when loading project, first the tree is loaded, afterwards the references to layers are resolved)
          connect( layerNode, &QgsLayerTreeLayer::layerLoaded, this, &QgsLayerTreeViewMemoryIndicatorProvider::onLayerLoaded );
        }
      }
    }
  }
}

void QgsLayerTreeViewMemoryIndicatorProvider::onLayerLoaded()
{
  QgsLayerTreeLayer *layerNode = qobject_cast<QgsLayerTreeLayer *>( sender() );
  if ( !layerNode )
    return;

  if ( layerNode->layer() && layerNode->layer()->dataProvider()->name() == QLatin1String( "memory" ) )
    addIndicatorForMemoryLayer( layerNode );
}

std::unique_ptr< QgsLayerTreeViewIndicator > QgsLayerTreeViewMemoryIndicatorProvider::newIndicator()
{
  std::unique_ptr< QgsLayerTreeViewIndicator > indicator = qgis::make_unique< QgsLayerTreeViewIndicator >( this );
  indicator->setIcon( mIcon );
  indicator->setToolTip( tr( "Temporary scratch layer only<br><b>Contents will be discarded after closing this project</b>" ) );
  mIndicators.insert( indicator.get() );
  return indicator;
}

void QgsLayerTreeViewMemoryIndicatorProvider::addIndicatorForMemoryLayer( QgsLayerTreeNode *node )
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
