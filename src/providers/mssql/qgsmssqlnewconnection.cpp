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

#include <QInputDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QRegExpValidator>

#include "qgsmssqlnewconnection.h"
#include "qgsmssqlprovider.h"
#include "qgssettings.h"

QgsMssqlNewConnection::QgsMssqlNewConnection( QWidget *parent, const QString &connName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mOriginalConnName( connName )
{
  setupUi( this );
  connect( btnListDatabase, &QPushButton::clicked, this, &QgsMssqlNewConnection::btnListDatabase_clicked );
  connect( btnConnect, &QPushButton::clicked, this, &QgsMssqlNewConnection::btnConnect_clicked );
  connect( cb_trustedConnection, &QCheckBox::clicked, this, &QgsMssqlNewConnection::cb_trustedConnection_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsMssqlNewConnection::showHelp );

  lblWarning->hide();

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QgsSettings settings;

    QString key = "/MSSQL/connections/" + connName;
    txtService->setText( settings.value( key + "/service" ).toString() );
    txtHost->setText( settings.value( key + "/host" ).toString() );
    listDatabase->addItem( settings.value( key + "/database" ).toString() );
    listDatabase->setCurrentRow( 0 );
    cb_geometryColumns->setChecked( settings.value( key + "/geometryColumns", true ).toBool() );
    cb_allowGeometrylessTables->setChecked( settings.value( key + "/allowGeometrylessTables", true ).toBool() );
    cb_useEstimatedMetadata->setChecked( settings.value( key + "/estimatedMetadata", false ).toBool() );

    if ( settings.value( key + "/saveUsername" ).toString() == QLatin1String( "true" ) )
    {
      txtUsername->setText( settings.value( key + "/username" ).toString() );
      chkStoreUsername->setChecked( true );
      cb_trustedConnection->setChecked( false );
    }

    if ( settings.value( key + "/savePassword" ).toString() == QLatin1String( "true" ) )
    {
      txtPassword->setText( settings.value( key + "/password" ).toString() );
      chkStorePassword->setChecked( true );
    }

    txtName->setText( connName );
  }
  txtName->setValidator( new QRegExpValidator( QRegExp( "[^\\/]+" ), txtName ) );
  cb_trustedConnection_clicked();
}
//! Autoconnected SLOTS *
void QgsMssqlNewConnection::accept()
{
  QgsSettings settings;
  QString baseKey = QStringLiteral( "/MSSQL/connections/" );
  settings.setValue( baseKey + "selected", txtName->text() );

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
  QString database;
  QListWidgetItem *item = listDatabase->currentItem();
  if ( item && item->text() != QLatin1String( "(from service)" ) )
  {
    database = item->text();
  }

  settings.setValue( baseKey + "/service", txtService->text() );
  settings.setValue( baseKey + "/host", txtHost->text() );
  settings.setValue( baseKey + "/database", database );
  settings.setValue( baseKey + "/username", chkStoreUsername->isChecked() ? txtUsername->text() : QLatin1String( "" ) );
  settings.setValue( baseKey + "/password", chkStorePassword->isChecked() ? txtPassword->text() : QLatin1String( "" ) );
  settings.setValue( baseKey + "/saveUsername", chkStoreUsername->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/savePassword", chkStorePassword->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/geometryColumns", cb_geometryColumns->isChecked() );
  settings.setValue( baseKey + "/allowGeometrylessTables", cb_allowGeometrylessTables->isChecked() );
  settings.setValue( baseKey + "/estimatedMetadata", cb_useEstimatedMetadata->isChecked() );

  QDialog::accept();
}

void QgsMssqlNewConnection::btnConnect_clicked()
{
  testConnection();
}

void QgsMssqlNewConnection::btnListDatabase_clicked()
{
  listDatabases();
}

void QgsMssqlNewConnection::cb_trustedConnection_clicked()
{
  if ( cb_trustedConnection->checkState() == Qt::Checked )
  {
    txtUsername->setEnabled( false );
    txtUsername->clear();
    txtPassword->setEnabled( false );
    txtPassword->clear();
  }
  else
  {
    txtUsername->setEnabled( true );
    txtPassword->setEnabled( true );
  }
}

//! End  Autoconnected SLOTS *

bool QgsMssqlNewConnection::testConnection( const QString &testDatabase )
{
  bar->pushMessage( QStringLiteral( "Testing connection" ), QStringLiteral( "....." ) );
  // Gross but needed to show the last message.
  qApp->processEvents();

  if ( txtService->text().isEmpty() && txtHost->text().isEmpty() )
  {
    bar->clearWidgets();
    bar->pushWarning( tr( "Connection Failed" ), tr( "Host name hasn't been specified." ) );
    return false;
  }

  QString database;
  QListWidgetItem *item = listDatabase->currentItem();
  if ( !testDatabase.isEmpty() )
  {
    database = testDatabase;
  }
  else if ( item && item->text() != QLatin1String( "(from service)" ) )
  {
    database = item->text();
  }

  QSqlDatabase db = QgsMssqlProvider::GetDatabase( txtService->text().trimmed(),
                    txtHost->text().trimmed(),
                    database,
                    txtUsername->text().trimmed(),
                    txtPassword->text().trimmed() );

  if ( db.isOpen() )
    db.close();

  if ( !db.open() )
  {
    bar->clearWidgets();
    bar->pushWarning( tr( "Error opening connection" ), db.lastError().text() );
    return false;
  }
  else
  {
    if ( database.isEmpty() )
    {
      database = txtService->text();
    }
    bar->clearWidgets();
  }

  return true;
}

void QgsMssqlNewConnection::listDatabases()
{
  testConnection( QStringLiteral( "master" ) );
  listDatabase->clear();
  QString queryStr = QStringLiteral( "SELECT name FROM master..sysdatabases WHERE name NOT IN ('master', 'tempdb', 'model', 'msdb')" );

  QSqlDatabase db = QgsMssqlProvider::GetDatabase( txtService->text().trimmed(),
                    txtHost->text().trimmed(),
                    QStringLiteral( "master" ),
                    txtUsername->text().trimmed(),
                    txtPassword->text().trimmed() );
  if ( db.open() )
  {
    QSqlQuery query = QSqlQuery( db );
    query.setForwardOnly( true );
    ( void )query.exec( queryStr );

    if ( !txtService->text().isEmpty() )
    {
      listDatabase->addItem( QStringLiteral( "(from service)" ) );
    }

    if ( query.isActive() )
    {
      while ( query.next() )
      {
        QString name = query.value( 0 ).toString();
        listDatabase->addItem( name );
      }
      listDatabase->setCurrentRow( 0 );
    }
    db.close();
  }
}

void QgsMssqlNewConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#connecting-to-mssql-spatial" ) );
}
