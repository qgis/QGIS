/***************************************************************************
    qgstablewidgetbase.cpp
     --------------------------------------
    Date                 : 08.2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi@camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstablewidgetbase.h"

QgsTableWidgetBase::QgsTableWidgetBase( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  connect( addButton, &QToolButton::clicked, this, &QgsTableWidgetBase::addButton_clicked );
  connect( removeButton, &QToolButton::clicked, this, &QgsTableWidgetBase::removeButton_clicked );
}

void QgsTableWidgetBase::init( QAbstractTableModel *model )
{
  tableView->setModel( model );
  connect( tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsTableWidgetBase::onSelectionChanged );
  connect( model, &QAbstractItemModel::dataChanged, this, &QgsTableWidgetBase::valueChanged );
  connect( model, &QAbstractItemModel::rowsRemoved, this, &QgsTableWidgetBase::valueChanged );
  connect( model, &QAbstractItemModel::rowsInserted, this, &QgsTableWidgetBase::valueChanged );
}

void QgsTableWidgetBase::addButton_clicked()
{
  if ( mReadOnly )
    return;

  const QItemSelectionModel *select = tableView->selectionModel();
  const int pos = select->hasSelection() ? select->selectedRows()[0].row() : 0;
  QAbstractItemModel *model = tableView->model();
  model->insertRows( pos, 1 );
  const QModelIndex index = model->index( pos, 0 );
  tableView->scrollTo( index );
  tableView->edit( index );
  tableView->selectRow( pos );
}

void QgsTableWidgetBase::removeButton_clicked()
{
  if ( mReadOnly )
    return;

  const QItemSelectionModel *select = tableView->selectionModel();
  // The UI is configured to have single row selection.
  if ( select->hasSelection() )
  {
    tableView->model()->removeRows( select->selectedRows()[0].row(), 1 );
  }
}

void QgsTableWidgetBase::onSelectionChanged()
{
  removeButton->setEnabled( tableView->selectionModel()->hasSelection() );
}

void QgsTableWidgetBase::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;

  addButton->setEnabled( !mReadOnly );
  removeButton->setEnabled( !mReadOnly && tableView->selectionModel()->hasSelection() );

  if ( mReadOnly )
  {
    mWidgetActions->hide();
    layout()->setSpacing( 0 );
  }
  else
  {
    mWidgetActions->show();
    layout()->setSpacing( 6 );
  }
}
