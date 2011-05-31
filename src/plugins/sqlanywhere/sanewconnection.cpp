/***************************************************************************
  sanewconnection.h
  Dialogue box for defining new connections to a SQL Anywhere database
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author               : David DeHaan
    email                : ddehaan at sybase dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>

#include "sanewconnection.h"
#include "sqlanyconnection.h"

#include "qgscontexthelp.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgscredentialdialog.h"

SaNewConnection::SaNewConnection( QWidget *parent, const QString& connName, Qt::WFlags fl )
    : QDialog( parent, fl ), mOriginalConnName( connName )
{
  setupUi( this );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    QSettings settings;

    QString key = "/SQLAnywhere/connections/" + connName;
    txtName->setText( connName );
    txtHost->setText( settings.value( key + "/host" ).toString() );
    txtPort->setText( settings.value( key + "/port" ).toString() );
    txtServer->setText( settings.value( key + "/server" ).toString() );
    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    txtParameters->setText( settings.value( key + "/parameters" ).toString() );

    if ( settings.value( key + "/saveUsername", true ).toBool() )
    {
      txtUsername->setText( settings.value( key + "/username" ).toString() );
      chkStoreUsername->setChecked( true );
    }
    if ( settings.value( key + "/savePassword", false ).toBool() )
    {
      txtPassword->setText( settings.value( key + "/password" ).toString() );
      chkStorePassword->setChecked( true );
    }
    chkSimpleEncryption->setChecked( settings.value( key + "/simpleEncryption", false ).toBool() );
    chkEstimateMetadata->setChecked( settings.value( key + "/estimateMetadata", false ).toBool() );
    chkOtherSchemas->setChecked( settings.value( key + "/otherSchemas", false ).toBool() );

  }
}

SaNewConnection::~SaNewConnection()
{
}

void SaNewConnection::accept()
{
  QSettings settings;
  QString baseKey = "/SQLAnywhere/connections/";
  settings.setValue( baseKey + "selected", txtName->text() );

  // warn if entry was renamed to an existing connection
  if (( mOriginalConnName.isNull() || mOriginalConnName != txtName->text() ) &&
      settings.contains( baseKey + txtName->text() + "/host" ) &&
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
  settings.setValue( baseKey + "/host", txtHost->text() );
  settings.setValue( baseKey + "/port", txtPort->text() );
  settings.setValue( baseKey + "/server", txtServer->text() );
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/parameters", txtParameters->text() );
  settings.setValue( baseKey + "/username", chkStoreUsername->isChecked() ? txtUsername->text() : "" );
  settings.setValue( baseKey + "/password", chkStorePassword->isChecked() ? txtPassword->text() : "" );
  settings.setValue( baseKey + "/saveUsername", chkStoreUsername->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/savePassword", chkStorePassword->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/simpleEncryption", chkSimpleEncryption->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/estimateMetadata", chkEstimateMetadata->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/otherSchemas", chkOtherSchemas->isChecked() ? "true" : "false" );

  QDialog::accept();
}

void SaNewConnection::on_btnConnect_clicked()
{
  testConnection();
}

void SaNewConnection::testConnection()
{
  char  errbuf[SACAPI_ERROR_SIZE];
  sacapi_i32  code;
  SqlAnyConnection *conn;

  // load the SQL Anywhere interface
  if ( !SqlAnyConnection::initApi() )
  {
    QMessageBox::information( this,
                              tr( "Failed to load interface" ),
                              tr( SqlAnyConnection::failedInitMsg() ) );
    return;
  }

  // establish read-only connection to the database
  conn = SqlAnyConnection::connect( txtName->text()
                                    , txtHost->text(), txtPort->text(), txtServer->text()
                                    , txtDatabase->text(), txtParameters->text(), txtUsername->text()
                                    , txtPassword->text(), chkSimpleEncryption->isChecked()
                                    , chkEstimateMetadata->isChecked(), true
                                    , code, errbuf, sizeof( errbuf ) );
  if ( conn )
  {
    // retrieve the username and password, in case the user adjusted them
    QgsDataSourceURI    theUri( conn->uri() );
    if ( chkStoreUsername->isChecked() )
    {
      txtUsername->setText( theUri.username() );
    }
    if ( chkStorePassword->isChecked() )
    {
      txtPassword->setText( theUri.password() );
    }
    conn->release();

    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "Connection to %1 was successful" )
                              .arg( txtDatabase->text() ) );
  }
  else
  {
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "Connection failed. "
                                  "Check settings and try again.\n\n"
                                  "SQL Anywhere error code: %1\n"
                                  "Description: %2" )
                              .arg( code )
                              .arg( errbuf ) );
  }
  SqlAnyConnection::releaseApi();
}
