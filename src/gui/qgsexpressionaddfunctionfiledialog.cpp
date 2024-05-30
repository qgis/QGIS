/***************************************************************************
    qgsexpressionaddfunctionfiledialog.cpp
    ---------------------
    begin                : May 2024
    copyright            : (C) 2024 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QPushButton>
#include <QStandardItemModel>
#include "qgsexpressionaddfunctionfiledialog.h"

QgsExpressionAddFunctionFileDialog::QgsExpressionAddFunctionFileDialog( const bool enableProjectFunctions, QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  cboFileOptions->addItem( QStringLiteral( "Function file" ) );
  cboFileOptions->addItem( QStringLiteral( "Project functions" ), QStringLiteral( "project" ) );

  // Disable project functions (they should be created only once)
  if ( !enableProjectFunctions )
  {
    QStandardItem *item = qobject_cast<QStandardItemModel *>( cboFileOptions->model() )->item( 1 );
    item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
  }

  connect( cboFileOptions, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsExpressionAddFunctionFileDialog::cboFileOptions_currentIndexChanged );
  connect( txtNewFileName, &QLineEdit::textChanged, this, [ = ]( const QString & ) { updateOkButtonStatus(); } );

  updateOkButtonStatus();
}

void QgsExpressionAddFunctionFileDialog::cboFileOptions_currentIndexChanged( int )
{
  bool projectSelected = cboFileOptions->currentData() == "project";
  lblNewFileName->setVisible( !projectSelected );
  txtNewFileName->setVisible( !projectSelected );
  updateOkButtonStatus();
}

void QgsExpressionAddFunctionFileDialog::updateOkButtonStatus()
{
  QPushButton *okBtn = buttonBox->button( QDialogButtonBox::StandardButton::Ok );
  okBtn->setEnabled( true );

  if ( cboFileOptions->currentData() != "project" )
  {
    okBtn->setEnabled( !txtNewFileName->text().trimmed().isEmpty() );
  }
}
