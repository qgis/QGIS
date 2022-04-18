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
#include "qgselevationprofilelayertreemodel.h"
#include "qgslayertreenode.h"
#include "qgslayertree.h"

#include <QHeaderView>

QgsElevationProfileLayerTreeView::QgsElevationProfileLayerTreeView( QgsLayerTree *rootNode, QWidget *parent )
  : QTreeView( parent )
  , mLayerTree( rootNode )
{
  mModel = new QgsElevationProfileLayerTreeModel( rootNode, this );
  mProxyModel = new QgsElevationProfileLayerTreeProxyModel( mModel, this );

  setHeaderHidden( true );

  setDragEnabled( true );
  setAcceptDrops( true );
  setDropIndicatorShown( true );
  setExpandsOnDoubleClick( false );

  // Ensure legend graphics are scrollable
  header()->setStretchLastSection( false );
  header()->setSectionResizeMode( QHeaderView::ResizeToContents );

  // If vertically scrolling by item, legend graphics can get clipped
  setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

  setDefaultDropAction( Qt::MoveAction );

  setModel( mProxyModel );
}

QgsMapLayer *QgsElevationProfileLayerTreeView::indexToLayer( const QModelIndex &index )
{
  if ( QgsLayerTreeNode *node = mModel->index2node( mProxyModel->mapToSource( index ) ) )
  {
    if ( QgsLayerTreeLayer *layerTreeLayerNode =  mLayerTree->toLayer( node ) )
    {
      return layerTreeLayerNode->layer();
    }
  }
  return nullptr;
}
