/***************************************************************************
  qgslayertreeviewindicatorprovider.cpp - QgsLayerTreeViewIndicatorProvider

 ---------------------
 begin                : 17.10.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso@itopen.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayertreeviewindicatorprovider.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeview.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgisapp.h"
#include "qgsapplication.h"

QgsLayerTreeViewIndicatorProvider::QgsLayerTreeViewIndicatorProvider( QgsLayerTreeView *view )
  : QObject( view )
  , mLayerTreeView( view )
{

  QgsLayerTree *tree = mLayerTreeView->layerTreeModel()->rootGroup();
  onAddedChildren( tree, 0, tree->children().count() - 1 );

  connect( tree, &QgsLayerTree::addedChildren, this, &QgsLayerTreeViewIndicatorProvider::onAddedChildren );
  connect( tree, &QgsLayerTree::willRemoveChildren, this, &QgsLayerTreeViewIndicatorProvider::onWillRemoveChildren );
}

void QgsLayerTreeViewIndicatorProvider::onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
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
        if ( layerNode->layer() )
        {
          connectSignals( layerNode->layer() );
          addOrRemoveIndicator( childNode, layerNode->layer() );
        }
        else
        {
          // wait for layer to be loaded (e.g. when loading project, first the tree is loaded, afterwards the references to layers are resolved)
          connect( layerNode, &QgsLayerTreeLayer::layerLoaded, this, &QgsLayerTreeViewIndicatorProvider::onLayerLoaded );
        }
      }
    }
  }
}

void QgsLayerTreeViewIndicatorProvider::onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  // recursively call disconnect signals

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
      if ( QgsLayerTreeUtils::countMapLayerInTree( mLayerTreeView->layerTreeModel()->rootGroup(), childLayerNode->layer() ) == 1 )
        disconnectSignals( childLayerNode->layer() );
    }
  }
}

void QgsLayerTreeViewIndicatorProvider::onLayerLoaded()
{

  QgsLayerTreeLayer *layerNode = qobject_cast<QgsLayerTreeLayer *>( sender() );
  if ( !layerNode )
    return;

  if ( !( qobject_cast<QgsVectorLayer *>( layerNode->layer() ) || qobject_cast<QgsRasterLayer *>( layerNode->layer() ) ) )
    return;

  if ( QgsMapLayer *mapLayer = qobject_cast<QgsMapLayer *>( layerNode->layer() ) )
  {
    if ( mapLayer )
    {
      connectSignals( mapLayer );
      addOrRemoveIndicator( layerNode, mapLayer );
    }
  }
}

void QgsLayerTreeViewIndicatorProvider::onLayerChanged()
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

void QgsLayerTreeViewIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  if ( !( qobject_cast<QgsVectorLayer *>( layer ) || qobject_cast<QgsRasterLayer *>( layer ) ) )
    return;
  QgsMapLayer *mapLayer = qobject_cast<QgsMapLayer *>( layer );
  connect( mapLayer, &QgsMapLayer::dataSourceChanged, this, &QgsLayerTreeViewIndicatorProvider::onLayerChanged );
}

void QgsLayerTreeViewIndicatorProvider::disconnectSignals( QgsMapLayer *layer )
{
  if ( !( qobject_cast<QgsVectorLayer *>( layer ) || qobject_cast<QgsRasterLayer *>( layer ) ) )
    return;
  QgsMapLayer *mapLayer = qobject_cast<QgsMapLayer *>( layer );
  disconnect( mapLayer, &QgsMapLayer::dataSourceChanged, this, &QgsLayerTreeViewIndicatorProvider::onLayerChanged );
}

std::unique_ptr< QgsLayerTreeViewIndicator > QgsLayerTreeViewIndicatorProvider::newIndicator( QgsMapLayer *layer )
{
  std::unique_ptr< QgsLayerTreeViewIndicator > indicator = qgis::make_unique< QgsLayerTreeViewIndicator >( this );
  indicator->setIcon( QgsApplication::getThemeIcon( iconName( layer ) ) );
  indicator->setToolTip( tooltipText( layer ) );
  connect( indicator.get(), &QgsLayerTreeViewIndicator::clicked, this, &QgsLayerTreeViewIndicatorProvider::onIndicatorClicked );
  mIndicators.insert( indicator.get() );
  return indicator;
}

void QgsLayerTreeViewIndicatorProvider::addOrRemoveIndicator( QgsLayerTreeNode *node, QgsMapLayer *layer )
{

  if ( acceptLayer( layer ) )
  {
    const QList<QgsLayerTreeViewIndicator *> nodeIndicators = mLayerTreeView->indicators( node );

    // maybe the indicator exists already
    for ( QgsLayerTreeViewIndicator *indicator : nodeIndicators )
    {
      if ( mIndicators.contains( indicator ) )
      {
        // Update just in case ...
        indicator->setToolTip( tooltipText( layer ) );
        indicator->setIcon( QgsApplication::getThemeIcon( iconName( layer ) ) );
        return;
      }
    }

    // it does not exist: need to create a new one
    mLayerTreeView->addIndicator( node, newIndicator( layer ).release() );
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

