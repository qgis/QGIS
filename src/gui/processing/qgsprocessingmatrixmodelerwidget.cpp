/***************************************************************************
                             qgsprocessingmatrixmodelerwidget.cpp
                             ------------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmatrixmodelerwidget.h"
#include "moc_qgsprocessingmatrixmodelerwidget.cpp"
#include "qgsgui.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QToolButton>

///@cond NOT_STABLE

QgsProcessingMatrixModelerWidget::QgsProcessingMatrixModelerWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mModel = new QStandardItemModel( this );
  mModel->appendColumn( QList<QStandardItem *>() << new QStandardItem( '0' ) );
  mTableView->setModel( mModel );

  connect( mButtonAddColumn, &QToolButton::clicked, this, &QgsProcessingMatrixModelerWidget::addColumn );
  connect( mButtonRemoveColumn, &QToolButton::clicked, this, &QgsProcessingMatrixModelerWidget::removeColumns );
  connect( mButtonAddRow, &QToolButton::clicked, this, &QgsProcessingMatrixModelerWidget::addRow );
  connect( mButtonRemoveRow, &QToolButton::clicked, this, &QgsProcessingMatrixModelerWidget::removeRows );
  connect( mButtonClear, &QToolButton::clicked, this, &QgsProcessingMatrixModelerWidget::clearTable );
  connect( mTableView->horizontalHeader(), &QHeaderView::sectionDoubleClicked, this, &QgsProcessingMatrixModelerWidget::changeHeader );
}

void QgsProcessingMatrixModelerWidget::addColumn()
{
  QList<QStandardItem *> items;
  for ( int i = 0; i < mModel->rowCount(); ++i )
    items << new QStandardItem( '0' );

  mModel->appendColumn( items );
}

void QgsProcessingMatrixModelerWidget::removeColumns()
{
  QModelIndexList selected = mTableView->selectionModel()->selectedColumns();
  std::sort( selected.begin(), selected.end(), []( const QModelIndex &a, const QModelIndex &b ) { return b < a; } );

  mTableView->setUpdatesEnabled( false );
  for ( QModelIndex i : std::as_const( selected ) )
    mModel->removeColumns( i.column(), 1 );

  mTableView->setUpdatesEnabled( true );
}

void QgsProcessingMatrixModelerWidget::addRow()
{
  QList<QStandardItem *> items;
  for ( int i = 0; i < mModel->columnCount(); ++i )
    items << new QStandardItem( '0' );

  mModel->appendRow( items );
}

void QgsProcessingMatrixModelerWidget::removeRows()
{
  QModelIndexList selected = mTableView->selectionModel()->selectedRows();
  std::sort( selected.begin(), selected.end(), []( const QModelIndex &a, const QModelIndex &b ) { return b < a; } );

  mTableView->setUpdatesEnabled( false );
  for ( QModelIndex i : std::as_const( selected ) )
    mModel->removeRows( i.row(), 1 );

  mTableView->setUpdatesEnabled( true );
}

void QgsProcessingMatrixModelerWidget::clearTable()
{
  if ( QMessageBox::question( nullptr, tr( "Clear table" ), tr( "Are you sure you want to clear table?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
    mModel->clear();
}

void QgsProcessingMatrixModelerWidget::changeHeader( int index )
{
  bool ok;
  QString text = QInputDialog::getText( nullptr, tr( "Enter column name" ), tr( "Column name" ), QLineEdit::Normal, QString(), &ok );
  if ( ok && !text.isEmpty() )
    mModel->setHeaderData( index, Qt::Horizontal, text );
}

QStringList QgsProcessingMatrixModelerWidget::headers() const
{
  QStringList headers;
  for ( int i = 0; i < mModel->columnCount(); ++i )
  {
    headers << mModel->headerData( i, Qt::Horizontal ).toString();
  }
  return headers;
}

QVariant QgsProcessingMatrixModelerWidget::value() const
{
  QVariantList defaults;
  const int cols = mModel->columnCount();
  const int rows = mModel->rowCount();

  for ( int row = 0; row < rows; ++row )
  {
    for ( int col = 0; col < cols; ++col )
    {
      defaults << mModel->item( row, col )->text();
    }
  }

  QVariant val( defaults );
  return val;
}

void QgsProcessingMatrixModelerWidget::setValue( const QStringList &headers, const QVariant &defaultValue )
{
  QVariantList contents = defaultValue.toList();

  const int cols = headers.count();
  const int rows = contents.count() / cols;

  mModel->setRowCount( rows );
  mModel->setColumnCount( cols );
  mModel->setHorizontalHeaderLabels( headers );

  for ( int row = 0; row < rows; ++row )
  {
    for ( int col = 0; col < cols; ++col )
    {
      QStandardItem *item = new QStandardItem( contents.at( row * cols + col ).toString() );
      mModel->setItem( row, col, item );
    }
  }
  mTableView->setModel( mModel );
}

bool QgsProcessingMatrixModelerWidget::fixedRows() const
{
  return mFixedRows->isChecked();
}

void QgsProcessingMatrixModelerWidget::setFixedRows( bool fixedRows )
{
  mFixedRows->setChecked( fixedRows );
}

///@endcond
