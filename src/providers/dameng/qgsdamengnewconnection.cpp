/***************************************************************************
    qgsdamengnewconnection.cpp  -  description
             -------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>
#include <QInputDialog>
#include <QRegularExpressionValidator>
#include <QRegularExpression>

#include "qgsdamengnewconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsdamengproviderconnection.h"
#include "qgsauthmanager.h"
#include "qgsdatasourceuri.h"
#include "qgsdamengconn.h"
#include "qgssettings.h"
#include "qgsgui.h"

QgsDamengNewConnection::QgsDamengNewConnection( QWidget *parent, const QString &connName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mOriginalConnName( connName )

{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( btnConnect, &QPushButton::clicked, this, &QgsDamengNewConnection::btnConnect_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDamengNewConnection::showHelp );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( txtName, &QLineEdit::textChanged, this, &QgsDamengNewConnection::updateOkButtonState );
  connect( txtHost, &QLineEdit::textChanged, this, &QgsDamengNewConnection::updateOkButtonState );
  connect( txtPort, &QLineEdit::textChanged, this, &QgsDamengNewConnection::updateOkButtonState );


  mAuthSettings->setDataprovider( QStringLiteral( "dameng" ) );
  mAuthSettings->showStoreCheckboxes( true );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QgsSettings settings;

    QString key = "/Dameng/connections/" + connName;
    txtHost->setText( settings.value( key + "/host" ).toString() );
    QString port = settings.value( key + "/port" ).toString();
    if ( port.length() == 0 )
    {
      port = QStringLiteral( "5236" );
    }
    txtPort->setText( port );
    cb_sysdbaSchemaOnly->setChecked( settings.value( key + "/sysdbaOnly", false ).toBool() );
    cb_dontResolveType->setChecked( settings.value( key + "/dontResolveType", false ).toBool() );
    cb_allowGeometrylessTables->setChecked( settings.value( key + "/allowGeometrylessTables", false ).toBool() );

    cb_useEstimatedMetadata->setChecked( settings.value( key + "/estimatedMetadata", false ).toBool() );
    cb_projectsInDatabase->setChecked( settings.value( key + "/projectsInDatabase", false ).toBool() );

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

    // Old save setting
    if ( settings.contains( key + "/save" ) )
    {
      mAuthSettings->setUsername( settings.value( key + "/username" ).toString() );
      mAuthSettings->setStoreUsernameChecked( !mAuthSettings->username().isEmpty() );

      if ( settings.value( key + "/save" ).toString() == QLatin1String( "true" ) )
        mAuthSettings->setPassword( settings.value( key + "/password" ).toString() );

      mAuthSettings->setStorePasswordChecked( true );
    }

    QString authcfg = settings.value( key + "/authcfg" ).toString();
    mAuthSettings->setConfigId( authcfg );

    txtName->setText( connName );
  }
  txtName->setValidator( new QRegularExpressionValidator( QRegularExpression( "[^\\/]*" ), txtName ) );
}

//! Autoconnected SLOTS
void QgsDamengNewConnection::accept()
{
  QgsSettings settings;
  QString baseKey = QStringLiteral( "/Dameng/connections/" );
  settings.setValue( baseKey + "selected", txtName->text() );
  bool hasAuthConfigID = !mAuthSettings->configId().isEmpty();
  testConnection();

  if ( !hasAuthConfigID && mAuthSettings->storePasswordIsChecked() && QMessageBox::question( this, tr( "Saving Passwords" ), tr( "WARNING: You have opted to save your password. It will be stored in unsecured plain text in your project files and in your home directory ( Unix-like OS ) or user profile ( Windows ). If you want to avoid this, press Cancel and either:\n\na ) Don't save a password in the connection settings — it will be requested interactively when needed;\nb ) Use the Configuration tab to add your credentials in an HTTP Basic Authentication method and store them in an encrypted database." ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( txtName->text(), Qt::CaseInsensitive ) != 0 ) && ( settings.contains( baseKey + txtName->text() + "/host" ) ) && QMessageBox::question( this, tr( "Save Connection" ), tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
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
  settings.setValue( baseKey + "/username", mAuthSettings->storeUsernameIsChecked() ? mAuthSettings->username() : QString() );
  settings.setValue( baseKey + "/password", mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID ? mAuthSettings->password() : QString() );
  settings.setValue( baseKey + "/authcfg", mAuthSettings->configId() );
  settings.setValue( baseKey + "/saveUsername", mAuthSettings->storeUsernameIsChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/savePassword", mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID ? "true" : "false" );

  // remove old save setting
  settings.remove( baseKey + "/save" );

  QVariantMap configuration;
  configuration.insert( "sysdbaOnly", cb_sysdbaSchemaOnly->isChecked() );
  configuration.insert( "dontResolveType", cb_dontResolveType->isChecked() );
  configuration.insert( "allowGeometrylessTables", cb_allowGeometrylessTables->isChecked() );
  configuration.insert( "saveUsername", mAuthSettings->storeUsernameIsChecked() ? "true" : "false" );
  configuration.insert( "savePassword", mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID ? "true" : "false" );
  configuration.insert( "estimatedMetadata", cb_useEstimatedMetadata->isChecked() );
  configuration.insert( "projectsInDatabase", cb_projectsInDatabase->isChecked() );

  QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "dameng" ) );
  std::unique_ptr<QgsDamengProviderConnection> providerConnection( qgis::down_cast<QgsDamengProviderConnection *>( providerMetadata->createConnection( txtName->text() ) ) );
  providerConnection->setUri( QgsDamengConn::connUri( txtName->text() ).uri( false ) );
  providerConnection->setConfiguration( configuration );
  providerMetadata->saveConnection( providerConnection.get(), txtName->text() );

  QDialog::accept();
}

void QgsDamengNewConnection::btnConnect_clicked()
{
  testConnection();
}

//! End  Autoconnected SLOTS

void QgsDamengNewConnection::testConnection()
{
  QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );

  QgsDataSourceUri uri;
  uri.setConnection( txtHost->text(), txtPort->text(), "dameng", mAuthSettings->username(), mAuthSettings->password(), QgsDataSourceUri::SslPrefer /* meaningless for dameng */, mAuthSettings->configId() );

  QgsDamengConn *conn = QgsDamengConn::connectDb( uri.connectionInfo( false ), true );

  if ( conn )
  {
    cb_projectsInDatabase->setEnabled( true );
    cb_projectsInDatabase->setToolTip( QString() );

    // Database successfully opened; we can now issue SQL commands.
    bar->pushMessage( tr( "Connection to %1 was successful." ).arg( txtName->text() ), Qgis::MessageLevel::Success );

    // free dm connection resources
    conn->unref();
  }
  else
  {
    bar->pushMessage( tr( "Connection failed - consult message log for details." ), Qgis::MessageLevel::Warning );
  }
}

void QgsDamengNewConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#creating-a-stored-connection" ) );
}

void QgsDamengNewConnection::updateOkButtonState()
{
  bool enabled = !txtName->text().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}
