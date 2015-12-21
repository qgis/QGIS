/***************************************************************************
 qgosorderbydialog.cpp

 ---------------------
 begin                : 20.12.2015
 copyright            : (C) 2015 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsorderbydialog.h"

#include "qgsexpressionbuilderdialog.h"

#include <QTableWidget>
#include <QCheckBox>
#include <QKeyEvent>

QgsOrderByDialog::QgsOrderByDialog( QgsVectorLayer* layer, QWidget* parent )
    : QDialog( parent )
    , mLayer( layer )
{
  setupUi( this );
  connect( mOrderByTableWidget, SIGNAL( cellDoubleClicked( int, int ) ), this, SLOT( onCellDoubleClicked( int, int ) ) );
  connect( mOrderByTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( onCellChanged( int, int ) ) );

  mOrderByTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  mOrderByTableWidget->horizontalHeader()->setResizeMode( 1, QHeaderView::Interactive );
  mOrderByTableWidget->horizontalHeader()->setResizeMode( 2, QHeaderView::Interactive );

  mOrderByTableWidget->installEventFilter( this );
}

void QgsOrderByDialog::setOrderBy( const QgsFeatureRequest::OrderBy& orderBy )
{
  mOrderByTableWidget->clear();
  mOrderByTableWidget->setRowCount( orderBy.length() + 1 );

  int i = 0;
  Q_FOREACH ( const QgsFeatureRequest::OrderByClause& orderByClause, orderBy )
  {
    QTableWidgetItem* expressionItem  = new QTableWidgetItem( orderByClause.expression().expression() );
    QCheckBox* ascCheckBox        = new QCheckBox();
    ascCheckBox->setChecked( orderByClause.ascending() );
    QCheckBox* nullsFirstCheckBox = new QCheckBox();
    nullsFirstCheckBox->setChecked( orderByClause.nullsFirst() );

    mOrderByTableWidget->setItem( i, 0, expressionItem );
    mOrderByTableWidget->setCellWidget( i, 1, ascCheckBox );
    mOrderByTableWidget->setCellWidget( i, 2, nullsFirstCheckBox );

    ++i;
  }

  // Add an empty widget at the end
  QTableWidgetItem* expressionItem  = new QTableWidgetItem( "" );
  QCheckBox* ascCheckBox        = new QCheckBox();
  ascCheckBox->setChecked( true );
  QCheckBox* nullsFirstCheckBox = new QCheckBox();

  mOrderByTableWidget->setItem( i, 0, expressionItem );
  mOrderByTableWidget->setCellWidget( i, 1, ascCheckBox );
  mOrderByTableWidget->setCellWidget( i, 2, nullsFirstCheckBox );
}

QgsFeatureRequest::OrderBy QgsOrderByDialog::orderBy()
{
  QgsFeatureRequest::OrderBy orderBys;

  for ( int i = 0; i < mOrderByTableWidget->rowCount(); ++i )
  {
    QString expressionText = mOrderByTableWidget->item( i, 0 )->text();

    if ( ! expressionText.isEmpty() )
    {
      bool asc = static_cast<QCheckBox*>( mOrderByTableWidget->cellWidget( i, 1 ) )->checkState();
      bool nullsFirst = static_cast<QCheckBox*>( mOrderByTableWidget->cellWidget( i, 2 ) )->checkState();
      QgsFeatureRequest::OrderByClause orderBy( expressionText, asc, nullsFirst );

      orderBys << orderBy;
    }
  }

  return orderBys;
}

void QgsOrderByDialog::onCellDoubleClicked( int row, int column )
{
  // Only act on first cell where the expression text is
  if ( 0 == column )
  {
    QgsExpressionBuilderDialog dlg( mLayer );

    dlg.setExpressionText( mOrderByTableWidget->item( row, column )->text() );

    if ( dlg.exec() )
    {
      QString expressionText = dlg.expressionText();

      mOrderByTableWidget->item( row, column )->setText( expressionText );

      if ( row == mOrderByTableWidget->rowCount() - 1 )
      {
        // Add an empty widget at the end if the last row was edited
        mOrderByTableWidget->insertRow( mOrderByTableWidget->rowCount() );

        QTableWidgetItem* expressionItem  = new QTableWidgetItem( "" );
        QCheckBox* ascCheckBox        = new QCheckBox();
        ascCheckBox->setChecked( true );
        QCheckBox* nullsFirstCheckBox = new QCheckBox();

        mOrderByTableWidget->setItem( row + 1, 0, expressionItem );
        mOrderByTableWidget->setCellWidget( row + 1, 1, ascCheckBox );
        mOrderByTableWidget->setCellWidget( row + 1, 2, nullsFirstCheckBox );
      }
    }
  }
}

void QgsOrderByDialog::onCellChanged( int row, int column )
{
  // If the text was cleared
  if ( mOrderByTableWidget->item( row, column )->text().isEmpty() )
  {
    // If the first column (expression text) and not the last row was edited
    if ( 0 == column && row != mOrderByTableWidget->rowCount() - 1 )
    {
      {
        mOrderByTableWidget->removeRow( row );
      }
    }
  }
  else
  {
    // If it's the last row and an expression was added: add a new empty one
    if ( row == mOrderByTableWidget->rowCount() - 1 && !mOrderByTableWidget->item( row, column )->text().isEmpty() )
    {
      // Add an empty widget at the end if the last row was edited
      mOrderByTableWidget->insertRow( mOrderByTableWidget->rowCount() );

      QTableWidgetItem* expressionItem  = new QTableWidgetItem( "" );
      QCheckBox* ascCheckBox        = new QCheckBox();
      ascCheckBox->setChecked( true );
      QCheckBox* nullsFirstCheckBox = new QCheckBox();

      mOrderByTableWidget->setItem( row + 1, 0, expressionItem );
      mOrderByTableWidget->setCellWidget( row + 1, 1, ascCheckBox );
      mOrderByTableWidget->setCellWidget( row + 1, 2, nullsFirstCheckBox );
    }
  }
}

bool QgsOrderByDialog::eventFilter( QObject* obj, QEvent* e )
{
  Q_UNUSED( obj )
  Q_ASSERT( obj == mOrderByTableWidget );

  if ( e->type() == QEvent::KeyPress )
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>( e );

    if ( keyEvent->key() == Qt::Key_Delete )
    {
      if ( mOrderByTableWidget->currentRow() != mOrderByTableWidget->rowCount() - 1 )
        mOrderByTableWidget->removeRow( mOrderByTableWidget->currentRow() );
      return true;
    }
  }

  return false;
}

