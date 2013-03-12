/***************************************************************************
  qgsosmimportdialog.cpp
  --------------------------------------
  Date                 : February 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsosmimportdialog.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "qgsosmimport.h"

QgsOSMImportDialog::QgsOSMImportDialog( QWidget* parent )
    : QDialog( parent ), mImport( new QgsOSMXmlImport )
{
  setupUi( this );

  connect( btnBrowseXml, SIGNAL( clicked() ), this, SLOT( onBrowseXml() ) );
  connect( btnBrowseDb, SIGNAL( clicked() ), this, SLOT( onBrowseDb() ) );
  connect( editXmlFileName, SIGNAL( textChanged( const QString& ) ), this, SLOT( xmlFileNameChanged( const QString& ) ) );
  connect( editDbFileName, SIGNAL( textChanged( const QString& ) ), this, SLOT( dbFileNameChanged( const QString& ) ) );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( onOK() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( onClose() ) );

  connect( mImport, SIGNAL( progress( int ) ), this, SLOT( onProgress( int ) ) );
}

QgsOSMImportDialog::~QgsOSMImportDialog()
{
  delete mImport;
}


void QgsOSMImportDialog::onBrowseXml()
{
  QSettings settings;
  QString lastDir = settings.value( "/osm/lastDir" ).toString();

  QString fileName = QFileDialog::getOpenFileName( this, QString(), lastDir, tr( "OpenStreetMap files (*.osm)" ) );
  if ( fileName.isNull() )
    return;

  settings.setValue( "/osm/lastDir", QFileInfo( fileName ).absolutePath() );
  editXmlFileName->setText( fileName );
}

void QgsOSMImportDialog::onBrowseDb()
{
  QSettings settings;
  QString lastDir = settings.value( "/osm/lastDir" ).toString();

  QString fileName = QFileDialog::getSaveFileName( this, QString(), lastDir, tr( "SQLite databases (*.db)" ) );
  if ( fileName.isNull() )
    return;

  settings.setValue( "/osm/lastDir", QFileInfo( fileName ).absolutePath() );
  editDbFileName->setText( fileName );
}


void QgsOSMImportDialog::xmlFileNameChanged( const QString& fileName )
{
  editDbFileName->setText( fileName + ".db" );
}

void QgsOSMImportDialog::dbFileNameChanged( const QString& fileName )
{
  editConnName->setText( QFileInfo( fileName ).baseName() );
}

void QgsOSMImportDialog::onOK()
{
  // output file exists?
  if ( QFileInfo( editDbFileName->text() ).exists() )
  {
    int res = QMessageBox::question( this, tr( "OpenStreetMap import" ), tr( "Output database file exists already. Overwrite?" ), QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
      return;
  }

  mImport->setInputXmlFileName( editXmlFileName->text() );
  mImport->setOutputDbFileName( editDbFileName->text() );

  buttonBox->setEnabled( false );
  QApplication::setOverrideCursor( Qt::WaitCursor );

  bool res = mImport->import();

  QApplication::restoreOverrideCursor();
  buttonBox->setEnabled( true );

  progressBar->setValue( 0 );

  if ( !res )
  {
    QMessageBox::critical( this, tr( "OpenStreetMap import" ), tr( "Failed to import OSM data:\n%1" ).arg( mImport->errorString() ) );
    return;
  }

  if ( groupCreateConn->isChecked() )
  {
    // create connection - this is a bit hacky, sorry for that.
    QSettings settings;
    settings.setValue( QString( "/SpatiaLite/connections/%1/sqlitepath" ).arg( editConnName->text() ), mImport->outputDbFileName() );
  }

  QMessageBox::information( this, tr( "OpenStreetMap import" ), tr( "Import has been successful." ) );
}

void QgsOSMImportDialog::onClose()
{
  reject();
}

void QgsOSMImportDialog::onProgress( int percent )
{
  progressBar->setValue( percent );
  qApp->processEvents( QEventLoop::ExcludeSocketNotifiers );
}
