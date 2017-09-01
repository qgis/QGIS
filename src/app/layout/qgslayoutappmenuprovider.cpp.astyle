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
#include "qgslayoutitempage.h"
#include "qgslayoutdesignerdialog.h"
#include "qgslayout.h"
#include <QMenu>
#include <QMessageBox>

QgsLayoutAppMenuProvider::QgsLayoutAppMenuProvider( QgsLayoutDesignerDialog *designer )
  : QObject( nullptr )
  , mDesigner( designer )
{

}

QMenu *QgsLayoutAppMenuProvider::createContextMenu( QWidget *parent, QgsLayout *layout, QPointF layoutPoint ) const
{
  QMenu *menu = new QMenu( parent );

  // is a page under the mouse?
  QgsLayoutItemPage *page = layout->pageCollection()->pageAtPoint( layoutPoint );
  if ( page )
  {
    QAction *pagePropertiesAction = new QAction( tr( "Page Properties…" ), menu );
    connect( pagePropertiesAction, &QAction::triggered, this, [this, page]()
    {
      mDesigner->showItemOptions( page );
    } );
    menu->addAction( pagePropertiesAction );
    QAction *removePageAction = new QAction( tr( "Remove Page" ), menu );
    connect( removePageAction, &QAction::triggered, this, [layout, page]()
    {
      if ( QMessageBox::question( nullptr, tr( "Remove Page" ),
                                  tr( "Remove page from layout?" ),
                                  QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
      {
        layout->pageCollection()->deletePage( page );
      }
    } );
    menu->addAction( removePageAction );
  }

  return menu;
}
