/***************************************************************************
  qgslayertreeviewjoinindicator.cpp
  --------------------------------------
  Date                 : October 2018
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

#include "qgslayertreeviewjoinindicator.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeview.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgisapp.h"


QgsLayerTreeViewJoinIndicatorProvider::QgsLayerTreeViewJoinIndicatorProvider( QgsLayerTreeView *view )
  : QObject( view )
  , mLayerTreeView( view )
{
  mIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIndicatorJoin.svg" ) );

  QgsLayerTree *tree = mLayerTreeView->layerTreeModel()->rootGroup();
  onAddedChildren( tree, 0, tree->children().count() - 1 );

  connect( tree, &QgsLayerTree::addedChildren, this, &QgsLayerTreeViewJoinIndicatorProvider::onAddedChildren );
  connect( tree, &QgsLayerTree::willRemoveChildren, this, &QgsLayerTreeViewJoinIndicatorProvider::onWillRemoveChildren );
}

void QgsLayerTreeViewJoinIndicatorProvider::onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
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
          if ( QgsLayerTreeUtils::countMapLayerInTree( mLayerTreeView->layerTreeModel()->rootGroup(), vlayer ) == 1 )
            connect( vlayer, &QgsVectorLayer::updatedFields, this, &QgsLayerTreeViewJoinIndicatorProvider::onUpdatedFields );
          addOrRemoveIndicator( childNode, vlayer );
        }
        else if ( !layerNode->layer() )
        {
          // wait for layer to be loaded (e.g. when loading project, first the tree is loaded, afterwards the references to layers are resolved)
          connect( layerNode, &QgsLayerTreeLayer::layerLoaded, this, &QgsLayerTreeViewJoinIndicatorProvider::onLayerLoaded );
        }
      }
    }
  }
}

void QgsLayerTreeViewJoinIndicatorProvider::onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
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
        if ( QgsLayerTreeUtils::countMapLayerInTree( mLayerTreeView->layerTreeModel()->rootGroup(), childLayerNode->layer() ) == 1 )
          disconnect( vlayer, &QgsVectorLayer::updatedFields, this, &QgsLayerTreeViewJoinIndicatorProvider::onUpdatedFields );
      }
    }
  }
}

void QgsLayerTreeViewJoinIndicatorProvider::onLayerLoaded()
{
  QgsLayerTreeLayer *layerNode = qobject_cast<QgsLayerTreeLayer *>( sender() );
  if ( !layerNode )
    return;

  if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layerNode->layer() ) )
  {
    if ( vlayer )
    {
      connect( vlayer, &QgsVectorLayer::updatedFields, this, &QgsLayerTreeViewJoinIndicatorProvider::onUpdatedFields );
      addOrRemoveIndicator( layerNode, vlayer );
    }
  }
}

void QgsLayerTreeViewJoinIndicatorProvider::onUpdatedFields()
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

std::unique_ptr< QgsLayerTreeViewIndicator > QgsLayerTreeViewJoinIndicatorProvider::newIndicator(const QString &joinsNames)
{
  std::unique_ptr< QgsLayerTreeViewIndicator > indicator = qgis::make_unique< QgsLayerTreeViewIndicator >( this );
  indicator->setIcon( mIcon );
  updateIndicator( indicator.get(), joinsNames );
  mIndicators.insert( indicator.get() );
  return indicator;
}

void QgsLayerTreeViewJoinIndicatorProvider::updateIndicator( QgsLayerTreeViewIndicator *indicator, const QString &joinsNames )
{
  indicator->setToolTip( QStringLiteral( "<b>%1:</b><br>%2" ).arg( tr( "Joined layers" ), joinsNames ) );
}

void QgsLayerTreeViewJoinIndicatorProvider::addOrRemoveIndicator( QgsLayerTreeNode *node, QgsVectorLayer *vlayer )
{
  const bool haveJoins = vlayer->joinBuffer()->containsJoins();

  if ( haveJoins )
  {
 
    QString joinsNames = QString("");
    for ( QgsVectorLayerJoinInfo joinInfo : vlayer->joinBuffer()->vectorJoins() )
    {
      joinsNames += joinInfo.joinLayer()->name() + '\n';
    }
    
    
    const QList<QgsLayerTreeViewIndicator *> nodeIndicators = mLayerTreeView->indicators( node );

    // maybe the indicator exists already
    for ( QgsLayerTreeViewIndicator *indicator : nodeIndicators )
    {
      if ( mIndicators.contains( indicator ) )
      {
        updateIndicator( indicator, joinsNames );
        return;
      }
    }

    // it does not exist: need to create a new one
    mLayerTreeView->addIndicator( node, newIndicator( joinsNames ).release() );
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
