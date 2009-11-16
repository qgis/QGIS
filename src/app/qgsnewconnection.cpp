/***************************************************************************
                    qgsnewconnection.cpp  -  description
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

#include "qgsnewconnection.h"
#include "qgscontexthelp.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"

extern "C"
{
#include <libpq-fe.h>
}

QgsNewConnection::QgsNewConnection( QWidget *parent, const QString& connName, Qt::WFlags fl )
    : QDialog( parent, fl ), mOriginalConnName( connName )
{
  setupUi( this );
  connect( buttonBox, SIGNAL( helpRequested() ), this, SLOT( helpClicked() ) );

  cbxSSLmode->insertItem( QgsDataSourceURI::SSLprefer, tr( "prefer" ) );
  cbxSSLmode->insertItem( QgsDataSourceURI::SSLrequire, tr( "require" ) );
  cbxSSLmode->insertItem( QgsDataSourceURI::SSLallow, tr( "allow" ) );
  cbxSSLmode->insertItem( QgsDataSourceURI::SSLdisable, tr( "disable" ) );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QSettings settings;

    QString key = "/PostgreSQL/connections/" + connName;
    txtHost->setText( settings.value( key + "/host" ).toString() );
    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    QString port = settings.value( key + "/port" ).toString();
    if ( port.length() == 0 )
    {
      port = "5432";
    }
    txtPort->setText( port );
    txtUsername->setText( settings.value( key + "/username" ).toString() );
    Qt::CheckState s = Qt::Checked;
    if ( ! settings.value( key + "/publicOnly", false ).toBool() )
      s = Qt::Unchecked;
    cb_publicSchemaOnly->setCheckState( s );
    s = Qt::Checked;
    if ( ! settings.value( key + "/geometrycolumnsOnly", false ).toBool() )
      s = Qt::Unchecked;
    cb_geometryColumnsOnly->setCheckState( s );
    // Ensure that cb_plublicSchemaOnly is set correctly
    on_cb_geometryColumnsOnly_clicked();

    cbxSSLmode->setCurrentIndex( settings.value( key + "/sslmode", QgsDataSourceURI::SSLprefer ).toInt() );

    if ( settings.value( key + "/save" ).toString() == "true" )
    {
      txtPassword->setText( settings.value( key + "/password" ).toString() );
      chkStorePassword->setChecked( true );
    }
    txtName->setText( connName );
  }
}
/** Autoconnected SLOTS **/
void QgsNewConnection::accept()
{
  saveConnection();
  QDialog::accept();
}

void QgsNewConnection::helpClicked()
{
  QgsContextHelp::run( context_id );
}

void QgsNewConnection::on_btnConnect_clicked()
{
  testConnection();
}

void QgsNewConnection::on_cb_geometryColumnsOnly_clicked()
{
  if ( cb_geometryColumnsOnly->checkState() == Qt::Checked )
    cb_publicSchemaOnly->setEnabled( false );
  else
    cb_publicSchemaOnly->setEnabled( true );
}

/** end  Autoconnected SLOTS **/

QgsNewConnection::~QgsNewConnection()
{
}

void QgsNewConnection::testConnection()
{
  QgsDataSourceURI uri;
  uri.setConnection( txtHost->text(), txtPort->text(), txtDatabase->text(), txtUsername->text(), txtPassword->text(), ( QgsDataSourceURI::SSLmode ) cbxSSLmode->currentIndex() );

  QgsLogger::debug( "PQconnectdb(" + uri.connectionInfo() + ");" );

  PGconn *pd = PQconnectdb( uri.connectionInfo().toLocal8Bit().data() );
  if ( PQstatus( pd ) == CONNECTION_OK )
  {
    // Database successfully opened; we can now issue SQL commands.
    QMessageBox::information( this, tr( "Test connection" ), tr( "Connection to %1 was successful" ).arg( txtDatabase->text() ) );
  }
  else
  {
    QMessageBox::information( this, tr( "Test connection" ), tr( "Connection failed - Check settings and try again.\n\nExtended error information:\n%1" ).arg( PQerrorMessage( pd ) ) );
  }
  // free pg connection resources
  PQfinish( pd );
}

void QgsNewConnection::saveConnection()
{
  QSettings settings;
  QString baseKey = "/PostgreSQL/connections/";
  settings.setValue( baseKey + "selected", txtName->text() );
  //delete original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != txtName->text() )
  {
    settings.remove( baseKey + mOriginalConnName );
  }
  baseKey += txtName->text();
  settings.setValue( baseKey + "/host", txtHost->text() );
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/port", txtPort->text() );
  settings.setValue( baseKey + "/username", txtUsername->text() );
  settings.setValue( baseKey + "/password", chkStorePassword->isChecked() ? txtPassword->text() : "" );
  settings.setValue( baseKey + "/publicOnly", cb_publicSchemaOnly->isChecked() );
  settings.setValue( baseKey + "/geometryColumnsOnly", cb_geometryColumnsOnly->isChecked() );
  settings.setValue( baseKey + "/save", chkStorePassword->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/sslmode", cbxSSLmode->currentIndex() );
}

#if 0
void QgsNewConnection::saveConnection()
{
  QSettings settings;
  QString baseKey = "/PostgreSQL/connections/";
  baseKey += txtName->text();
  settings.setValue( baseKey + "/host", txtHost->text() );
  settings.setValue( baseKey + "/database", txtDatabase->text() );

  settings.setValue( baseKey + "/username", txtUsername->text() );
  settings.setValue( baseKey + "/password", chkStorePassword->isChecked() ? txtPassword->text() : "" );
  accept();
}
#endif
