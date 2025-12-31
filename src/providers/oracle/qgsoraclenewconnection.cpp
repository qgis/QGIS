/***************************************************************************
                    qgsoraclenewconnection.cpp  -  description
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoraclenewconnection.h"

#include "qgsdatasourceuri.h"
#include "qgsoracleconnpool.h"
#include "qgsoracleproviderconnection.h"
#include "qgsoracletablemodel.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgssettings.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpressionValidator>

#include "moc_qgsoraclenewconnection.cpp"

QgsOracleNewConnection::QgsOracleNewConnection( QWidget *parent, const QString &connName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mOriginalConnName( connName )
{
  setupUi( this );

  txtSchema->setShowClearButton( true );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsOracleNewConnection::showHelp );
  connect( btnConnect, &QPushButton::clicked, this, &QgsOracleNewConnection::testConnection );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  btnConnect->setDisabled( true );
  connect( txtName, &QLineEdit::textChanged, this, &QgsOracleNewConnection::updateOkButtonState );
  connect( txtDatabase, &QLineEdit::textChanged, this, &QgsOracleNewConnection::updateOkButtonState );
  connect( txtHost, &QLineEdit::textChanged, this, &QgsOracleNewConnection::updateOkButtonState );
  connect( txtPort, &QLineEdit::textChanged, this, &QgsOracleNewConnection::updateOkButtonState );

  mAuthSettings->setDataprovider( u"oracle"_s );
  mAuthSettings->showStoreCheckboxes( true );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QgsSettings settings;

    QString key = u"/Oracle/connections/"_s + connName;
    txtDatabase->setText( settings.value( key + u"/database"_s ).toString() );
    txtHost->setText( settings.value( key + u"/host"_s ).toString() );
    QString port = settings.value( key + u"/port"_s ).toString();

    // User can set database without host and port, meaning he is using a service (tnsnames.ora)
    // if he sets host, port has to be set also (and vice versa)
    // https://github.com/qgis/QGIS/issues/38979
    if ( port.length() == 0
         && ( !txtHost->text().isEmpty() || txtDatabase->text().isEmpty() ) )
    {
      port = u"1521"_s;
    }
    txtPort->setText( port );

    txtOptions->setText( settings.value( key + u"/dboptions"_s ).toString() );
    txtWorkspace->setText( settings.value( key + u"/dbworkspace"_s ).toString() );
    txtSchema->setText( settings.value( key + u"/schema"_s ).toString() );
    cb_userTablesOnly->setChecked( settings.value( key + u"/userTablesOnly"_s, false ).toBool() );
    cb_geometryColumnsOnly->setChecked( settings.value( key + u"/geometryColumnsOnly"_s, true ).toBool() );
    cb_allowGeometrylessTables->setChecked( settings.value( key + u"/allowGeometrylessTables"_s, false ).toBool() );
    cb_useEstimatedMetadata->setChecked( settings.value( key + u"/estimatedMetadata"_s, false ).toBool() );
    cb_onlyExistingTypes->setChecked( settings.value( key + u"/onlyExistingTypes"_s, true ).toBool() );
    cb_includeGeoAttributes->setChecked( settings.value( key + u"/includeGeoAttributes"_s, false ).toBool() );
    cb_projectsInDatabase->setChecked( settings.value( key + "/projectsInDatabase", false ).toBool() );

    if ( settings.value( key + u"/saveUsername"_s ).toString() == "true"_L1 )
    {
      mAuthSettings->setUsername( settings.value( key + "/username" ).toString() );
      mAuthSettings->setStoreUsernameChecked( true );
    }

    if ( settings.value( key + u"/savePassword"_s ).toString() == "true"_L1 )
    {
      mAuthSettings->setPassword( settings.value( key + "/password" ).toString() );
      mAuthSettings->setStorePasswordChecked( true );
    }

    // Old save setting
    if ( settings.contains( key + u"/save"_s ) )
    {
      mAuthSettings->setUsername( settings.value( key + "/username" ).toString() );
      mAuthSettings->setStoreUsernameChecked( !mAuthSettings->username().isEmpty() );

      if ( settings.value( key + "/save" ).toString() == "true"_L1 )
        mAuthSettings->setPassword( settings.value( key + "/password" ).toString() );

      mAuthSettings->setStorePasswordChecked( true );
    }

    QString authcfg = settings.value( key + "/authcfg" ).toString();
    mAuthSettings->setConfigId( authcfg );

    txtName->setText( connName );
  }
  txtName->setValidator( new QRegularExpressionValidator( QRegularExpression( u"[^\\/]+"_s ), txtName ) );
}

void QgsOracleNewConnection::accept()
{
  QgsSettings settings;
  QString baseKey = u"/Oracle/connections/"_s;
  settings.setValue( baseKey + u"selected"_s, txtName->text() );
  bool hasAuthConfigID = !mAuthSettings->configId().isEmpty();

  if ( !hasAuthConfigID && mAuthSettings->storePasswordIsChecked() && QMessageBox::question( this, tr( "Saving Passwords" ), tr( "WARNING: You have opted to save your password. It will be stored in plain text in your project files and in your home directory on Unix-like systems, or in your user profile on Windows. If you do not want this to happen, please press the Cancel button.\n" ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( txtName->text(), Qt::CaseInsensitive ) != 0 ) && ( settings.contains( baseKey + txtName->text() + u"/service"_s ) || settings.contains( baseKey + txtName->text() + u"/host"_s ) ) && QMessageBox::question( this, tr( "Save Connection" ), tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // on rename delete the original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != txtName->text() )
  {
    settings.remove( baseKey + mOriginalConnName );
    settings.sync();
  }

  // This settings will be overridden by later saveConnections call on providerMetadata
  // but there are still used when QgsOracleConn::connUri is called to generate uri
  // so don't remove them
  baseKey += txtName->text();
  settings.setValue( baseKey + u"/database"_s, txtDatabase->text() );
  settings.setValue( baseKey + u"/host"_s, txtHost->text() );
  settings.setValue( baseKey + u"/port"_s, txtPort->text() );
  settings.setValue( baseKey + u"/username"_s, mAuthSettings->storeUsernameIsChecked() ? mAuthSettings->username() : QString() );
  settings.setValue( baseKey + u"/password"_s, mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID ? mAuthSettings->password() : QString() );
  settings.setValue( baseKey + u"/authcfg"_s, mAuthSettings->configId() );
  settings.setValue( baseKey + u"/userTablesOnly"_s, cb_userTablesOnly->isChecked() );
  settings.setValue( baseKey + u"/geometryColumnsOnly"_s, cb_geometryColumnsOnly->isChecked() );
  settings.setValue( baseKey + u"/allowGeometrylessTables"_s, cb_allowGeometrylessTables->isChecked() );
  settings.setValue( baseKey + u"/estimatedMetadata"_s, cb_useEstimatedMetadata->isChecked() ? u"true"_s : u"false"_s );
  settings.setValue( baseKey + u"/onlyExistingTypes"_s, cb_onlyExistingTypes->isChecked() ? u"true"_s : u"false"_s );
  settings.setValue( baseKey + u"/includeGeoAttributes"_s, cb_includeGeoAttributes->isChecked() ? u"true"_s : u"false"_s );
  settings.setValue( baseKey + u"/projectsInDatabase"_s, cb_projectsInDatabase->isChecked() );
  settings.setValue( baseKey + u"/saveUsername"_s, mAuthSettings->storeUsernameIsChecked() ? u"true"_s : u"false"_s );
  settings.setValue( baseKey + u"/savePassword"_s, mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID ? u"true"_s : u"false"_s );
  settings.setValue( baseKey + u"/dboptions"_s, txtOptions->text() );
  settings.setValue( baseKey + u"/dbworkspace"_s, txtWorkspace->text() );
  settings.setValue( baseKey + u"/schema"_s, txtSchema->text() );

  QVariantMap configuration;
  configuration.insert( u"userTablesOnly"_s, cb_userTablesOnly->isChecked() );
  configuration.insert( u"geometryColumnsOnly"_s, cb_geometryColumnsOnly->isChecked() );
  configuration.insert( u"allowGeometrylessTables"_s, cb_allowGeometrylessTables->isChecked() );
  configuration.insert( u"onlyExistingTypes"_s, cb_onlyExistingTypes->isChecked() ? u"true"_s : u"false"_s );
  configuration.insert( u"saveUsername"_s, mAuthSettings->storeUsernameIsChecked() ? u"true"_s : u"false"_s );
  configuration.insert( u"savePassword"_s, mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID ? u"true"_s : u"false"_s );
  configuration.insert( u"includeGeoAttributes"_s, cb_includeGeoAttributes->isChecked() );
  configuration.insert( u"schema"_s, txtSchema->text() );
  configuration.insert( u"projectsInDatabase"_s, cb_projectsInDatabase->isChecked() );

  QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( u"oracle"_s );
  QgsOracleProviderConnection *providerConnection = static_cast<QgsOracleProviderConnection *>( providerMetadata->createConnection( txtName->text() ) );
  providerConnection->setUri( QgsOracleConn::connUri( txtName->text() ).uri( false ) );
  providerConnection->setConfiguration( configuration );
  providerMetadata->saveConnection( providerConnection, txtName->text() );

  QDialog::accept();
}

void QgsOracleNewConnection::testConnection()
{
  QgsDataSourceUri uri;
  uri.setConnection( txtHost->text(), txtPort->text(), txtDatabase->text(), mAuthSettings->username(), mAuthSettings->password(), QgsDataSourceUri::SslPrefer /* meaningless for oracle */, mAuthSettings->configId() );
  if ( !txtOptions->text().isEmpty() )
    uri.setParam( u"dboptions"_s, txtOptions->text() );
  if ( !txtWorkspace->text().isEmpty() )
    uri.setParam( u"dbworkspace"_s, txtWorkspace->text() );

  QgsOracleConn *conn = QgsOracleConnPool::instance()->acquireConnection( QgsOracleConn::toPoolName( uri ) );

  if ( conn )
  {
    // Database successfully opened; we can now issue SQL commands.
    bar->pushMessage( tr( "Connection to %1 was successful." ).arg( txtName->text() ), Qgis::MessageLevel::Success );
    // free connection resources
    QgsOracleConnPool::instance()->releaseConnection( conn );
  }
  else
  {
    bar->pushMessage( tr( "Connection failed - consult message log for details." ), Qgis::MessageLevel::Warning );
  }
}

void QgsOracleNewConnection::showHelp()
{
  QgsHelp::openHelp( u"managing_data_source/opening_data.html#connecting-to-oracle-spatial"_s );
}

void QgsOracleNewConnection::updateOkButtonState()
{
  // User can set database without host and port, meaning he is using a service (tnsnames.ora)
  // if he sets host, port has to be set also (and vice versa)
  // https://github.com/qgis/QGIS/issues/38979

  bool enabled = !txtName->text().isEmpty() && !txtDatabase->text().isEmpty()
                 && ( txtHost->text().isEmpty() == txtPort->text().isEmpty() );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
  btnConnect->setEnabled( enabled );
}
