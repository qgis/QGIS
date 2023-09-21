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
#include "qgssqlconnectionconfigurator.h"

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

    txtService->setText( QgsPostgreSqlConnectionSettings::sService->value( connName ) );
    txtHost->setText( QgsPostgreSqlConnectionSettings::sHost->value( connName ) );
    QString port = QgsPostgreSqlConnectionSettings::sPort->value( connName );
    if ( port.length() == 0 )
    {
      port = QgsPostgreSqlConnectionSettings::mConnectionTypePort;
    }
    txtPort->setText( port );
    txtDatabase->setText( QgsPostgreSqlConnectionSettings::sDatabase->value( connName ) );
    txtSessionRole->setText( QgsPostgreSqlConnectionSettings::sSessionRole->value( connName ) );
    cb_publicSchemaOnly->setChecked( QgsPostgreSqlConnectionSettings::sPublicOnly->value( connName ) );
    cb_geometryColumnsOnly->setChecked( QgsPostgreSqlConnectionSettings::sGeometryColumnsOnly->value( connName ) );
    cb_dontResolveType->setChecked( QgsPostgreSqlConnectionSettings::sDontResolveType->value( connName ) );
    cb_allowGeometrylessTables->setChecked( QgsPostgreSqlConnectionSettings::sAllowGeometrylessTables->value( connName ) );
    // Ensure that cb_publicSchemaOnly is set correctly
    cb_geometryColumnsOnly_clicked();

    cb_useEstimatedMetadata->setChecked( QgsPostgreSqlConnectionSettings::sEstimatedMetadata->value( connName ) );
    cb_projectsInDatabase->setChecked( QgsPostgreSqlConnectionSettings::sProjectsInDatabase->value( connName ) );
    cb_metadataInDatabase->setChecked( QgsPostgreSqlConnectionSettings::sMetadataInDatabase->value( connName ) );

    cbxSSLmode->setCurrentIndex( cbxSSLmode->findData( QgsPostgreSqlConnectionSettings::sSslMode->value( connName ) ) );

    if ( QgsPostgreSqlConnectionSettings::sOldSave->value( connName ) )
    {
      mAuthSettings->setUsername( QgsPostgreSqlConnectionSettings::sUsername->value( connName ) );
      mAuthSettings->setStoreUsernameChecked( true );
    }

    if ( QgsPostgreSqlConnectionSettings::sOldSave->value( connName ) )
    {
      mAuthSettings->setPassword( QgsPostgreSqlConnectionSettings::sPassword->value( connName ) );
      mAuthSettings->setStorePasswordChecked( true );
    }

    // Old save setting
    if ( QgsPostgreSqlConnectionSettings::sOldSave->value( connName ) )
    {
      mAuthSettings->setUsername( QgsPostgreSqlConnectionSettings::sUsername->value( connName ) );
      mAuthSettings->setStoreUsernameChecked( !mAuthSettings->username().isEmpty() );

      if ( QgsPostgreSqlConnectionSettings::sOldSave->value( connName ) )
        mAuthSettings->setPassword( QgsPostgreSqlConnectionSettings::sPassword->value( connName ) );

      mAuthSettings->setStorePasswordChecked( true );
    }

    const QString authcfg = QgsPostgreSqlConnectionSettings::sAuthCfg->value( connName );
    mAuthSettings->setConfigId( authcfg );

    txtName->setText( connName );
  }
  txtName->setValidator( new QRegularExpressionValidator( QRegularExpression( "[^\\/]*" ), txtName ) );
}

