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

#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>

#include "qgspgnewconnection.h"
#include "qgsauthmanager.h"
#include "qgscontexthelp.h"
#include "qgsdatasourceuri.h"
#include "qgspostgresconn.h"

QgsPgNewConnection::QgsPgNewConnection( QWidget *parent, const QString& connName, Qt::WindowFlags fl )
    : QDialog( parent, fl )
    , mOriginalConnName( connName )
    , mAuthConfigSelect( nullptr )
{
  setupUi( this );

  cbxSSLmode->addItem( tr( "disable" ), QgsDataSourceURI::SSLdisable );
  cbxSSLmode->addItem( tr( "allow" ), QgsDataSourceURI::SSLallow );
  cbxSSLmode->addItem( tr( "prefer" ), QgsDataSourceURI::SSLprefer );
  cbxSSLmode->addItem( tr( "require" ), QgsDataSourceURI::SSLrequire );
  cbxSSLmode->addItem( tr( "verify-ca" ), QgsDataSourceURI::SSLverifyCA );
  cbxSSLmode->addItem( tr( "verify-full" ), QgsDataSourceURI::SSLverifyFull );

  mAuthConfigSelect = new QgsAuthConfigSelect( this, "postgres" );
  tabAuthentication->insertTab( 1, mAuthConfigSelect, tr( "Configurations" ) );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QSettings settings;

    QString key = "/PostgreSQL/connections/" + connName;
    txtService->setText( settings.value( key + "/service" ).toString() );
    txtHost->setText( settings.value( key + "/host" ).toString() );
    QString port = settings.value( key + "/port" ).toString();
    if ( port.length() == 0 )
    {
      port = "5432";
    }
    txtPort->setText( port );
    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    cb_publicSchemaOnly->setChecked( settings.value( key + "/publicOnly", false ).toBool() );
    cb_geometryColumnsOnly->setChecked( settings.value( key + "/geometryColumnsOnly", true ).toBool() );
    cb_dontResolveType->setChecked( settings.value( key + "/dontResolveType", false ).toBool() );
    cb_allowGeometrylessTables->setChecked( settings.value( key + "/allowGeometrylessTables", false ).toBool() );
    // Ensure that cb_publicSchemaOnly is set correctly
    on_cb_geometryColumnsOnly_clicked();

    cb_useEstimatedMetadata->setChecked( settings.value( key + "/estimatedMetadata", false ).toBool() );

    cbxSSLmode->setCurrentIndex( cbxSSLmode->findData( settings.value( key + "/sslmode", QgsDataSourceURI::SSLprefer ).toInt() ) );

    if ( settings.value( key + "/saveUsername" ).toString() == "true" )
    {
      txtUsername->setText( settings.value( key + "/username" ).toString() );
      chkStoreUsername->setChecked( true );
    }

    if ( settings.value( key + "/savePassword" ).toString() == "true" )
    {
      txtPassword->setText( settings.value( key + "/password" ).toString() );
      chkStorePassword->setChecked( true );
    }

    // Old save setting
    if ( settings.contains( key + "/save" ) )
    {
      txtUsername->setText( settings.value( key + "/username" ).toString() );
      chkStoreUsername->setChecked( !txtUsername->text().isEmpty() );

      if ( settings.value( key + "/save" ).toString() == "true" )
        txtPassword->setText( settings.value( key + "/password" ).toString() );

      chkStorePassword->setChecked( true );
    }

    QString authcfg = settings.value( key + "/authcfg" ).toString();
    mAuthConfigSelect->setConfigId( authcfg );
    if ( !authcfg.isEmpty() )
    {
      tabAuthentication->setCurrentIndex( tabAuthentication->indexOf( mAuthConfigSelect ) );
    }


    txtName->setText( connName );
  }
}
/** Autoconnected SLOTS **/
void QgsPgNewConnection::accept()
{
  QSettings settings;
  QString baseKey = "/PostgreSQL/connections/";
  settings.setValue( baseKey + "selected", txtName->text() );
  bool hasAuthConfigID = !mAuthConfigSelect->configId().isEmpty();

  if ( !hasAuthConfigID && chkStorePassword->isChecked() &&
       QMessageBox::question( this,
                              tr( "Saving passwords" ),
                              tr( "WARNING: You have opted to save your password. It will be stored in plain text in your project files and in your home directory on Unix-like systems, or in your user profile on Windows. If you do not want this to happen, please press the Cancel button.\n" ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // warn if entry was renamed to an existing connection
  if (( mOriginalConnName.isNull() || mOriginalConnName.compare( txtName->text(), Qt::CaseInsensitive ) != 0 ) &&
      ( settings.contains( baseKey + txtName->text() + "/service" ) ||
        settings.contains( baseKey + txtName->text() + "/host" ) ) &&
      QMessageBox::question( this,
                             tr( "Save connection" ),
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
  settings.setValue( baseKey + "/username", chkStoreUsername->isChecked() && !hasAuthConfigID ? txtUsername->text() : "" );
  settings.setValue( baseKey + "/password", chkStorePassword->isChecked() && !hasAuthConfigID ? txtPassword->text() : "" );
  settings.setValue( baseKey + "/authcfg", mAuthConfigSelect->configId() );
  settings.setValue( baseKey + "/publicOnly", cb_publicSchemaOnly->isChecked() );
  settings.setValue( baseKey + "/geometryColumnsOnly", cb_geometryColumnsOnly->isChecked() );
  settings.setValue( baseKey + "/dontResolveType", cb_dontResolveType->isChecked() );
  settings.setValue( baseKey + "/allowGeometrylessTables", cb_allowGeometrylessTables->isChecked() );
  settings.setValue( baseKey + "/sslmode", cbxSSLmode->itemData( cbxSSLmode->currentIndex() ).toInt() );
  settings.setValue( baseKey + "/saveUsername", chkStoreUsername->isChecked() && !hasAuthConfigID ? "true" : "false" );
  settings.setValue( baseKey + "/savePassword", chkStorePassword->isChecked() && !hasAuthConfigID ? "true" : "false" );
  settings.setValue( baseKey + "/estimatedMetadata", cb_useEstimatedMetadata->isChecked() );

  // remove old save setting
  settings.remove( baseKey + "/save" );

  QDialog::accept();
}

void QgsPgNewConnection::on_btnConnect_clicked()
{
  testConnection();
}

void QgsPgNewConnection::on_cb_geometryColumnsOnly_clicked()
{
  if ( cb_geometryColumnsOnly->checkState() == Qt::Checked )
    cb_publicSchemaOnly->setEnabled( false );
  else
    cb_publicSchemaOnly->setEnabled( true );
}

/** End  Autoconnected SLOTS **/

QgsPgNewConnection::~QgsPgNewConnection()
{
}

void QgsPgNewConnection::testConnection()
{
  QgsDataSourceURI uri;
  if ( !txtService->text().isEmpty() )
  {
    uri.setConnection( txtService->text(), txtDatabase->text(),
                       txtUsername->text(), txtPassword->text(),
                       ( QgsDataSourceURI::SSLmode ) cbxSSLmode->itemData( cbxSSLmode->currentIndex() ).toInt(),
                       mAuthConfigSelect->configId() );
  }
  else
  {
    uri.setConnection( txtHost->text(), txtPort->text(), txtDatabase->text(),
                       txtUsername->text(), txtPassword->text(),
                       ( QgsDataSourceURI::SSLmode ) cbxSSLmode->itemData( cbxSSLmode->currentIndex() ).toInt(),
                       mAuthConfigSelect->configId() );
  }

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), true );

  if ( conn )
  {
    // Database successfully opened; we can now issue SQL commands.
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "Connection to %1 was successful" ).arg( txtDatabase->text() ) );

    // free pg connection resources
    conn->unref();
  }
  else
  {
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "Connection failed - consult message log for details.\n\n" ) );
  }
}
