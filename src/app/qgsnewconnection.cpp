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
#include <QInputDialog>

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

    cbxSSLmode->setCurrentIndex( cbxSSLmode->findData( settings.value( key + "/sslmode", QgsDataSourceURI::SSLprefer ).toInt() ) );

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
  QSettings settings;
  QString baseKey = "/PostgreSQL/connections/";
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
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/port", txtPort->text() );
  settings.setValue( baseKey + "/username", txtUsername->text() );
  settings.setValue( baseKey + "/password", chkStorePassword->isChecked() ? txtPassword->text() : "" );
  settings.setValue( baseKey + "/publicOnly", cb_publicSchemaOnly->isChecked() );
  settings.setValue( baseKey + "/geometryColumnsOnly", cb_geometryColumnsOnly->isChecked() );
  settings.setValue( baseKey + "/save", chkStorePassword->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/sslmode", cbxSSLmode->itemData( cbxSSLmode->currentIndex() ).toInt() );

  QDialog::accept();
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
  uri.setConnection( txtHost->text(), txtPort->text(), txtDatabase->text(), txtUsername->text(), txtPassword->text(), ( QgsDataSourceURI::SSLmode ) cbxSSLmode->itemData( cbxSSLmode->currentIndex() ).toInt() );

  QgsDebugMsg( "PQconnectdb(\"" + uri.connectionInfo() + "\");" );

  PGconn *pd = PQconnectdb( uri.connectionInfo().toLocal8Bit() );  // use what is set based on locale; after connecting, use Utf8
  // check the connection status
  if ( PQstatus( pd ) != CONNECTION_OK && QString::fromUtf8( PQerrorMessage( pd ) ) == PQnoPasswordSupplied )
  {
    QString password = QString::null;

    while( PQstatus( pd ) != CONNECTION_OK )
    {
      bool ok = true;
      password = QInputDialog::getText( this,
                                        tr( "Enter password" ),
                                        tr( "Error: %1Enter password for %2")
                                          .arg( QString::fromUtf8( PQerrorMessage( pd ) ) )
                                          .arg( uri.connectionInfo() ),
                                        QLineEdit::Password,
                                        password,
                                        &ok );

      ::PQfinish( pd );

      if( !ok )
        break;

      pd = PQconnectdb( QString( "%1 password='%2'" ).arg( uri.connectionInfo() ).arg( password ).toLocal8Bit() );
    }
  }

  if ( PQstatus( pd ) == CONNECTION_OK  )
  {
    // Database successfully opened; we can now issue SQL commands.
    QMessageBox::information( this, tr( "Test connection" ), tr( "Connection to %1 was successful" ).arg( txtDatabase->text() ) );
  }
  else
  {
    QMessageBox::information( this, tr( "Test connection" ), tr( "Connection failed - Check settings and try again.\n\nExtended error information:\n%1" ).arg( QString::fromUtf8( PQerrorMessage( pd ) ) ) );
  }
  // free pg connection resources
  PQfinish( pd );
}
