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
#include "qgslayoutdesignerinterface.h"
#include "qgslayoutview.h"
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>


QgsLayoutItemsListViewModel::QgsLayoutItemsListViewModel( QgsLayoutModel *model, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mModel( model )
{
  setSourceModel( mModel );
}

QgsLayoutItem *QgsLayoutItemsListViewModel::itemFromIndex( const QModelIndex &index ) const
{
  return mModel->itemFromIndex( mapToSource( index ) );
}

void QgsLayoutItemsListViewModel::setSelected( const QModelIndex &index )
{
  mModel->setSelected( mapToSource( index ) );
}

bool QgsLayoutItemsListViewModel::filterAcceptsRow( int sourceRow, const QModelIndex & ) const
{
  if ( sourceRow == 0 )
    return false; // hide empty null item row
  return true;
}

QVariant QgsLayoutItemsListViewModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsLayoutItem *item = itemFromIndex( index );
  if ( !item )
  {
    return QVariant();
  }

  if ( role == Qt::FontRole )
  {
    if ( index.column() == QgsLayoutModel::ItemId && item->isSelected() )
    {
      //draw name of selected items in bold
      QFont boldFont;
      boldFont.setBold( true );
      return boldFont;
    }
  }

  return QSortFilterProxyModel::data( index, role );
}


//
// QgsLayoutItemsListView
//

QgsLayoutItemsListView::QgsLayoutItemsListView( QWidget *parent, QgsLayoutDesignerInterface *designer )
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
  mModel = new QgsLayoutItemsListViewModel( layout->itemsModel(), this );
  setModel( mModel );

  header()->setSectionResizeMode( 0, QHeaderView::Fixed );
  header()->setSectionResizeMode( 1, QHeaderView::Fixed );
  setColumnWidth( 0, Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'x' ) * 4 );
  setColumnWidth( 1, Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'x' ) * 4 );
  header()->setSectionsMovable( false );

  connect( selectionModel(), &QItemSelectionModel::currentChanged, mModel, &QgsLayoutItemsListViewModel::setSelected );
}

void QgsLayoutItemsListView::showContextMenu( QPoint point )
{
  if ( !mModel )
    return;
  const QModelIndex index = indexAt( point );
  QgsLayoutItem *item = mModel->itemFromIndex( index );
  if ( !item )
    return;

  QMenu *menu = new QMenu( this );

  QAction *copyAction = new QAction( tr( "Copy Item" ), menu );
  connect( copyAction, &QAction::triggered, this, [this, item]()
  {
    mDesigner->view()->copyItems( QList< QgsLayoutItem * >() << item, QgsLayoutView::ClipboardCopy );
  } );
  menu->addAction( copyAction );
  QAction *deleteAction = new QAction( tr( "Delete Item" ), menu );
  connect( deleteAction, &QAction::triggered, this, [this, item]()
  {
    mDesigner->view()->deleteItems( QList< QgsLayoutItem * >() << item );
  } );
  menu->addAction( deleteAction );
  menu->addSeparator();

  QAction *itemPropertiesAction = new QAction( tr( "Item Properties…" ), menu );
  connect( itemPropertiesAction, &QAction::triggered, this, [this, item]()
  {
    mDesigner->showItemOptions( item, true );
  } );
  menu->addAction( itemPropertiesAction );

  menu->popup( mapToGlobal( point ) );
}
