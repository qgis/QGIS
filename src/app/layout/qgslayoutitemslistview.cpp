/***************************************************************************
                             qgslayoutitemslistview.cpp
                             --------------------------
    Date                 : October 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitemslistview.h"
#include "qgslayout.h"
#include "qgslayoutmodel.h"
#include "qgslayoutdesignerdialog.h"
#include <QHeaderView>

QgsLayoutItemsListView::QgsLayoutItemsListView( QWidget *parent, QgsLayoutDesignerDialog *designer )
  : QTreeView( parent )
  , mDesigner( designer )
{
  setColumnWidth( 0, 30 );
  setColumnWidth( 1, 30 );
  setDragEnabled( true );
  setAcceptDrops( true );
  setDropIndicatorShown( true );
  setDragDropMode( QAbstractItemView::InternalMove );
  setContextMenuPolicy( Qt::CustomContextMenu );
  setIndentation( 0 );
  connect( this, &QWidget::customContextMenuRequested, this, &QgsLayoutItemsListView::showContextMenu );
}

void QgsLayoutItemsListView::setCurrentLayout( QgsLayout *layout )
{
  mLayout = layout;
  mModel = layout->itemsModel();
  setModel( mModel );

  header()->setSectionResizeMode( 0, QHeaderView::Fixed );
  header()->setSectionResizeMode( 1, QHeaderView::Fixed );
  setColumnWidth( 0, Qgis::UI_SCALE_FACTOR * fontMetrics().width( QStringLiteral( "xxxx" ) ) );
  setColumnWidth( 1, Qgis::UI_SCALE_FACTOR * fontMetrics().width( QStringLiteral( "xxxx" ) ) );
  header()->setSectionsMovable( false );

  connect( selectionModel(), &QItemSelectionModel::currentChanged, mLayout->itemsModel(), &QgsLayoutModel::setSelected );
}

void QgsLayoutItemsListView::showContextMenu( QPoint point )
{
  QModelIndex index = indexAt( point );
  QgsLayoutItem *item = mModel->itemFromIndex( index );
  if ( !item )
    return;

  QMenu *menu = new QMenu( this );

  QAction *itemPropertiesAction = new QAction( tr( "Item Properties…" ), menu );
  connect( itemPropertiesAction, &QAction::triggered, this, [this, item]()
  {
    mDesigner->showItemOptions( item, true );
  } );
  menu->addAction( itemPropertiesAction );

  menu->popup( mapToGlobal( point ) );
}
