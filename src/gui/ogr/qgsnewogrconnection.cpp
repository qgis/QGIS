/***************************************************************************
                          qgsnewogrconnection.cpp  -  description
                             -------------------
    begin                : Mon Jan 2 2009
    copyright            : (C) 2009 by Godofredo Contreras Nava
    email                : frdcn at hotmail.com
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
#include <QRegularExpressionValidator>
#include <QRegularExpression>

#include "qgsnewogrconnection.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"
#include "qgsogrhelperfunctions.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgsgui.h"

#include <ogr_api.h>
#include <cpl_error.h>
#include "qgshelp.h"

QgsNewOgrConnection::QgsNewOgrConnection( QWidget *parent, const QString &connType, const QString &connName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mOriginalConnName( connName )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( btnConnect, &QPushButton::clicked, this, &QgsNewOgrConnection::btnConnect_clicked );
  Q_NOWARN_DEPRECATED_PUSH
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsNewOgrConnection::showHelp );
  Q_NOWARN_DEPRECATED_POP

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( txtName, &QLineEdit::textChanged, this, &QgsNewOgrConnection::updateOkButtonState );
  connect( txtHost, &QLineEdit::textChanged, this, &QgsNewOgrConnection::updateOkButtonState );
  connect( txtDatabase, &QLineEdit::textChanged, this, &QgsNewOgrConnection::updateOkButtonState );
  connect( txtPort, &QLineEdit::textChanged, this, &QgsNewOgrConnection::updateOkButtonState );

  const QgsSettings settings;

  //add database drivers
  const QStringList dbDrivers = QgsProviderRegistry::instance()->databaseDrivers().split( ';' );
  for ( int i = 0; i < dbDrivers.count(); i++ )
  {
    const QString dbDrive = dbDrivers.at( i );
    cmbDatabaseTypes->addItem( dbDrive.split( ',' ).at( 0 ) );
  }
  txtName->setEnabled( true );
  cmbDatabaseTypes->setEnabled( true );
  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    const QString key = '/' + connType + "/connections/" + connName;
    txtHost->setText( settings.value( key + "/host" ).toString() );
    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    const QString port = settings.value( key + "/port" ).toString();
    txtPort->setText( port );
    if ( settings.value( key + "/store_username" ).toString() == QLatin1String( "true" ) )
    {
      mAuthSettingsDatabase->setUsername( settings.value( key + "/username" ).toString() );
      mAuthSettingsDatabase->setStoreUsernameChecked( true );
    }
    if ( settings.value( key + "/store_password" ).toString() == QLatin1String( "true" ) )
    {
      mAuthSettingsDatabase->setPassword( settings.value( key + "/password" ).toString() );
      mAuthSettingsDatabase->setStorePasswordChecked( true );
    }
    mAuthSettingsDatabase->setConfigId( settings.value( key + "/configid" ).toString() );
    cmbDatabaseTypes->setCurrentIndex( cmbDatabaseTypes->findText( connType ) );
    txtName->setText( connName );
    txtName->setEnabled( false );
    cmbDatabaseTypes->setEnabled( false );
  }
  txtName->setValidator( new QRegularExpressionValidator( QRegularExpression( "[^\\/]+" ), txtName ) );
  mAuthSettingsDatabase->setDataprovider( QStringLiteral( "ogr" ) );
  mAuthSettingsDatabase->showStoreCheckboxes( true );
}

void QgsNewOgrConnection::testConnection()
{
  QString uri;
  uri = createDatabaseURI( cmbDatabaseTypes->currentText(),
                           txtHost->text(),
                           txtDatabase->text(),
                           txtPort->text(),
                           mAuthSettingsDatabase->configId(),
                           mAuthSettingsDatabase->username(),
                           mAuthSettingsDatabase->password(),
                           true );
  QgsDebugMsg( "Connecting using uri = " + uri );
  OGRRegisterAll();
  OGRDataSourceH       poDS;
  OGRSFDriverH         pahDriver;
  CPLErrorReset();
  poDS = OGROpen( uri.toUtf8().constData(), false, &pahDriver );
  if ( !poDS )
  {
    QMessageBox::information( this, tr( "Test Connection" ), tr( "Connection failed - Check settings and try again.\n\nExtended error information:\n%1" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }
  else
  {
    QMessageBox::information( this, tr( "Test Connection" ), tr( "Connection to %1 was successful." ).arg( uri ) );
    OGRReleaseDataSource( poDS );
  }
}

void QgsNewOgrConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#creating-a-stored-connection" ) );
}

void QgsNewOgrConnection::updateOkButtonState()
{
  const bool enabled = !txtName->text().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}


//! Autoconnected SLOTS
void QgsNewOgrConnection::accept()
{
  QgsSettings settings;
  QString baseKey = '/' + cmbDatabaseTypes->currentText() + "/connections/";
  settings.setValue( baseKey + "selected", txtName->text() );

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName != txtName->text() ) &&
       settings.contains( baseKey + txtName->text() + "/host" ) &&
       QMessageBox::question( this,
                              tr( "Save Connection" ),
                              tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // on rename delete original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != txtName->text() )
  {
    settings.remove( baseKey + mOriginalConnName );
  }

  baseKey += txtName->text();
  settings.setValue( baseKey + "/host", txtHost->text() );
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/port", txtPort->text() );
  settings.setValue( baseKey + "/username", mAuthSettingsDatabase->storeUsernameIsChecked() ? mAuthSettingsDatabase->username() : QString() );
  settings.setValue( baseKey + "/password", mAuthSettingsDatabase->storePasswordIsChecked() ? mAuthSettingsDatabase->password() : QString() );
  settings.setValue( baseKey + "/store_username", mAuthSettingsDatabase->storeUsernameIsChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/store_password", mAuthSettingsDatabase->storePasswordIsChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/configid", mAuthSettingsDatabase->configId() );

  QDialog::accept();
}

void QgsNewOgrConnection::btnConnect_clicked()
{
  testConnection();
}

//! End  Autoconnected SLOTS
