/***************************************************************************
  qgssettingstreewidget.h
  --------------------------------------
  Date                 : April 2023
  Copyright            : (C) 2023 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssettingstreewidget.h"
#include "qgssettingstreemodel.h"
#include "qgssettingstree.h"

#include <QAction>
#include <QTreeView>
#include <QHBoxLayout>
#include <QVBoxLayout>


QgsSettingsTreeWidget::QgsSettingsTreeWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setContentsMargins( 0, 0, 0, 0 );

  mTreeModel = new QgsSettingsTreeModel( QgsSettingsTree::treeRoot() );

  mTreeView = new QTreeView( this );
  mTreeView->setModel( mTreeModel );
  mTreeView->setItemDelegate( new QgsSettingsTreeItemDelegate( mTreeModel, this ) );
  mTreeView->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
  mTreeView->setMinimumWidth( 400 );
  mTreeView->resizeColumnToContents( 0 );

  mainLayout->addWidget( mTreeView );
}

void QgsSettingsTreeWidget::applyChanges() const
{
  mTreeModel->applyChanges();
}

