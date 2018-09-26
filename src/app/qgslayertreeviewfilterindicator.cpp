/***************************************************************************
  qgslayertreeviewfilterindicator.cpp
  --------------------------------------
  Date                 : January 2018
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

#include "qgslayertreeviewfilterindicator.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeview.h"
#include "qgsquerybuilder.h"
#include "qgsvectorlayer.h"


QgsLayerTreeViewFilterIndicatorProvider::QgsLayerTreeViewFilterIndicatorProvider( QgsLayerTreeView *view )
  : QObject( view )
  , mLayerTreeView( view )
{
  mIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIndicatorFilter.svg" ) );

  QgsLayerTree *tree = mLayerTreeView->layerTreeModel()->rootGroup();
  onAddedChildren( tree, 0, tree->children().count() - 1 );

  connect( tree, &QgsLayerTree::addedChildren, this, &QgsLayerTreeViewFilterIndicatorProvider::onAddedChildren );
  connect( tree, &QgsLayerTree::willRemoveChildren, this, &QgsLayerTreeViewFilterIndicatorProvider::onWillRemoveChildren );
}


void QgsLayerTreeViewFilterIndicatorProvider::onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
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
      if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( childLayerNode->layer() ) )
      {
        if ( QgsLayerTreeUtils::countMapLayerInTree( mLayerTreeView->layerTreeModel()->rootGroup(), vlayer ) == 1 )
          connect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onSubsetStringChanged );

        addOrRemoveIndicator( childLayerNode, vlayer );
      }
      else if ( !childLayerNode->layer() )
      {
        // wait for layer to be loaded (e.g. when loading project, first the tree is loaded, afterwards the references to layers are resolved)
        connect( childLayerNode, &QgsLayerTreeLayer::layerLoaded, this, &QgsLayerTreeViewFilterIndicatorProvider::onLayerLoaded );
      }
    }
  }
}


void QgsLayerTreeViewFilterIndicatorProvider::onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
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
        if ( QgsLayerTreeUtils::countMapLayerInTree( mLayerTreeView->layerTreeModel()->rootGroup(), vlayer ) == 1 )
          disconnect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onSubsetStringChanged );
      }
    }
  }
}


void QgsLayerTreeViewFilterIndicatorProvider::onLayerLoaded()
{
  QgsLayerTreeLayer *nodeLayer = qobject_cast<QgsLayerTreeLayer *>( sender() );
  if ( !nodeLayer )
    return;

  if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( nodeLayer->layer() ) )
  {
    if ( vlayer )
    {
      connect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &QgsLayerTreeViewFilterIndicatorProvider::onSubsetStringChanged );

      addOrRemoveIndicator( nodeLayer, vlayer );
    }
  }
}


void QgsLayerTreeViewFilterIndicatorProvider::onSubsetStringChanged()
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


void QgsLayerTreeViewFilterIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->layerTreeModel()->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() );
  if ( !vlayer )
    return;

  // launch the query builder
  QgsQueryBuilder qb( vlayer );
  qb.setSql( vlayer->subsetString() );
  if ( qb.exec() )
    vlayer->setSubsetString( qb.sql() );
}

std::unique_ptr<QgsLayerTreeViewIndicator> QgsLayerTreeViewFilterIndicatorProvider::newIndicator( const QString &filter )
{
  std::unique_ptr< QgsLayerTreeViewIndicator > indicator = qgis::make_unique< QgsLayerTreeViewIndicator >( this );
  indicator->setIcon( mIcon );
  updateIndicator( indicator.get(), filter );
  connect( indicator.get(), &QgsLayerTreeViewIndicator::clicked, this, &QgsLayerTreeViewFilterIndicatorProvider::onIndicatorClicked );
  mIndicators.insert( indicator.get() );
  return indicator;
}

void QgsLayerTreeViewFilterIndicatorProvider::updateIndicator( QgsLayerTreeViewIndicator *indicator, const QString &filter )
{
  indicator->setToolTip( QStringLiteral( "<b>%1:</b><br>%2" ).arg( tr( "Filter" ), filter ) );
}


void QgsLayerTreeViewFilterIndicatorProvider::addOrRemoveIndicator( QgsLayerTreeNode *node, QgsVectorLayer *vlayer )
{
  QString filter = vlayer->subsetString();
  if ( !filter.isEmpty() )
  {
    const QList<QgsLayerTreeViewIndicator *> nodeIndicators = mLayerTreeView->indicators( node );

    // maybe the indicator exists already
    for ( QgsLayerTreeViewIndicator *indicator : nodeIndicators )
    {
      if ( mIndicators.contains( indicator ) )
      {
        updateIndicator( indicator, filter );
        return;
      }
    }

    // it does not exist: need to create a new one
    mLayerTreeView->addIndicator( node, newIndicator( filter ).release() );
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