//! Autoconnected SLOTS
void QgsPgNewConnection::accept()
{
  const QString currentConnection = txtName->text();
  QgsPostgresConn::setSelectedConnection( currentConnection );
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
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( currentConnection, Qt::CaseInsensitive ) != 0 ) &&
       ( QgsPostgreSqlConnectionSettings::sService->exists( currentConnection ) ||
         QgsPostgreSqlConnectionSettings::sHost->exists( currentConnection ) ) &&
       QMessageBox::question( this,
                              tr( "Save Connection" ),
                              tr( "Should the existing connection %1 be overwritten?" ).arg( currentConnection ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // on rename delete the original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != currentConnection )
  {
    QgsPostgresConn::deleteConnection( mOriginalConnName );
  }

  QgsPostgreSqlConnectionSettings::sService->setValue( txtService->text(), currentConnection );
  QgsPostgreSqlConnectionSettings::sHost->setValue( txtHost->text(), currentConnection );
  QgsPostgreSqlConnectionSettings::sPort->setValue( txtPort->text(), currentConnection );
  QgsPostgreSqlConnectionSettings::sDatabase->setValue( txtDatabase->text(), currentConnection );
  QgsPostgreSqlConnectionSettings::sSessionRole->setValue( txtSessionRole->text(), currentConnection );
  QgsPostgreSqlConnectionSettings::sUsername->setValue( mAuthSettings->storeUsernameIsChecked( ) ? mAuthSettings->username() : QString(), currentConnection );
  QgsPostgreSqlConnectionSettings::sPassword->setValue( mAuthSettings->storePasswordIsChecked( ) && !hasAuthConfigID ? mAuthSettings->password() : QString(), currentConnection );
  QgsPostgreSqlConnectionSettings::sAuthCfg->setValue( mAuthSettings->configId(), currentConnection );
  QgsPostgreSqlConnectionSettings::sPublicOnly->setValue( cb_publicSchemaOnly->isChecked(), currentConnection );
  QgsPostgreSqlConnectionSettings::sGeometryColumnsOnly->setValue( cb_geometryColumnsOnly->isChecked(), currentConnection );
  QgsPostgreSqlConnectionSettings::sDontResolveType->setValue( cb_dontResolveType->isChecked(), currentConnection );
  QgsPostgreSqlConnectionSettings::sAllowGeometrylessTables->setValue( cb_allowGeometrylessTables->isChecked(), currentConnection );
  QgsPostgreSqlConnectionSettings::sSslMode->setValue( ( QgsDataSourceUri::SslMode )cbxSSLmode->currentData().toInt(), currentConnection );
  QgsPostgreSqlConnectionSettings::sSaveUsername->setValue( mAuthSettings->storeUsernameIsChecked( ), currentConnection );
  QgsPostgreSqlConnectionSettings::sSavePassword->setValue( mAuthSettings->storePasswordIsChecked( ) && !hasAuthConfigID ? true : false, currentConnection );
  QgsPostgreSqlConnectionSettings::sEstimatedMetadata->setValue( cb_useEstimatedMetadata->isChecked(), currentConnection );
  QgsPostgreSqlConnectionSettings::sProjectsInDatabase->setValue( cb_projectsInDatabase->isChecked(), currentConnection );
  QgsPostgreSqlConnectionSettings::sMetadataInDatabase->setValue( cb_metadataInDatabase->isChecked(), currentConnection );

  // remove old save setting
  QgsPostgreSqlConnectionSettings::sOldSave->remove( currentConnection );

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
  configuration.insert( "metadataInDatabase", cb_metadataInDatabase->isChecked() );


  QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) );
  std::unique_ptr< QgsPostgresProviderConnection > providerConnection( qgis::down_cast<QgsPostgresProviderConnection *>( providerMetadata->createConnection( currentConnection ) ) );
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

  if ( !txtSessionRole->text().isEmpty() )
  {
    uri.setParam( QStringLiteral( "session_role" ), txtSessionRole->text() );
  }

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri, true );

  if ( conn )
  {
    if ( conn->pgVersion() < 90500 )
    {
      cb_projectsInDatabase->setEnabled( false );
      cb_projectsInDatabase->setChecked( false );
      cb_projectsInDatabase->setToolTip( tr( "Saving projects in databases not available for PostgreSQL databases earlier than 9.5" ) );
      cb_metadataInDatabase->setEnabled( false );
      cb_metadataInDatabase->setChecked( false );
      cb_metadataInDatabase->setToolTip( tr( "Saving metadata in databases not available for PostgreSQL databases earlier than 9.5" ) );
    }
    else
    {
      cb_projectsInDatabase->setEnabled( true );
      cb_projectsInDatabase->setToolTip( QString() );
      cb_metadataInDatabase->setEnabled( true );
      cb_metadataInDatabase->setToolTip( QString() );
    }

    // Database successfully opened; we can now issue SQL commands.
    bar->pushMessage( tr( "Connection to %1 was successful." ).arg( txtName->text() ),
                      Qgis::MessageLevel::Success );

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
