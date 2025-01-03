/***************************************************************************
    qgstiledsceneconnectiondialog.cpp
    ---------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledsceneconnectiondialog.h"
#include "moc_qgstiledsceneconnectiondialog.cpp"
#include "qgstiledsceneconnection.h"
#include "qgsgui.h"
#include <QMessageBox>
#include <QPushButton>

///@cond PRIVATE

QgsTiledSceneConnectionDialog::QgsTiledSceneConnectionDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );

  connect( mEditName, &QLineEdit::textChanged, this, &QgsTiledSceneConnectionDialog::updateOkButtonState );
  connect( mEditUrl, &QLineEdit::textChanged, this, &QgsTiledSceneConnectionDialog::updateOkButtonState );
}

void QgsTiledSceneConnectionDialog::setConnection( const QString &name, const QString &uri )
{
  mEditName->setText( name );

  const QgsTiledSceneProviderConnection::Data conn = QgsTiledSceneProviderConnection::decodedUri( uri );
  mEditUrl->setText( conn.url );

  mAuthSettings->setUsername( conn.username );
  mAuthSettings->setPassword( conn.password );
  mEditReferer->setText( conn.httpHeaders[QgsHttpHeaders::KEY_REFERER].toString() );
  mAuthSettings->setConfigId( conn.authCfg );
}

QString QgsTiledSceneConnectionDialog::connectionUri() const
{
  QgsTiledSceneProviderConnection::Data conn;
  conn.url = mEditUrl->text();

  conn.username = mAuthSettings->username();
  conn.password = mAuthSettings->password();
  conn.httpHeaders[QgsHttpHeaders::KEY_REFERER] = mEditReferer->text();
  conn.authCfg = mAuthSettings->configId();

  return QgsTiledSceneProviderConnection::encodedUri( conn );
}

QString QgsTiledSceneConnectionDialog::connectionName() const
{
  return mEditName->text();
}

void QgsTiledSceneConnectionDialog::updateOkButtonState()
{
  const bool enabled = !mEditName->text().isEmpty() && !mEditUrl->text().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void QgsTiledSceneConnectionDialog::accept()
{
  QDialog::accept();
}

///@endcond
