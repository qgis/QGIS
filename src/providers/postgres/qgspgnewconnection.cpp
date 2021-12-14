/***************************************************************************
                    qgspgnewconnection.cpp  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>
#include <QInputDialog>
#include <QRegularExpressionValidator>
#include <QRegularExpression>

#include "qgspgnewconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgspostgresproviderconnection.h"
#include "qgsauthmanager.h"
#include "qgsdatasourceuri.h"
#include "qgspostgresconn.h"
#include "qgssettings.h"
#include "qgsgui.h"

QgsPgNewConnection::QgsPgNewConnection( QWidget *parent, const QString &connName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mOriginalConnName( connName )

{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( btnConnect, &QPushButton::clicked, this, &QgsPgNewConnection::btnConnect_clicked );
  connect( cb_geometryColumnsOnly, &QCheckBox::clicked, this, &QgsPgNewConnection::cb_geometryColumnsOnly_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsPgNewConnection::showHelp );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( txtName, &QLineEdit::textChanged, this, &QgsPgNewConnection::updateOkButtonState );
  connect( txtService, &QLineEdit::textChanged, this, &QgsPgNewConnection::updateOkButtonState );
  connect( txtHost, &QLineEdit::textChanged, this, &QgsPgNewConnection::updateOkButtonState );
  connect( txtPort, &QLineEdit::textChanged, this, &QgsPgNewConnection::updateOkButtonState );
  connect( txtDatabase, &QLineEdit::textChanged, this, &QgsPgNewConnection::updateOkButtonState );

  cbxSSLmode->addItem( tr( "disable" ), QgsDataSourceUri::SslDisable );
  cbxSSLmode->addItem( tr( "allow" ), QgsDataSourceUri::SslAllow );
  cbxSSLmode->addItem( tr( "prefer" ), QgsDataSourceUri::SslPrefer );
  cbxSSLmode->addItem( tr( "require" ), QgsDataSourceUri::SslRequire );
  cbxSSLmode->addItem( tr( "verify-ca" ), QgsDataSourceUri::SslVerifyCa );
  cbxSSLmode->addItem( tr( "verify-full" ), QgsDataSourceUri::SslVerifyFull );

  mAuthSettings->setDataprovider( QStringLiteral( "postgres" ) );
  mAuthSettings->showStoreCheckboxes( true );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QgsSettings settings;

    QString key = "/PostgreSQL/connections/" + connName;
    txtService->setText( settings.value( key + "/service" ).toString() );
    txtHost->setText( settings.value( key + "/host" ).toString() );
    QString port = settings.value( key + "/port" ).toString();
    if ( port.length() == 0 )
    {
      port = QStringLiteral( "5432" );
    }
    txtPort->setText( port );
    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    cb_publicSchemaOnly->setChecked( settings.value( key + "/publicOnly", false ).toBool() );
    cb_geometryColumnsOnly->setChecked( settings.value( key + "/geometryColumnsOnly", true ).toBool() );
    cb_dontResolveType->setChecked( settings.value( key + "/dontResolveType", false ).toBool() );
    cb_allowGeometrylessTables->setChecked( settings.value( key + "/allowGeometrylessTables", false ).toBool() );
    // Ensure that cb_publicSchemaOnly is set correctly
    cb_geometryColumnsOnly_clicked();

    cb_useEstimatedMetadata->setChecked( settings.value( key + "/estimatedMetadata", false ).toBool() );
    cb_projectsInDatabase->setChecked( settings.value( key + "/projectsInDatabase", false ).toBool() );

    cbxSSLmode->setCurrentIndex( cbxSSLmode->findData( settings.enumValue( key + "/sslmode", QgsDataSourceUri::SslPrefer ) ) );

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
void QgsPgNewConnection::accept()
{
  QgsSettings settings;
  QString baseKey = QStringLiteral( "/PostgreSQL/connections/" );
  settings.setValue( baseKey + "selected", txtName->text() );
  bool hasAuthConfigID = !mAuthSettings->configId().isEmpty();
  testConnection();

  if ( !hasAuthConfigID && mAuthSettings->storePasswordIsChecked( ) &&
       QMessageBox::question( this,
                              tr( "Saving Passwords" ),
                              tr( "WARNING: You have opted to save your password. It will be stored in unsecured plain text in your project files and in your home directory (Unix-like OS) or user profile (Windows). If you want to avoid this, press Cancel and either:\n\na) Don't save a password in the connection settings — it will be requested interactively when needed;\nb) Use the Configuration tab to add your credentials in an HTTP Basic Authentication method and store them in an encrypted database." ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( txtName->text(), Qt::CaseInsensitive ) != 0 ) &&
       ( settings.contains( baseKey + txtName->text() + "/service" ) ||
         settings.contains( baseKey + txtName->text() + "/host" ) ) &&
       QMessageBox::question( this,
                              tr( "Save Connection" ),
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
  settings.setValue( baseKey + "/service", txtService->text() );
  settings.setValue( baseKey + "/host", txtHost->text() );
  settings.setValue( baseKey + "/port", txtPort->text() );
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/username", mAuthSettings->storeUsernameIsChecked( ) ? mAuthSettings->username() : QString() );
  settings.setValue( baseKey + "/password", mAuthSettings->storePasswordIsChecked( ) && !hasAuthConfigID ? mAuthSettings->password() : QString() );
  settings.setValue( baseKey + "/authcfg", mAuthSettings->configId() );
  settings.setValue( baseKey + "/publicOnly", cb_publicSchemaOnly->isChecked() );
  settings.setValue( baseKey + "/geometryColumnsOnly", cb_geometryColumnsOnly->isChecked() );
  settings.setValue( baseKey + "/dontResolveType", cb_dontResolveType->isChecked() );
  settings.setValue( baseKey + "/allowGeometrylessTables", cb_allowGeometrylessTables->isChecked() );
  settings.setValue( baseKey + "/sslmode", cbxSSLmode->currentData().toInt() );
  settings.setValue( baseKey + "/saveUsername", mAuthSettings->storeUsernameIsChecked( ) ? "true" : "false" );
  settings.setValue( baseKey + "/savePassword", mAuthSettings->storePasswordIsChecked( ) && !hasAuthConfigID ? "true" : "false" );
  settings.setValue( baseKey + "/estimatedMetadata", cb_useEstimatedMetadata->isChecked() );
  settings.setValue( baseKey + "/projectsInDatabase", cb_projectsInDatabase->isChecked() );

  // remove old save setting
  settings.remove( baseKey + "/save" );

  QVariantMap configuration;
  configuration.insert( "publicOnly", cb_publicSchemaOnly->isChecked() );
  configuration.insert( "geometryColumnsOnly", cb_geometryColumnsOnly->isChecked() );
  configuration.insert( "dontResolveType", cb_dontResolveType->isChecked() );
  configuration.insert( "allowGeometrylessTables", cb_allowGeometrylessTables->isChecked() );
  configuration.insert( "sslmode", cbxSSLmode->currentData().toInt() );
  configuration.insert( "saveUsername", mAuthSettings->storeUsernameIsChecked( ) ? "true" : "false" );
  configuration.insert( "savePassword", mAuthSettings->storePasswordIsChecked( ) && !hasAuthConfigID ? "true" : "false" );
  configuration.insert( "estimatedMetadata", cb_useEstimatedMetadata->isChecked() );
  configuration.insert( "projectsInDatabase", cb_projectsInDatabase->isChecked() );

  QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) );
  std::unique_ptr< QgsPostgresProviderConnection > providerConnection( qgis::down_cast<QgsPostgresProviderConnection *>( providerMetadata->createConnection( txtName->text() ) ) );
  providerConnection->setUri( QgsPostgresConn::connUri( txtName->text() ).uri( false ) );
  providerConnection->setConfiguration( configuration );
  providerMetadata->saveConnection( providerConnection.get(), txtName->text() );

  QDialog::accept();
}

void QgsPgNewConnection::btnConnect_clicked()
{
  testConnection();
}

void QgsPgNewConnection::cb_geometryColumnsOnly_clicked()
{
  if ( cb_geometryColumnsOnly->checkState() == Qt::Checked )
    cb_publicSchemaOnly->setEnabled( false );
  else
    cb_publicSchemaOnly->setEnabled( true );
}

//! End  Autoconnected SLOTS

void QgsPgNewConnection::testConnection()
{
  QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );

  QgsDataSourceUri uri;
  if ( !txtService->text().isEmpty() )
  {
    uri.setConnection( txtService->text(), txtDatabase->text(),
                       mAuthSettings->username(), mAuthSettings->password(),
                       ( QgsDataSourceUri::SslMode ) cbxSSLmode->currentData().toInt(),
                       mAuthSettings->configId() );
  }
  else
  {
    uri.setConnection( txtHost->text(), txtPort->text(), txtDatabase->text(),
                       mAuthSettings->username(), mAuthSettings->password(),
                       ( QgsDataSourceUri::SslMode ) cbxSSLmode->currentData().toInt(),
                       mAuthSettings->configId() );
  }

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), true );

  if ( conn )
  {
    if ( conn->pgVersion() < 90500 )
    {
      cb_projectsInDatabase->setEnabled( false );
      cb_projectsInDatabase->setChecked( false );
      cb_projectsInDatabase->setToolTip( tr( "Saving projects in databases not available for PostgreSQL databases earlier than 9.5" ) );
    }
    else
    {
      cb_projectsInDatabase->setEnabled( true );
      cb_projectsInDatabase->setToolTip( QString() );
    }

    // Database successfully opened; we can now issue SQL commands.
    bar->pushMessage( tr( "Connection to %1 was successful." ).arg( txtName->text() ),
                      Qgis::MessageLevel::Info );

    // free pg connection resources
    conn->unref();
  }
  else
  {
    bar->pushMessage( tr( "Connection failed - consult message log for details." ),
                      Qgis::MessageLevel::Warning );
  }
}

void QgsPgNewConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#creating-a-stored-connection" ) );
}

void QgsPgNewConnection::updateOkButtonState()
{
  bool enabled = !txtName->text().isEmpty() && (
                   !txtService->text().isEmpty() ||
                   !txtDatabase->text().isEmpty() );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}
