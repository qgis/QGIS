/***************************************************************************
                             qgsmodeloutputreorderwidget.cpp
                             ------------------------------------
    Date                 : April 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodeloutputreorderwidget.h"
#include "moc_qgsmodeloutputreorderwidget.cpp"
#include "qgsgui.h"
#include "qgsprocessingmodelalgorithm.h"
#include <QDialogButtonBox>
#include <QStandardItemModel>
///@cond NOT_STABLE

QgsModelOutputReorderWidget::QgsModelOutputReorderWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mItemModel = new QStandardItemModel( 0, 1, this );
  mOutputsList->setModel( mItemModel );

  mOutputsList->setDropIndicatorShown( true );
  mOutputsList->setDragDropOverwriteMode( false );
  mOutputsList->setDragEnabled( true );
  mOutputsList->setDragDropMode( QAbstractItemView::InternalMove );

  connect( mButtonUp, &QPushButton::clicked, this, [=] {
    int currentRow = mOutputsList->currentIndex().row();
    if ( currentRow == 0 )
      return;

    mItemModel->insertRow( currentRow - 1, mItemModel->takeRow( currentRow ) );
    mOutputsList->setCurrentIndex( mItemModel->index( currentRow - 1, 0 ) );
  } );

  connect( mButtonDown, &QPushButton::clicked, this, [=] {
    int currentRow = mOutputsList->currentIndex().row();
    if ( currentRow == mItemModel->rowCount() - 1 )
      return;

    mItemModel->insertRow( currentRow + 1, mItemModel->takeRow( currentRow ) );
    mOutputsList->setCurrentIndex( mItemModel->index( currentRow + 1, 0 ) );
  } );
}

void QgsModelOutputReorderWidget::setModel( QgsProcessingModelAlgorithm *model )
{
  mModel = model;
  mOutputs = mModel->orderedOutputs();
  mItemModel->clear();
  for ( const QgsProcessingModelOutput &output : std::as_const( mOutputs ) )
  {
    QStandardItem *item = new QStandardItem( output.name() );
    item->setData( QStringLiteral( "%1:%2" ).arg( output.childId(), output.childOutputName() ), Qt::UserRole + 1 );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
    // we show the outputs list reversed in the gui, because we want the "higher" outputs to be at the top of the list
    mItemModel->insertRow( 0, item );
  }

  mPlaceInGroupCheck->setChecked( !model->outputGroup().isEmpty() );
  mGroupNameEdit->setText( model->outputGroup() );
}

QStringList QgsModelOutputReorderWidget::outputOrder() const
{
  QStringList order;
  order.reserve( mItemModel->rowCount() );
  // we show the outputs list reversed in the gui, because we want the "higher" outputs to be at the top of the list
  for ( int row = mItemModel->rowCount() - 1; row >= 0; --row )
  {
    order << mItemModel->data( mItemModel->index( row, 0 ), Qt::UserRole + 1 ).toString();
  }
  return order;
}

QString QgsModelOutputReorderWidget::outputGroup() const
{
  return mPlaceInGroupCheck->isChecked() ? mGroupNameEdit->text() : QString();
}


QgsModelOutputReorderDialog::QgsModelOutputReorderDialog( QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( tr( "Reorder Output Layers" ) );
  mWidget = new QgsModelOutputReorderWidget();
  QVBoxLayout *vl = new QVBoxLayout();
  vl->addWidget( mWidget, 1 );
  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  vl->addWidget( buttonBox );
  setLayout( vl );
}

void QgsModelOutputReorderDialog::setModel( QgsProcessingModelAlgorithm *model )
{
  mWidget->setModel( model );
}

QStringList QgsModelOutputReorderDialog::outputOrder() const
{
  return mWidget->outputOrder();
}

QString QgsModelOutputReorderDialog::outputGroup() const
{
  return mWidget->outputGroup();
}

///@endcond
