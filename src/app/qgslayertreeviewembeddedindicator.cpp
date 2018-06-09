/***************************************************************************
  qgslayertreeviewembeddedindicator.h
  --------------------------------------
  Date                 : June 2018
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

#include "qgslayertreeviewembeddedindicator.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeview.h"

QgsLayerTreeViewEmbeddedIndicatorProvider::QgsLayerTreeViewEmbeddedIndicatorProvider( QgsLayerTreeView *view )
  : QObject( view )
  , mLayerTreeView( view )
{
  mIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIndicatorFilter.svg" ) );

  QgsLayerTree *tree = mLayerTreeView->layerTreeModel()->rootGroup();
  onAddedChildren( tree, 0, tree->children().count() - 1 );

  connect( tree, &QgsLayerTree::addedChildren, this, &QgsLayerTreeViewEmbeddedIndicatorProvider::onAddedChildren );
}

void QgsLayerTreeViewEmbeddedIndicatorProvider::onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
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
    else if ( QgsLayerTree::isLayer( childNode ) && childNode->customProperty( QStringLiteral( "embedded" ) ).toInt() )
    {
      QgsLayerTreeLayer *childLayerNode = QgsLayerTree::toLayer( childNode );
      addIndicatorForEmbeddedLayer( childLayerNode );
    }
  }
}

QgsLayerTreeViewIndicator *QgsLayerTreeViewEmbeddedIndicatorProvider::newIndicator( const QString &project )
{
  QgsLayerTreeViewIndicator *indicator = new QgsLayerTreeViewIndicator( this );
  indicator->setIcon( mIcon );
  indicator->setToolTip( tr( "Embedded from <b>%1</b>" ).arg( project ) );
  mIndicators.insert( indicator );
  return indicator;
}

void QgsLayerTreeViewEmbeddedIndicatorProvider::addIndicatorForEmbeddedLayer( QgsLayerTreeNode *node )
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
  mLayerTreeView->addIndicator( node, newIndicator( node->customProperty( QStringLiteral( "embedded_project" ) ).toString() ) );
}
