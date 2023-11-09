/***************************************************************************
    qgsexpressionstoredialog.cpp
    ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Alessandro Pasotti
    email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionstoredialog.h"
#include <QPushButton>
#include <QStyle>

QgsExpressionStoreDialog::QgsExpressionStoreDialog( const QString &label, const QString &expression, const QString &helpText, const QStringList &existingLabels, QWidget *parent )
  : QDialog( parent )
  , mExistingLabels( existingLabels )
  , mOriginalLabel( label )
{
  setupUi( this );
  mExpression->setText( expression );
  mHelpText->setText( helpText );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsExpressionStoreDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsExpressionStoreDialog::reject );
  mValidationError->hide();
  mValidationError->setStyleSheet( QStringLiteral( "QLabel { color : red; }" ) );
  QPushButton *saveBtn { buttonBox->button( QDialogButtonBox::StandardButton::Save ) };
  saveBtn->setEnabled( false );
  connect( mLabel, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    QString errorMessage;
    if ( mOriginalLabel.simplified() != text.simplified() &&
         mExistingLabels.contains( text.simplified() ) )
    {
      errorMessage = tr( "A stored expression with this name already exists" );
    }
    else if ( text.contains( '/' ) || text.contains( '\\' ) )
    {
      errorMessage = tr( "Labels cannot contain slashes (/ or \\)" );
    }
    if ( ! errorMessage.isEmpty() )
    {
      mValidationError->show();
      mValidationError->setText( errorMessage );
      saveBtn->setEnabled( false );
    }
    else
    {
      mValidationError->hide();
      saveBtn->setEnabled( true );
    }
  } );
  // No slashes in labels!
  QString labelFixed { label };
  labelFixed.remove( '/' ).remove( '\\' );
  mLabel->setText( labelFixed.simplified() );
}

QString QgsExpressionStoreDialog::helpText() const
{
  // remove meta qrichtext instruction from html. It fails rendering
  // when mixing with other html content
  // see issue https://github.com/qgis/QGIS/issues/36191
  return mHelpText->toHtml().replace( "<meta name=\"qrichtext\" content=\"1\" />", QString() );
}
