/***************************************************************************
    qgsgdalcloudconnectiondialog.cpp
    ---------------------
    Date                 : June 2024
    Copyright            : (C) 2024 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalcloudconnectiondialog.h"
#include "moc_qgsgdalcloudconnectiondialog.cpp"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsgdalcredentialoptionswidget.h"
#include "qgsgdalcloudconnection.h"

#include <QPushButton>
#include <QMessageBox>

///@cond PRIVATE

QgsGdalCloudConnectionDialog::QgsGdalCloudConnectionDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mCredentialsWidget = new QgsGdalCredentialOptionsWidget();
  QHBoxLayout *hlayout = new QHBoxLayout();
  hlayout->addWidget( mCredentialsWidget );
  mCredentialsGroupBox->setLayout( hlayout );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, [=] {
    QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html" ) );
  } );
  connect( mEditName, &QLineEdit::textChanged, this, &QgsGdalCloudConnectionDialog::updateOkButtonState );
  connect( mBucket, &QLineEdit::textChanged, this, &QgsGdalCloudConnectionDialog::updateOkButtonState );
}

void QgsGdalCloudConnectionDialog::setVsiHandler( const QString &handler )
{
  mHandler = handler;
  mCredentialsWidget->setHandler( mHandler );
}

void QgsGdalCloudConnectionDialog::setConnection( const QString &name, const QString &uri )
{
  mEditName->setText( name );

  const QgsGdalCloudProviderConnection::Data conn = QgsGdalCloudProviderConnection::decodedUri( uri );
  setVsiHandler( conn.vsiHandler );

  mBucket->setText( conn.container );
  mKey->setText( conn.rootPath );

  mCredentialsWidget->setCredentialOptions( conn.credentialOptions );
}

QString QgsGdalCloudConnectionDialog::connectionUri() const
{
  QgsGdalCloudProviderConnection::Data conn;
  conn.vsiHandler = mHandler;
  conn.container = mBucket->text();
  conn.rootPath = mKey->text();
  conn.credentialOptions = mCredentialsWidget->credentialOptions();

  return QgsGdalCloudProviderConnection::encodedUri( conn );
}

QString QgsGdalCloudConnectionDialog::connectionName() const
{
  return mEditName->text();
}

void QgsGdalCloudConnectionDialog::updateOkButtonState()
{
  const bool enabled = !mEditName->text().isEmpty()
                       && !mBucket->text().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void QgsGdalCloudConnectionDialog::accept()
{
  // validate here if required
  QDialog::accept();
}

///@endcond PRIVATE
