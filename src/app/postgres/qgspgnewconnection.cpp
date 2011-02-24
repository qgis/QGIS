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
/* $Id$ */

#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>

#include "qgspgnewconnection.h"
#include "qgscontexthelp.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgscredentialdialog.h"

extern "C"
{
#include <libpq-fe.h>
}

QgsPgNewConnection::QgsPgNewConnection( QWidget *parent, const QString& connName, Qt::WFlags fl )
    : QDialog( parent, fl ), mOriginalConnName( connName )
{
  setupUi( this );

  cbxSSLmode->addItem( tr( "disable" ), QgsDataSourceURI::SSLdisable );
  cbxSSLmode->addItem( tr( "allow" ), QgsDataSourceURI::SSLallow );
  cbxSSLmode->addItem( tr( "prefer" ), QgsDataSourceURI::SSLprefer );
  cbxSSLmode->addItem( tr( "require" ), QgsDataSourceURI::SSLrequire );

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
    cb_geometryColumnsOnly->setChecked( settings.value( key + "/geometrycolumnsOnly", false ).toBool() );
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

    txtName->setText( connName );
  }
}
/** Autoconnected SLOTS **/
void QgsPgNewConnection::accept()
{
  QSettings settings;
  QString baseKey = "/PostgreSQL/connections/";
  settings.setValue( baseKey + "selected", txtName->text() );

  // warn if entry was renamed to an existing connection
  if (( mOriginalConnName.isNull() || mOriginalConnName != txtName->text() ) &&
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
  }

  baseKey += txtName->text();
  settings.setValue( baseKey + "/service", txtService->text() );
  settings.setValue( baseKey + "/host", txtHost->text() );
  settings.setValue( baseKey + "/port", txtPort->text() );
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/username", chkStoreUsername->isChecked() ? txtUsername->text() : "" );
  settings.setValue( baseKey + "/password", chkStorePassword->isChecked() ? txtPassword->text() : "" );
  settings.setValue( baseKey + "/publicOnly", cb_publicSchemaOnly->isChecked() );
  settings.setValue( baseKey + "/geometryColumnsOnly", cb_geometryColumnsOnly->isChecked() );
  settings.setValue( baseKey + "/allowGeometrylessTables", cb_allowGeometrylessTables->isChecked() );
  settings.setValue( baseKey + "/sslmode", cbxSSLmode->itemData( cbxSSLmode->currentIndex() ).toInt() );
  settings.setValue( baseKey + "/saveUsername", chkStoreUsername->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/savePassword", chkStorePassword->isChecked() ? "true" : "false" );
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

/** end  Autoconnected SLOTS **/

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
                       ( QgsDataSourceURI::SSLmode ) cbxSSLmode->itemData( cbxSSLmode->currentIndex() ).toInt() );
  }
  else
  {
    uri.setConnection( txtHost->text(), txtPort->text(), txtDatabase->text(),
                       txtUsername->text(), txtPassword->text(),
                       ( QgsDataSourceURI::SSLmode ) cbxSSLmode->itemData( cbxSSLmode->currentIndex() ).toInt() );
  }
  QString conninfo = uri.connectionInfo();
  QgsDebugMsg( "PQconnectdb(\"" + conninfo + "\");" );

  PGconn *pd = PQconnectdb( conninfo.toLocal8Bit() );  // use what is set based on locale; after connecting, use Utf8
  // check the connection status
  if ( PQstatus( pd ) != CONNECTION_OK )
  {
    QString username = txtUsername->text();
    QString password = txtPassword->text();

    uri.setUsername( "" );
    uri.setPassword( "" );

    while ( PQstatus( pd ) != CONNECTION_OK )
    {
      bool ok = QgsCredentials::instance()->get( conninfo, username, password, QString::fromUtf8( PQerrorMessage( pd ) ) );
      if ( !ok )
        break;

      ::PQfinish( pd );

      QgsDataSourceURI uri( conninfo );

      if ( !username.isEmpty() )
        uri.setUsername( username );

      if ( !password.isEmpty() )
        uri.setPassword( password );

      QgsDebugMsg( "PQconnectdb(\"" + uri.connectionInfo() + "\");" );
      pd = PQconnectdb( uri.connectionInfo().toLocal8Bit() );
    }

    if ( PQstatus( pd ) == CONNECTION_OK )
    {
      if ( chkStoreUsername->isChecked() )
        txtUsername->setText( username );
      if ( chkStorePassword->isChecked() )
        txtPassword->setText( password );

      QgsCredentials::instance()->put( conninfo, username, password );
    }
  }

  if ( PQstatus( pd ) == CONNECTION_OK )
  {
    // Database successfully opened; we can now issue SQL commands.
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "Connection to %1 was successful" ).arg( txtDatabase->text() ) );
  }
  else
  {
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "Connection failed - Check settings and try again.\n\nExtended error information:\n%1" )
                              .arg( QString::fromUtf8( PQerrorMessage( pd ) ) ) );
  }
  // free pg connection resources
  PQfinish( pd );
}
