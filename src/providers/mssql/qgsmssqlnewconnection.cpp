/***************************************************************************
                    qgsmssqlnewconnection.cpp  -  description
                             -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
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

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

#include "qgsmssqlnewconnection.h"
#include "qgsmssqlprovider.h"
#include "qgscontexthelp.h"

QgsMssqlNewConnection::QgsMssqlNewConnection( QWidget *parent, const QString& connName, Qt::WFlags fl )
    : QDialog( parent, fl ), mOriginalConnName( connName )
{
  setupUi( this );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QSettings settings;

    QString key = "/MSSQL/connections/" + connName;
    txtService->setText( settings.value( key + "/service" ).toString() );
    txtHost->setText( settings.value( key + "/host" ).toString() );
    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    cb_geometryColumns->setChecked( settings.value( key + "/geometryColumns", true ).toBool() );
    cb_allowGeometrylessTables->setChecked( settings.value( key + "/allowGeometrylessTables", true ).toBool() );
    cb_useEstimatedMetadata->setChecked( settings.value( key + "/estimatedMetadata", false ).toBool() );

    if ( settings.value( key + "/saveUsername" ).toString() == "true" )
    {
      txtUsername->setText( settings.value( key + "/username" ).toString() );
      chkStoreUsername->setChecked( true );
      cb_trustedConnection->setChecked( false );
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
  on_cb_trustedConnection_clicked();
}
/** Autoconnected SLOTS **/
void QgsMssqlNewConnection::accept()
{
  QSettings settings;
  QString baseKey = "/MSSQL/connections/";
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
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/username", chkStoreUsername->isChecked() ? txtUsername->text() : "" );
  settings.setValue( baseKey + "/password", chkStorePassword->isChecked() ? txtPassword->text() : "" );
  settings.setValue( baseKey + "/saveUsername", chkStoreUsername->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/savePassword", chkStorePassword->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/geometryColumns", cb_geometryColumns->isChecked() );
  settings.setValue( baseKey + "/allowGeometrylessTables", cb_allowGeometrylessTables->isChecked() );
  settings.setValue( baseKey + "/estimatedMetadata", cb_useEstimatedMetadata->isChecked() );

  QDialog::accept();
}

void QgsMssqlNewConnection::on_btnConnect_clicked()
{
  testConnection();
}

void QgsMssqlNewConnection::on_cb_trustedConnection_clicked()
{
  if ( cb_trustedConnection->checkState() == Qt::Checked )
  {
    txtUsername->setEnabled( false );
    txtUsername->setText( "" );
    txtPassword->setEnabled( false );
    txtPassword->setText( "" );
  }
  else
  {
    txtUsername->setEnabled( true );
    txtPassword->setEnabled( true );
  }
}

/** end  Autoconnected SLOTS **/

QgsMssqlNewConnection::~QgsMssqlNewConnection()
{
}

void QgsMssqlNewConnection::testConnection()
{
  if ( txtService->text().isEmpty() )
  {
    if ( txtHost->text().isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Test connection" ),
                                tr( "Connection failed - Host name hasn't been specified.\n\n" ) );
      return;
    }

    if ( txtDatabase->text().isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Test connection" ),
                                tr( "Connection failed - Database name hasn't been specified.\n\n" ) );
      return;
    }
  }

  QSqlDatabase db = QgsMssqlProvider::GetDatabase( txtService->text().trimmed(),
                    txtHost->text().trimmed(), txtDatabase->text().trimmed(),
                    txtUsername->text().trimmed(), txtPassword->text().trimmed() );

  if ( db.isOpen() )
    db.close();

  if ( !db.open() )
  {
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              db.lastError( ).text( ) );
  }
  else
  {
    QString dbName = txtDatabase->text();
    if ( dbName.isEmpty() )
    {
      dbName = txtService->text();
    }
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "Connection to %1 was successful" ).arg( dbName ) );
  }
}
