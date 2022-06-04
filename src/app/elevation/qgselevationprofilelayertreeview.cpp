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
#include "qgisapp.h"

#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>

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

void QgsElevationProfileLayerTreeView::contextMenuEvent( QContextMenuEvent *event )
{
  const QModelIndex index = indexAt( event->pos() );
  if ( !index.isValid() )
    setCurrentIndex( QModelIndex() );

  if ( QgsMapLayer *layer = indexToLayer( index ) )
  {
    QMenu *menu = new QMenu();

    QAction *propertiesAction = new QAction( tr( "Properties…" ), menu );
    connect( propertiesAction, &QAction::triggered, this, [layer]
    {
      QgisApp::instance()->showLayerProperties( layer, QStringLiteral( "mOptsPage_Elevation" ) );
    } );
    menu->addAction( propertiesAction );

    menu->exec( mapToGlobal( event->pos() ) );
    delete menu;
  }
}

void QgsElevationProfileLayerTreeView::resizeEvent( QResizeEvent *event )
{
  header()->setMinimumSectionSize( viewport()->width() );
  QTreeView::resizeEvent( event );
}
