/***************************************************************************
                             qgslayoutappmenuprovider.cpp
                             ---------------------------
    Date                 : July 2017
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

#include "qgslayoutappmenuprovider.h"

#include "qgslayout.h"
#include "qgslayoutdesignerdialog.h"
#include "qgslayoutguidewidget.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutitempage.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"

#include <QMenu>
#include <QMessageBox>

#include "moc_qgslayoutappmenuprovider.cpp"

QgsLayoutAppMenuProvider::QgsLayoutAppMenuProvider( QgsLayoutDesignerDialog *designer )
  : QObject( nullptr )
  , mDesigner( designer )
{
}

QMenu *QgsLayoutAppMenuProvider::createContextMenu( QWidget *parent, QgsLayout *layout, QPointF layoutPoint ) const
{
  QMenu *menu = new QMenu( parent );

  //undo/redo
  menu->addAction( layout->undoStack()->stack()->createUndoAction( menu ) );
  menu->addAction( layout->undoStack()->stack()->createRedoAction( menu ) );
  menu->addSeparator()->setObjectName( "UndoRedoSeparator"_L1 );


  const QList<QgsLayoutItem *> selectedItems = layout->selectedLayoutItems();
  if ( !selectedItems.empty() )
  {
    bool addedGroupAction = false;
    if ( selectedItems.count() > 1 )
    {
      QAction *groupAction = new QAction( tr( "Group" ), menu );
      connect( groupAction, &QAction::triggered, this, [this]() {
        mDesigner->view()->groupSelectedItems();
      } );
      menu->addAction( groupAction );
      addedGroupAction = true;
    }
    bool foundSelectedGroup = false;
    QList<QgsLayoutItemGroup *> groups;
    layout->layoutItems( groups );
    for ( QgsLayoutItemGroup *group : std::as_const( groups ) )
    {
      if ( group->isSelected() )
      {
        foundSelectedGroup = true;
        break;
      }
    }
    if ( foundSelectedGroup )
    {
      QAction *ungroupAction = new QAction( tr( "Ungroup" ), menu );
      connect( ungroupAction, &QAction::triggered, this, [this]() {
        mDesigner->view()->ungroupSelectedItems();
      } );
      menu->addAction( ungroupAction );
      addedGroupAction = true;
    }

    if ( addedGroupAction )
      menu->addSeparator()->setObjectName( "AddedGroupSeparator"_L1 );

    QAction *copyAction = new QAction( tr( "Copy" ), menu );
    connect( copyAction, &QAction::triggered, this, [this]() {
      mDesigner->view()->copySelectedItems( QgsLayoutView::ClipboardCopy );
    } );
    menu->addAction( copyAction );
    QAction *cutAction = new QAction( tr( "Cut" ), menu );
    connect( cutAction, &QAction::triggered, this, [this]() {
      mDesigner->view()->copySelectedItems( QgsLayoutView::ClipboardCut );
    } );
    menu->addAction( cutAction );
    menu->addSeparator()->setObjectName( "CopyCutSeparator"_L1 );
  }
  else if ( mDesigner->view()->hasItemsInClipboard() )
  {
    QAction *pasteAction = new QAction( tr( "Paste" ), menu );
    connect( pasteAction, &QAction::triggered, this, [this, menu]() {
      QPointF pt = mDesigner->view()->mapToScene( mDesigner->view()->mapFromGlobal( menu->pos() ) );
      mDesigner->view()->pasteItems( pt );
    } );
    menu->addAction( pasteAction );
    menu->addSeparator()->setObjectName( "PasteSeparator"_L1 );
  }

  // is a page under the mouse?
  QgsLayoutItemPage *page = layout->pageCollection()->pageAtPoint( layoutPoint );
  if ( page )
  {
    const int pageNumber = layout->pageCollection()->pageNumber( page );
    QAction *pagePropertiesAction = new QAction( tr( "Page Properties…" ), menu );
    connect( pagePropertiesAction, &QAction::triggered, this, [this, page]() {
      mDesigner->showItemOptions( page, true );
    } );
    menu->addAction( pagePropertiesAction );

    if ( mDesigner->guideWidget() )
    {
      QAction *manageGuidesAction = new QAction( tr( "Manage Guides for Page…" ), menu );
      QPointer<QgsLayoutGuideWidget> guideManager( mDesigner->guideWidget() );
      connect( manageGuidesAction, &QAction::triggered, this, [this, pageNumber, guideManager = std::move( guideManager )]() {
        if ( guideManager )
        {
          guideManager->setCurrentPage( pageNumber );
          mDesigner->showGuideDock( true );
        }
      } );
      menu->addAction( manageGuidesAction );
    }
    QAction *removePageAction = new QAction( tr( "Remove Page" ), menu );
    connect( removePageAction, &QAction::triggered, this, [layout, page]() {
      if ( QMessageBox::question( nullptr, tr( "Remove Page" ), tr( "Remove page from layout?" ), QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
      {
        layout->pageCollection()->deletePage( page );
      }
    } );
    if ( layout->pageCollection()->pageCount() < 2 )
      removePageAction->setEnabled( false );
    menu->addAction( removePageAction );

    menu->addSeparator()->setObjectName( "ManagePageSeparator"_L1 );
  }

  if ( !selectedItems.empty() )
  {
    QAction *itemPropertiesAction = new QAction( tr( "Item Properties…" ), menu );
    QgsLayoutItem *item = selectedItems.at( 0 );
    connect( itemPropertiesAction, &QAction::triggered, this, [this, item]() {
      mDesigner->showItemOptions( item, true );
    } );
    menu->addAction( itemPropertiesAction );
  }

  return menu;
}
