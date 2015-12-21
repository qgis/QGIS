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
#include "qgsfieldexpressionwidget.h"

#include <QTableWidget>
#include <QCheckBox>
#include <QKeyEvent>

QgsOrderByDialog::QgsOrderByDialog( QgsVectorLayer* layer, QWidget* parent )
    : QDialog( parent )
    , mLayer( layer )
{
  setupUi( this );

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
    setRow( i, orderByClause );

    ++i;
  }

  // Add an empty widget at the end
  setRow( i, QgsFeatureRequest::OrderByClause( "" ) );
}

QgsFeatureRequest::OrderBy QgsOrderByDialog::orderBy()
{
  QgsFeatureRequest::OrderBy orderBy;

  for ( int i = 0; i < mOrderByTableWidget->rowCount(); ++i )
  {
    QString expressionText = static_cast<QgsFieldExpressionWidget*>( mOrderByTableWidget->cellWidget( i, 0 ) )->currentText();

    if ( ! expressionText.isEmpty() )
    {
      bool asc = static_cast<QCheckBox*>( mOrderByTableWidget->cellWidget( i, 1 ) )->checkState();
      bool nullsFirst = static_cast<QCheckBox*>( mOrderByTableWidget->cellWidget( i, 2 ) )->checkState();
      QgsFeatureRequest::OrderByClause orderByClause( expressionText, asc, nullsFirst );

      orderBy << orderByClause;
    }
  }

  return orderBy;
}

void QgsOrderByDialog::onExpressionChanged( const QString& expression )
{
  // The sender() is the field widget which is the cell widget of the first column
  int row;
  for ( row = 0; row < mOrderByTableWidget->rowCount(); ++row )
  {
    if ( mOrderByTableWidget->cellWidget( row, 0 ) == sender() )
    {
      break;
    }
  }

  if ( expression.isEmpty() && row != mOrderByTableWidget->rowCount() - 1 )
  {
    mOrderByTableWidget->removeRow( row );
  }
  else if ( !expression.isEmpty() && row == mOrderByTableWidget->rowCount() - 1 )
  {
    mOrderByTableWidget->insertRow( mOrderByTableWidget->rowCount() );
    setRow( row + 1, QgsFeatureRequest::OrderByClause( "" ) );
  }
}

void QgsOrderByDialog::setRow( int row, const QgsFeatureRequest::OrderByClause& orderByClause )
{
  QgsFieldExpressionWidget* fieldExpression = new QgsFieldExpressionWidget();
  fieldExpression->setLayer( mLayer );
  fieldExpression->setField( orderByClause.expression().expression() );
  connect( fieldExpression, SIGNAL( fieldChanged( QString ) ), this, SLOT( onExpressionChanged( QString ) ) );
  QCheckBox* ascCheckBox        = new QCheckBox();
  ascCheckBox->setChecked( orderByClause.ascending() );
  QCheckBox* nullsFirstCheckBox = new QCheckBox();
  nullsFirstCheckBox->setChecked( orderByClause.nullsFirst() );

  mOrderByTableWidget->setCellWidget( row, 0, fieldExpression );
  mOrderByTableWidget->setCellWidget( row, 1, ascCheckBox );
  mOrderByTableWidget->setCellWidget( row, 2, nullsFirstCheckBox );
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

