/***************************************************************************
  qgsdb2newconnection.cpp - new DB2 connection dialog
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xiaoshir at us.ibm.com, nguyend at us.ibm.com
/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>
#include <qgslogger.h>
#include <qlistwidget.h>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

#include "qgsdb2newconnection.h"
#include "qgsdb2provider.h"
#include "qgscontexthelp.h"

QgsDb2NewConnection::QgsDb2NewConnection( QWidget *parent, const QString& connName, Qt::WindowFlags fl )
    : QDialog( parent, fl ), mOriginalConnName( connName )
{
  setupUi( this );
  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QSettings settings;

    QString key = "/DB2/connections/" + connName;
    txtService->setText( settings.value( key + "/service" ).toString() );
    txtHost->setText( settings.value( key + "/host" ).toString() );
    txtPort->setText( settings.value( key + "/port" ).toString() );
    txtDriver->setText( settings.value( key + "/driver" ).toString() );
    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    txtUsername->setText( settings.value( key + "/username" ).toString() );
    if ( settings.value( key + "/savePassword" ).toString() == "true" )
    {
      txtPassword->setText( settings.value( key + "/password" ).toString() );
      savePassword->setChecked( true );
    }
    txtName->setText( connName );
    if ( settings.value( key + "/environment" ) == ENV_LUW )
      radioLuw->setDown( true );
    else
      radioZos->setDown( true );
  }

}

/** Autoconnected SLOTS **/
void QgsDb2NewConnection::accept()
{
  QSettings settings;
  QString baseKey = "/DB2/connections/";
  settings.setValue( baseKey + "selected", txtName->text() );

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
  settings.setValue( baseKey + "/driver", txtDriver->text() );
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/username", txtUsername->text() );
  settings.setValue( baseKey + "/password", savePassword->isChecked() ? txtPassword->text() : "" );
  settings.setValue( baseKey + "/savePassword", savePassword->isChecked() ? "true" : "false" );
  if ( radioLuw->isChecked() )
    settings.setValue( baseKey + "/environment", ENV_LUW );
  else
    settings.setValue( baseKey + "/environment", ENV_ZOS );

  QDialog::accept();
}

void QgsDb2NewConnection::on_btnConnect_clicked()
{
  QgsDebugMsg( "DB2: TestDatabase; button clicked" );
  testConnection();
}

void QgsDb2NewConnection::on_btnListDatabase_clicked()
{
  listDatabases();
}

void QgsDb2NewConnection::on_cb_trustedConnection_clicked()
{

}

/** End  Autoconnected SLOTS **/

QgsDb2NewConnection::~QgsDb2NewConnection()
{

}

bool QgsDb2NewConnection::testConnection( QString testDatabase )
{
  QSqlDatabase db;

  QString dsn;
  QString driver;
  QString host;
  QString port;
  QString database;
  QString userid;
  QString password;

  dsn =  txtService->text();
  driver = txtDriver->text();
  host = txtHost->text();
  port = txtPort->text();
  database = txtDatabase->text();
  userid = txtUsername->text();
  password = txtPassword->text();

  bool convertIntOk;
  int portNum = port.toInt( &convertIntOk, 10 );
  db = QgsDb2Provider::GetDatabase( dsn, driver, host, portNum, database, userid, password );
  if ( db.open() )
  {
    QgsDebugMsg("connection open succeeded on " + testDatabase );
    db2ConnectStatus -> setText( "DB2 connection open succeeded" );
    return true;
  }
  else
  {
    QgsDebugMsg( "DB2: TestDatabase; connection open failed: " + db.lastError().text() );
    db2ConnectStatus -> setText( "DB2 connection failed : " + db.lastError().text() );
    return false;
  }
}

void QgsDb2NewConnection::listDatabases()
{
  QgsDebugMsg( "DB2 New Connection Dialogue : list database" );
}
