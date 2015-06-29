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

#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>

#include "qgsoraclenewconnection.h"
#include "qgscontexthelp.h"
#include "qgsdatasourceuri.h"
#include "qgsoracletablemodel.h"

QgsOracleNewConnection::QgsOracleNewConnection( QWidget *parent, const QString& connName, Qt::WindowFlags fl )
    : QDialog( parent, fl ), mOriginalConnName( connName )
{
  setupUi( this );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QSettings settings;

    QString key = "/Oracle/connections/" + connName;
    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    txtHost->setText( settings.value( key + "/host" ).toString() );
    QString port = settings.value( key + "/port" ).toString();
    if ( port.length() == 0 )
    {
      port = "1521";
    }
    txtPort->setText( port );
    txtOptions->setText( settings.value( key + "/dboptions" ).toString() );
    cb_userTablesOnly->setChecked( settings.value( key + "/userTablesOnly", false ).toBool() );
    cb_geometryColumnsOnly->setChecked( settings.value( key + "/geometryColumnsOnly", true ).toBool() );
    cb_allowGeometrylessTables->setChecked( settings.value( key + "/allowGeometrylessTables", false ).toBool() );
    cb_useEstimatedMetadata->setChecked( settings.value( key + "/estimatedMetadata", false ).toBool() );
    cb_onlyExistingTypes->setChecked( settings.value( key + "/onlyExistingTypes", true ).toBool() );

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
void QgsOracleNewConnection::accept()
{
  QSettings settings;
  QString baseKey = "/Oracle/connections/";
  settings.setValue( baseKey + "selected", txtName->text() );

  if ( chkStorePassword->isChecked() &&
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
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/host", txtHost->text() );
  settings.setValue( baseKey + "/port", txtPort->text() );
  settings.setValue( baseKey + "/username", chkStoreUsername->isChecked() ? txtUsername->text() : "" );
  settings.setValue( baseKey + "/password", chkStorePassword->isChecked() ? txtPassword->text() : "" );
  settings.setValue( baseKey + "/userTablesOnly", cb_userTablesOnly->isChecked() );
  settings.setValue( baseKey + "/geometryColumnsOnly", cb_geometryColumnsOnly->isChecked() );
  settings.setValue( baseKey + "/allowGeometrylessTables", cb_allowGeometrylessTables->isChecked() );
  settings.setValue( baseKey + "/estimatedMetadata", cb_useEstimatedMetadata->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/onlyExistingTypes", cb_onlyExistingTypes->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/saveUsername", chkStoreUsername->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/savePassword", chkStorePassword->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/dboptions", txtOptions->text() );

  QDialog::accept();
}

void QgsOracleNewConnection::on_btnConnect_clicked()
{
  QgsDataSourceURI uri;
  uri.setConnection( txtHost->text(), txtPort->text(), txtDatabase->text(), txtUsername->text(), txtPassword->text() );
  if ( !txtOptions->text().isEmpty() )
    uri.setParam( "dboptions", txtOptions->text() );

  QgsOracleConn *conn = QgsOracleConn::connectDb( uri );

  if ( conn )
  {
    // Database successfully opened; we can now issue SQL commands.
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "Connection to %1 was successful" ).arg( txtDatabase->text() ) );

    // free connection resources
    conn->disconnect();
  }
  else
  {
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "Connection failed - consult message log for details.\n\n" ) );
  }
}

/** end  Autoconnected SLOTS **/

QgsOracleNewConnection::~QgsOracleNewConnection()
{
}
