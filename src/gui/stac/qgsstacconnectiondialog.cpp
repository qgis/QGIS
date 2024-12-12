/***************************************************************************
    qgsstacconnectiondialog.h
    ---------------------
    begin                : September 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacconnectiondialog.h"
#include "moc_qgsstacconnectiondialog.cpp"
#include "qgsstacconnection.h"
#include "qgsgui.h"
#include <QMessageBox>
#include <QPushButton>

///@cond PRIVATE

QgsStacConnectionDialog::QgsStacConnectionDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );

  connect( mEditName, &QLineEdit::textChanged, this, &QgsStacConnectionDialog::updateOkButtonState );
  connect( mEditUrl, &QLineEdit::textChanged, this, &QgsStacConnectionDialog::updateOkButtonState );
}

void QgsStacConnectionDialog::setConnection( const QString &name, const QString &uri )
{
  mEditName->setText( name );

  const QgsStacConnection::Data conn = QgsStacConnection::decodedUri( uri );
  mEditUrl->setText( conn.url );

  mAuthSettings->setUsername( conn.username );
  mAuthSettings->setPassword( conn.password );
  mEditReferer->setText( conn.httpHeaders[QgsHttpHeaders::KEY_REFERER].toString() );
  mAuthSettings->setConfigId( conn.authCfg );
}

QString QgsStacConnectionDialog::connectionUri() const
{
  QgsStacConnection::Data conn;
  conn.url = mEditUrl->text();

  conn.username = mAuthSettings->username();
  conn.password = mAuthSettings->password();
  conn.httpHeaders[QgsHttpHeaders::KEY_REFERER] = mEditReferer->text();
  conn.authCfg = mAuthSettings->configId();

  return QgsStacConnection::encodedUri( conn );
}

QString QgsStacConnectionDialog::connectionName() const
{
  return mEditName->text();
}

void QgsStacConnectionDialog::updateOkButtonState()
{
  const bool enabled = !mEditName->text().isEmpty() && !mEditUrl->text().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void QgsStacConnectionDialog::accept()
{
  QDialog::accept();
}

///@endcond
