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
#include <QStringListModel>
///@cond NOT_STABLE

QgsModelInputReorderWidget::QgsModelInputReorderWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mModel = new QStringListModel( this );
  mInputsList->setModel( mModel );

  connect( mButtonUp, &QPushButton::clicked, this, [ = ]
  {
    int currentRow = mInputsList->currentIndex().row() - 1;
    if ( currentRow == -1 )
      return;

    mModel->moveRow( QModelIndex(), currentRow, QModelIndex(), currentRow + 2 );
  } );

  connect( mButtonDown, &QPushButton::clicked, this, [ = ]
  {
    int currentRow = mInputsList->currentIndex().row();
    if ( currentRow == mModel->rowCount() - 1 )
      return;

    mModel->moveRow( QModelIndex(), currentRow, QModelIndex(), currentRow + 2 );
  } );

}

void QgsModelInputReorderWidget::setInputs( const QList<QgsProcessingModelParameter> &inputs )
{
  mParameters = inputs;
  QStringList res;
  for ( const QgsProcessingModelParameter &param : inputs )
    res << param.description();
  mModel->setStringList( res );
}

QStringList QgsModelInputReorderWidget::inputOrder() const
{
  const QStringList order = mModel->stringList();
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
