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
#include <QSettings>
#include <QMessageBox>

#include "qgsnewogrconnection.h"
#include "qgscontexthelp.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"
#include "qgsogrhelperfunctions.h"
#include <ogr_api.h>
#include <cpl_error.h>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8F(x) (x).toUtf8().constData()
#else
#define TO8F(x) QFile::encodeName( x ).constData()
#endif

QgsNewOgrConnection::QgsNewOgrConnection( QWidget *parent, const QString& connType, const QString& connName, Qt::WFlags fl )
    : QDialog( parent, fl ),
    mOriginalConnName( connName )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/OGRDatabaseConnection/geometry" ).toByteArray() );

  //add database drivers
  QStringList dbDrivers = QgsProviderRegistry::instance()->databaseDrivers().split( ";" );
  for ( int i = 0; i < dbDrivers.count(); i++ )
  {
    QString dbDrive = dbDrivers.at( i );
    cmbDatabaseTypes->addItem( dbDrive.split( "," ).at( 0 ) );
  }
  txtName->setEnabled( true );
  cmbDatabaseTypes->setEnabled( true );
  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QString key = "/" + connType + "/connections/" + connName;
    txtHost->setText( settings.value( key + "/host" ).toString() );
    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    QString port = settings.value( key + "/port" ).toString();
    txtPort->setText( port );
    txtUsername->setText( settings.value( key + "/username" ).toString() );
    if ( settings.value( key + "/save" ).toString() == "true" )
    {
      txtPassword->setText( settings.value( key + "/password" ).toString() );
      chkStorePassword->setChecked( true );
    }
    cmbDatabaseTypes->setCurrentIndex( cmbDatabaseTypes->findText( connType ) );
    txtName->setText( connName );
    txtName->setEnabled( false );
    cmbDatabaseTypes->setEnabled( false );
  }
}

QgsNewOgrConnection::~QgsNewOgrConnection()
{
  QSettings settings;
  settings.setValue( "/Windows/OGRDatabaseConnection/geometry", saveGeometry() );
}

void QgsNewOgrConnection::testConnection()
{
  QString uri;
  uri = createDatabaseURI( cmbDatabaseTypes->currentText(), txtHost->text(),
                           txtDatabase->text(), txtPort->text(),
                           txtUsername->text(), txtPassword->text() );
  QgsDebugMsg( "Connecting using uri = " + uri );
  OGRRegisterAll();
  OGRDataSourceH       poDS;
  OGRSFDriverH         pahDriver;
  CPLErrorReset();
  poDS = OGROpen( TO8F( uri ), false, &pahDriver );
  if ( poDS == NULL )
  {
    QMessageBox::information( this, tr( "Test connection" ), tr( "Connection failed - Check settings and try again.\n\nExtended error information:\n%1" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
  }
  else
  {
    QMessageBox::information( this, tr( "Test connection" ), tr( "Connection to %1 was successful" ).arg( uri ) );
    OGRReleaseDataSource( poDS );
  }
}

/** Autoconnected SLOTS **/
void QgsNewOgrConnection::accept()
{
  QSettings settings;
  QString baseKey = "/" + cmbDatabaseTypes->currentText() + "/connections/";
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

  // on rename delete original entry first
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
  settings.setValue( baseKey + "/save", chkStorePassword->isChecked() ? "true" : "false" );

  QDialog::accept();
}

void QgsNewOgrConnection::on_btnConnect_clicked()
{
  testConnection();
}

/** end  Autoconnected SLOTS **/
