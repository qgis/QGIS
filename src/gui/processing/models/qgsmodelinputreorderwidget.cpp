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
#include <QDialogButtonBox>
#include <QStandardItemModel>
///@cond NOT_STABLE

QgsModelInputReorderWidget::QgsModelInputReorderWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mModel = new QStandardItemModel( 0, 1, this );
  mInputsList->setModel( mModel );

  mInputsList->setDropIndicatorShown( true );
  mInputsList->setDragDropOverwriteMode( false );
  mInputsList->setDragEnabled( true );
  mInputsList->setDragDropMode( QAbstractItemView::InternalMove );

  connect( mButtonUp, &QPushButton::clicked, this, [ = ]
  {
    int currentRow = mInputsList->currentIndex().row();
    if ( currentRow == 0 )
      return;

    mModel->insertRow( currentRow - 1, mModel->takeRow( currentRow ) );
    mInputsList->setCurrentIndex( mModel->index( currentRow - 1, 0 ) );
  } );

  connect( mButtonDown, &QPushButton::clicked, this, [ = ]
  {
    int currentRow = mInputsList->currentIndex().row();
    if ( currentRow == mModel->rowCount() - 1 )
      return;

    mModel->insertRow( currentRow + 1, mModel->takeRow( currentRow ) );
    mInputsList->setCurrentIndex( mModel->index( currentRow + 1, 0 ) );
  } );

}

void QgsModelInputReorderWidget::setInputs( const QList<QgsProcessingModelParameter> &inputs )
{
  mParameters = inputs;
  QStringList res;
  mModel->clear();
  for ( const QgsProcessingModelParameter &param : inputs )
  {
    QStandardItem *item = new QStandardItem( param.description() );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
    mModel->appendRow( item );
  }
}

QStringList QgsModelInputReorderWidget::inputOrder() const
{
  QStringList order;
  order.reserve( mModel->rowCount( ) );
  for ( int row = 0; row < mModel->rowCount(); ++row )
  {
    order << mModel->data( mModel->index( row, 0 ) ).toString();
  }
  QStringList res;
  for ( const QString &description : order )
  {
    for ( auto it = mParameters.constBegin(); it != mParameters.constEnd(); ++it )
    {
      if ( it->description() == description )
      {
        res << it->parameterName();
      }
    }
  }
  return res;
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

void QgsModelInputReorderDialog::setInputs( const QList<QgsProcessingModelParameter> &inputs )
{
  mWidget->setInputs( inputs );
}

QStringList QgsModelInputReorderDialog::inputOrder() const
{
  return mWidget->inputOrder();
}

///@endcond
