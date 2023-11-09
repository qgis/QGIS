/***************************************************************************
                             qgsmodelinputreorderwidget.cpp
                             ------------------------------------
    Date                 : April 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelinputreorderwidget.h"
#include "qgsgui.h"
#include "qgsprocessingmodelalgorithm.h"
#include <QDialogButtonBox>
#include <QStandardItemModel>
///@cond NOT_STABLE

QgsModelInputReorderWidget::QgsModelInputReorderWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mItemModel = new QStandardItemModel( 0, 1, this );
  mInputsList->setModel( mItemModel );

  mInputsList->setDropIndicatorShown( true );
  mInputsList->setDragDropOverwriteMode( false );
  mInputsList->setDragEnabled( true );
  mInputsList->setDragDropMode( QAbstractItemView::InternalMove );

  connect( mButtonUp, &QPushButton::clicked, this, [ = ]
  {
    int currentRow = mInputsList->currentIndex().row();
    if ( currentRow == 0 )
      return;

    mItemModel->insertRow( currentRow - 1, mItemModel->takeRow( currentRow ) );
    mInputsList->setCurrentIndex( mItemModel->index( currentRow - 1, 0 ) );
  } );

  connect( mButtonDown, &QPushButton::clicked, this, [ = ]
  {
    int currentRow = mInputsList->currentIndex().row();
    if ( currentRow == mItemModel->rowCount() - 1 )
      return;

    mItemModel->insertRow( currentRow + 1, mItemModel->takeRow( currentRow ) );
    mInputsList->setCurrentIndex( mItemModel->index( currentRow + 1, 0 ) );
  } );

}

void QgsModelInputReorderWidget::setModel( QgsProcessingModelAlgorithm *model )
{
  mModel = model;
  mParameters = mModel->orderedParameters();
  mItemModel->clear();
  for ( const QgsProcessingModelParameter &param : std::as_const( mParameters ) )
  {
    QStandardItem *item = new QStandardItem( mModel->parameterDefinition( param.parameterName() )->description() );
    item->setData( param.parameterName() );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
    mItemModel->appendRow( item );
  }
}

QStringList QgsModelInputReorderWidget::inputOrder() const
{
  QStringList order;
  order.reserve( mItemModel->rowCount( ) );
  for ( int row = 0; row < mItemModel->rowCount(); ++row )
  {
    order << mItemModel->data( mItemModel->index( row, 0 ), Qt::UserRole + 1 ).toString();
  }
  return order;
}


QgsModelInputReorderDialog::QgsModelInputReorderDialog( QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( tr( "Reorder Model Inputs" ) );
  mWidget = new QgsModelInputReorderWidget();
  QVBoxLayout *vl = new QVBoxLayout();
  vl->addWidget( mWidget, 1 );
  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  vl->addWidget( buttonBox );
  setLayout( vl );
}

void QgsModelInputReorderDialog::setModel( QgsProcessingModelAlgorithm *model )
{
  mWidget->setModel( model );
}

QStringList QgsModelInputReorderDialog::inputOrder() const
{
  return mWidget->inputOrder();
}

///@endcond
