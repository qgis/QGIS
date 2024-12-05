/***************************************************************************
                             qgsprocessingmatrixparameterdialog.cpp
                             ------------------------------------
    Date                 : February 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmatrixparameterdialog.h"
#include "moc_qgsprocessingmatrixparameterdialog.cpp"
#include "qgsgui.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPushButton>
#include <QLineEdit>
#include <QToolButton>

///@cond NOT_STABLE

QgsProcessingMatrixParameterPanelWidget::QgsProcessingMatrixParameterPanelWidget( QWidget *parent, const QgsProcessingParameterMatrix *param, const QVariantList &initialTable )
  : QgsPanelWidget( parent )
  , mParam( param )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  mTblView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mTblView->setSelectionMode( QAbstractItemView::ExtendedSelection );

  mButtonAdd = new QPushButton( tr( "Add Row" ) );
  mButtonBox->addButton( mButtonAdd, QDialogButtonBox::ActionRole );

  mButtonRemove = new QPushButton( tr( "Remove Row(s)" ) );
  mButtonBox->addButton( mButtonRemove, QDialogButtonBox::ActionRole );

  mButtonRemoveAll = new QPushButton( tr( "Remove All" ) );
  mButtonBox->addButton( mButtonRemoveAll, QDialogButtonBox::ActionRole );

  connect( mButtonBox->button( QDialogButtonBox::Ok ), &QPushButton::clicked, this, &QgsPanelWidget::acceptPanel );
  connect( mButtonBox->button( QDialogButtonBox::Cancel ), &QPushButton::clicked, this, [=] {
    mWasCanceled = true;
    acceptPanel();
  } );
  connect( this, &QgsPanelWidget::panelAccepted, this, [=]() {
    // save any current cell edits
    mTblView->setCurrentIndex( QModelIndex() );

    if ( !mWasCanceled )
      emit widgetChanged();
  } );

  connect( mButtonAdd, &QPushButton::clicked, this, &QgsProcessingMatrixParameterPanelWidget::addRow );
  connect( mButtonRemove, &QPushButton::clicked, this, &QgsProcessingMatrixParameterPanelWidget::deleteRow );
  connect( mButtonRemoveAll, &QPushButton::clicked, this, &QgsProcessingMatrixParameterPanelWidget::deleteAllRows );

  if ( param && param->hasFixedNumberRows() )
  {
    mButtonAdd->setEnabled( false );
    mButtonRemove->setEnabled( false );
    mButtonRemoveAll->setEnabled( false );
  }

  populateTable( initialTable );
}

QVariantList QgsProcessingMatrixParameterPanelWidget::table() const
{
  const int cols = mModel->columnCount();
  const int rows = mModel->rowCount();
  // Table MUST BE 1-dimensional to match core QgsProcessingParameterMatrix expectations
  QVariantList res;
  res.reserve( cols * rows );
  for ( int row = 0; row < rows; ++row )
  {
    for ( int col = 0; col < cols; ++col )
    {
      res << mModel->item( row, col )->text();
    }
  }
  return res;
}

void QgsProcessingMatrixParameterPanelWidget::addRow()
{
  QList<QStandardItem *> items;
  for ( int i = 0; i < mTblView->model()->columnCount(); ++i )
  {
    items << new QStandardItem( '0' );
  }
  mModel->appendRow( items );
}

void QgsProcessingMatrixParameterPanelWidget::deleteRow()
{
  QModelIndexList selected = mTblView->selectionModel()->selectedRows();
  QSet<int> rows;
  rows.reserve( selected.count() );
  for ( const QModelIndex &i : selected )
    rows << i.row();

  QList<int> rowsToDelete = qgis::setToList( rows );
  std::sort( rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>() );
  mTblView->setUpdatesEnabled( false );
  for ( int i : std::as_const( rowsToDelete ) )
    mModel->removeRows( i, 1 );

  mTblView->setUpdatesEnabled( true );
}

void QgsProcessingMatrixParameterPanelWidget::deleteAllRows()
{
  mModel->clear();
  if ( mParam )
    mModel->setHorizontalHeaderLabels( mParam->headers() );
}

void QgsProcessingMatrixParameterPanelWidget::populateTable( const QVariantList &contents )
{
  if ( !mParam )
    return;

  const int cols = mParam->headers().count();
  const int rows = contents.length() / cols;
  mModel = new QStandardItemModel( rows, cols, this );
  mModel->setHorizontalHeaderLabels( mParam->headers() );

  for ( int row = 0; row < rows; ++row )
  {
    for ( int col = 0; col < cols; ++col )
    {
      QStandardItem *item = new QStandardItem( contents.at( row * cols + col ).toString() );
      mModel->setItem( row, col, item );
    }
  }
  mTblView->setModel( mModel );
}

//
// QgsProcessingMatrixParameterPanel
//

QgsProcessingMatrixParameterPanel::QgsProcessingMatrixParameterPanel( QWidget *parent, const QgsProcessingParameterMatrix *param )
  : QWidget( parent )
  , mParam( param )
{
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  mLineEdit->setEnabled( false );
  hl->addWidget( mLineEdit, 1 );

  mToolButton = new QToolButton();
  mToolButton->setText( QString( QChar( 0x2026 ) ) );
  hl->addWidget( mToolButton );

  setLayout( hl );

  if ( mParam )
  {
    for ( int row = 0; row < mParam->numberRows(); ++row )
    {
      for ( int col = 0; col < mParam->headers().count(); ++col )
      {
        mTable.append( '0' );
      }
    }
    mLineEdit->setText( tr( "Fixed table (%1x%2)" ).arg( mParam->numberRows() ).arg( mParam->headers().count() ) );
  }

  connect( mToolButton, &QToolButton::clicked, this, &QgsProcessingMatrixParameterPanel::showDialog );
}

void QgsProcessingMatrixParameterPanel::setValue( const QVariantList &value )
{
  mTable = value;
  updateSummaryText();
  emit changed();
}

void QgsProcessingMatrixParameterPanel::showDialog()
{
  if ( QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this ) )
  {
    QgsProcessingMatrixParameterPanelWidget *widget = new QgsProcessingMatrixParameterPanelWidget( this, mParam, mTable );

    widget->setPanelTitle( mParam->description() );

    panel->openPanel( widget );

    connect( widget, &QgsPanelWidget::widgetChanged, this, [=] {
      setValue( widget->table() );
    } );
  }
}

void QgsProcessingMatrixParameterPanel::updateSummaryText()
{
  if ( mParam )
    mLineEdit->setText( tr( "Fixed table (%1x%2)" ).arg( mTable.count() / mParam->headers().count() ).arg( mParam->headers().count() ) );
}


///@endcond
