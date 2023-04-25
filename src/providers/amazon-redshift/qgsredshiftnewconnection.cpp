/***************************************************************************
   qgsredshiftnewconnection.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftnewconnection.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QRegExpValidator>

#include "qgsauthmanager.h"
#include "qgsdatasourceuri.h"
#include "qgsgui.h"
#include "qgsredshiftconn.h"
#include "qgssettings.h"

QgsRedshiftNewConnection::QgsRedshiftNewConnection( QWidget *parent, const QString &connName, Qt::WindowFlags fl )
  : QDialog( parent, fl ), mOriginalConnName( connName )

{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( btnConnect, &QPushButton::clicked, this, &QgsRedshiftNewConnection::btnConnect_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsRedshiftNewConnection::showHelp );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( txtName, &QLineEdit::textChanged, this, &QgsRedshiftNewConnection::updateOkButtonState );
  connect( txtHost, &QLineEdit::textChanged, this, &QgsRedshiftNewConnection::updateOkButtonState );
  connect( txtPort, &QLineEdit::textChanged, this, &QgsRedshiftNewConnection::updateOkButtonState );
  connect( txtDatabase, &QLineEdit::textChanged, this, &QgsRedshiftNewConnection::updateOkButtonState );

  cbxSSLmode->addItem( tr( "disable" ), QgsDataSourceUri::SslDisable );
  cbxSSLmode->addItem( tr( "allow" ), QgsDataSourceUri::SslAllow );
  cbxSSLmode->addItem( tr( "prefer" ), QgsDataSourceUri::SslPrefer );
  cbxSSLmode->addItem( tr( "require" ), QgsDataSourceUri::SslRequire );
  cbxSSLmode->addItem( tr( "verify-ca" ), QgsDataSourceUri::SslVerifyCa );
  cbxSSLmode->addItem( tr( "verify-full" ), QgsDataSourceUri::SslVerifyFull );

  mAuthSettings->setDataprovider( QStringLiteral( "redshift" ) );
  mAuthSettings->showStoreCheckboxes( true );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QgsSettings settings;

    QString key = "/Redshift/connections/" + connName;
    txtHost->setText( settings.value( key + "/host" ).toString() );
    QString port = settings.value( key + "/port" ).toString();
    if ( port.length() == 0 )
    {
      port = QStringLiteral( "5439" );
    }
    txtPort->setText( port );
    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    cb_publicSchemaOnly->setChecked( settings.value( key + "/publicOnly", false ).toBool() );
    cb_dontResolveType->setChecked( settings.value( key + "/dontResolveType", false ).toBool() );
    cb_allowGeometrylessTables->setChecked( settings.value( key + "/allowGeometrylessTables", false ).toBool() );

    cb_useEstimatedMetadata->setChecked( settings.value( key + "/estimatedMetadata", false ).toBool() );
    cb_projectsInDatabase->setChecked( settings.value( key + "/projectsInDatabase", false ).toBool() );

    cbxSSLmode->setCurrentIndex(
      cbxSSLmode->findData( settings.enumValue( key + "/sslmode", QgsDataSourceUri::SslPrefer ) ) );

    if ( settings.value( key + "/saveUsername" ).toString() == QLatin1String( "true" ) )
    {
      mAuthSettings->setUsername( settings.value( key + "/username" ).toString() );
      mAuthSettings->setStoreUsernameChecked( true );
    }

    if ( settings.value( key + "/savePassword" ).toString() == QLatin1String( "true" ) )
    {
      mAuthSettings->setPassword( settings.value( key + "/password" ).toString() );
      mAuthSettings->setStorePasswordChecked( true );
    }

    QString authcfg = settings.value( key + "/authcfg" ).toString();
    mAuthSettings->setConfigId( authcfg );

    txtName->setText( connName );
  }
  txtName->setValidator( new QRegExpValidator( QRegExp( "[^\\/]*" ), txtName ) );
}

//! Autoconnected SLOTS
void QgsRedshiftNewConnection::accept()
{
  QgsSettings settings;
  QString baseKey = QStringLiteral( "/Redshift/connections/" );
  settings.setValue( baseKey + "selected", txtName->text() );
  bool hasAuthConfigID = !mAuthSettings->configId().isEmpty();
  testConnection();

  if ( !hasAuthConfigID && mAuthSettings->storePasswordIsChecked() &&
       QMessageBox::question( this, tr( "Saving Passwords" ),
                              tr( "WARNING: You have opted to save your password. It will be stored "
                                  "in unsecured plain text in your project files and in your home "
                                  "directory (Unix-like OS) or user profile (Windows). If you want "
                                  "to avoid this, press Cancel and either:\n\na) Don't save a "
                                  "password in the connection settings — it will be requested "
                                  "interactively when needed;\nb) Use the Configuration tab to add "
                                  "your credentials in an HTTP Basic Authentication method and "
                                  "store them in an encrypted database." ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( txtName->text(), Qt::CaseInsensitive ) != 0 ) &&
       settings.contains( baseKey + txtName->text() + "/host" ) &&
       QMessageBox::question( this, tr( "Save Connection" ),
                              tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // on rename delete the original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != txtName->text() )
  {
    settings.remove( baseKey + mOriginalConnName );
    settings.sync();
  }

  baseKey += txtName->text();
  settings.setValue( baseKey + "/host", txtHost->text() );
  settings.setValue( baseKey + "/port", txtPort->text() );
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/username",
                     mAuthSettings->storeUsernameIsChecked() ? mAuthSettings->username() : QString() );
  settings.setValue( baseKey + "/password", mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID
                     ? mAuthSettings->password()
                     : QString() );
  settings.setValue( baseKey + "/authcfg", mAuthSettings->configId() );
  settings.setValue( baseKey + "/publicOnly", cb_publicSchemaOnly->isChecked() );
  settings.setValue( baseKey + "/dontResolveType", cb_dontResolveType->isChecked() );
  settings.setValue( baseKey + "/allowGeometrylessTables", cb_allowGeometrylessTables->isChecked() );
  settings.setValue( baseKey + "/sslmode", cbxSSLmode->currentData().toInt() );
  settings.setValue( baseKey + "/saveUsername", mAuthSettings->storeUsernameIsChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/savePassword",
                     mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID ? "true" : "false" );
  settings.setValue( baseKey + "/estimatedMetadata", cb_useEstimatedMetadata->isChecked() );
  settings.setValue( baseKey + "/projectsInDatabase", cb_projectsInDatabase->isChecked() );

  QDialog::accept();
}

void QgsRedshiftNewConnection::btnConnect_clicked()
{
  testConnection();
}

//! End  Autoconnected SLOTS

void QgsRedshiftNewConnection::testConnection()
{
  QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );

  QgsDataSourceUri uri;

  uri.setConnection( txtHost->text(), txtPort->text(), txtDatabase->text(), mAuthSettings->username(),
                     mAuthSettings->password(), ( QgsDataSourceUri::SslMode )cbxSSLmode->currentData().toInt(),
                     mAuthSettings->configId() );

  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( uri.connectionInfo( false ), true );

  if ( conn )
  {
    // Database successfully opened; we can now issue SQL commands.
    bar->pushMessage( tr( "Connection to %1 was successful." ).arg( txtName->text() ), Qgis::Info );

    // free redshift connection resources
    conn->unref();
  }
  else
  {
    bar->pushMessage( tr( "Connection failed - consult message log for details." ), Qgis::Warning );
  }
}

void QgsRedshiftNewConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#creating-a-stored-connection" ) );
}

void QgsRedshiftNewConnection::updateOkButtonState()
{
  bool enabled = !txtName->text().isEmpty() && !txtDatabase->text().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}
