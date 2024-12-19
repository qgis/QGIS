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
#include "qgslayoutitemgroup.h"
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

QModelIndex QgsLayoutItemsListViewModel::indexForItem( QgsLayoutItem *item, const int column ) const
{
  return mapFromSource( mModel->indexForItem( item, column ) );
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

  // Allow multi selection from the list view
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setSelectionBehavior( QAbstractItemView::SelectRows );

  connect( this, &QWidget::customContextMenuRequested, this, &QgsLayoutItemsListView::showContextMenu );
  connect( mDesigner->view(), &QgsLayoutView::itemFocused, this, &QgsLayoutItemsListView::onItemFocused );
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

  connect( selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsLayoutItemsListView::updateSelection );
}

void QgsLayoutItemsListView::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Space )
  {
    const auto constSelectedIndexes = selectionModel()->selectedIndexes();
    if ( !constSelectedIndexes.isEmpty() )
    {
      const bool isFirstItemVisible = mModel->itemFromIndex( constSelectedIndexes[0] )->isVisible();

      for ( const QModelIndex &index : constSelectedIndexes )
      {
        if ( QgsLayoutItem *item = mModel->itemFromIndex( index ) )
        {
          item->setVisibility( !isFirstItemVisible );
        }
      }
      //return here, because otherwise it will just invert visibility for each item instead setting all the same
      return;
    }
  }

  QTreeView::keyPressEvent( event );
}

void QgsLayoutItemsListView::updateSelection()
{
  // Do nothing if we are currently updating the selection
  // because user has selected/deselected some items in the
  // graphics view
  if ( !mModel || mUpdatingFromView )
    return;

  // Set the updating flag
  mUpdatingSelection = true;

  // Deselect all items from the layout (prevent firing signals, selection will be changed)
  whileBlocking( mLayout )->deselectAll();

  // Build the list of selected items
  QList<QgsLayoutItem *> selectedItems;
  for ( const QModelIndex &index : selectionModel()->selectedIndexes() )
  {
    if ( QgsLayoutItem *item = mModel->itemFromIndex( index ) )
    {
      selectedItems << item;
    }
  }


  bool itemSelected = false;

  // Check if the clicked item was selected or deselected
  if ( selectionModel()->isSelected( selectionModel()->currentIndex() ) )
  {
    // It it was selected, set it as the layout's selected item;
    // This will show the item properties
    QgsLayoutItem *currentItem = mModel->itemFromIndex( selectionModel()->currentIndex() );
    mLayout->setSelectedItem( currentItem );
    itemSelected = true;
  }
  for ( QgsLayoutItem *item : selectedItems )
  {
    if ( !itemSelected )
    {
      // If clicked item was actually deselected, set the first selected item in the list
      // as the layout's selected item
      mLayout->setSelectedItem( item );
      itemSelected = true;
    }
    else
    {
      item->setSelected( true );
    }

    // find top level group this item is contained within, and mark the group as selected
    QgsLayoutItemGroup *group = item->parentGroup();
    while ( group && group->parentGroup() )
    {
      group = group->parentGroup();
    }
    if ( group && group != item )
      group->setSelected( true );

  }
  // Reset the updating flag
  mUpdatingSelection = false;
}

void QgsLayoutItemsListView::onItemFocused( QgsLayoutItem *focusedItem )
{
  // Do nothing if we are in the middle of selecting items in the layoutView
  if ( !mModel || mUpdatingSelection )
    return;

  // Set the updating flag
  mUpdatingFromView = true;

  // Deselect all items in list
  clearSelection();

  // Set the current index to the focused item
  QModelIndex index = mModel->indexForItem( focusedItem );
  if ( index.isValid() )
  {
    setCurrentIndex( index );
  }

  // Select rows in the item list for every selected items in the graphics view
  const QList< QgsLayoutItem *> selectedItems = mLayout->selectedLayoutItems();
  for ( QgsLayoutItem *item : selectedItems )
  {
    const QModelIndex firstCol = mModel->indexForItem( item );
    if ( firstCol.isValid() )
    {
      // Select the whole row
      QItemSelection selection;
      selection.select( firstCol, firstCol.siblingAtColumn( mModel->columnCount( firstCol.parent() ) - 1 ) );
      selectionModel()->select( selection, QItemSelectionModel::Select );
    }
  }
  // Reset the updating flag
  mUpdatingFromView = false;
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
