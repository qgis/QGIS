/***************************************************************************
                          qgsopenvectorlayerdialog.cpp
 Dialog to select the type and source for ogr vectors, supports
 file, database, directory and protocol sources.
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
/* $Id:$ */
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextCodec>

#include "qgslogger.h"
#include "qgsopenvectorlayerdialog.h"
#include "qgsvectordataprovider.h"

#include <ogr_api.h>
#include "qgsproviderregistry.h"
#include "qgsnewogrconnection.h"
#include "qgsogrhelperfunctions.h"
#include "qgscontexthelp.h"

QgsOpenVectorLayerDialog::QgsOpenVectorLayerDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  cmbDatabaseTypes->blockSignals( true );
  cmbConnections->blockSignals( true );
  radioSrcFile->setChecked( true );
  mDataSourceType = "file";

  //set encoding
  cmbEncodings->addItems( QgsVectorDataProvider::availableEncodings() );

  QSettings settings;
  QString enc = settings.value( "/UI/encoding", QString( "System" ) ).toString();

  // The specified decoding is added if not existing alread, and then set current.
  // This should select it.
  int encindex = cmbEncodings->findText( enc );
  if ( encindex < 0 )
  {
    cmbEncodings->insertItem( 0, enc );
    encindex = 0;
  }
  cmbEncodings->setCurrentIndex( encindex );

  //add database drivers
  mVectorFileFilter = QgsProviderRegistry::instance()->fileVectorFilters();
  QgsDebugMsg( "Database drivers :" + QgsProviderRegistry::instance()->databaseDrivers() );
  QStringList dbDrivers = QgsProviderRegistry::instance()->databaseDrivers().split( ";" );

  for ( int i = 0; i < dbDrivers.count(); i++ )
  {
    QString dbDriver = dbDrivers.at( i );
    if (( !dbDriver.isEmpty() ) && ( !dbDriver.isNull() ) )
      cmbDatabaseTypes->addItem( dbDriver.split( "," ).at( 0 ) );
  }

  //add directory drivers
  QStringList dirDrivers = QgsProviderRegistry::instance()->directoryDrivers().split( ";" );
  for ( int i = 0; i < dirDrivers.count(); i++ )
  {
    QString dirDriver = dirDrivers.at( i );
    if (( !dirDriver.isEmpty() ) && ( !dirDriver.isNull() ) )
      cmbDirectoryTypes->addItem( dirDriver.split( "," ).at( 0 ) );
  }

  //add protocol drivers
  QStringList proDrivers = QgsProviderRegistry::instance()->protocolDrivers().split( ";" );
  for ( int i = 0; i < proDrivers.count(); i++ )
  {
    QString proDriver = proDrivers.at( i );
    if (( !proDriver.isEmpty() ) && ( !proDriver.isNull() ) )
      cmbProtocolTypes->addItem( proDriver.split( "," ).at( 0 ) );
  }
  cmbDatabaseTypes->blockSignals( false );
  cmbConnections->blockSignals( false );
}

QgsOpenVectorLayerDialog::~QgsOpenVectorLayerDialog()
{
}

QStringList QgsOpenVectorLayerDialog::openFile()
{
  QStringList selectedFiles;
  QgsDebugMsg( "Vector file filters: " + mVectorFileFilter );
  QString enc = encoding();
  QString title = tr( "Open an OGR Supported Vector Layer" );
  QgisGui::openFilesRememberingFilter( "lastVectorFileFilter", mVectorFileFilter, selectedFiles, enc, title );

  return selectedFiles;
}

QString QgsOpenVectorLayerDialog::openDirectory()
{
  QSettings settings;

  bool haveLastUsedDir = settings.contains( "/UI/LastUsedDirectory" );
  QString lastUsedDir = settings.value( "/UI/LastUsedDirectory", QVariant() ).toString();
  if ( !haveLastUsedDir )
    lastUsedDir = "";

  QString path = QFileDialog::getExistingDirectory( this,
                 tr( "Open Directory" ), lastUsedDir,
                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

  settings.setValue( "/UI/LastUsedDirectory", path );
  //process path if it is grass
  if ( cmbDirectoryTypes->currentText() == "Grass Vector" )
  {
#ifdef WIN32
    //replace backslashes with forward slashes
    path.replace( "\\", "/" );
#endif
    path = path + "/head";
  }
  return path;
}

QStringList QgsOpenVectorLayerDialog::dataSources()
{
  return mDataSources;
}

QString QgsOpenVectorLayerDialog::encoding()
{
  return cmbEncodings->currentText();
}

QString QgsOpenVectorLayerDialog::dataSourceType()
{
  return mDataSourceType;
}

void QgsOpenVectorLayerDialog::addNewConnection()
{
  QgsNewOgrConnection *nc = new QgsNewOgrConnection( this );
  nc->exec();
  delete nc;

  populateConnectionList();
}

void QgsOpenVectorLayerDialog::editConnection()
{
  QgsNewOgrConnection *nc = new QgsNewOgrConnection( this, cmbDatabaseTypes->currentText(), cmbConnections->currentText() );
  nc->exec();
  delete nc;

  populateConnectionList();
}

void QgsOpenVectorLayerDialog::deleteConnection()
{
  QSettings settings;
  QString key = "/" + cmbDatabaseTypes->currentText() + "/connections/" + cmbConnections->currentText();
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    settings.remove( key + "/host" );
    settings.remove( key + "/database" );
    settings.remove( key + "/username" );
    settings.remove( key + "/password" );
    settings.remove( key + "/port" );
    settings.remove( key + "/save" );
    settings.remove( key );
    cmbConnections->removeItem( cmbConnections->currentIndex() );  // populateConnectionList();
    setConnectionListPosition();
  }
}

void QgsOpenVectorLayerDialog::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/" + cmbDatabaseTypes->currentText() + "/connections" );
  QStringList keys = settings.childGroups();
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while ( it != keys.end() )
  {
    cmbConnections->addItem( *it );
    ++it;
  }
  settings.endGroup();
  setConnectionListPosition();
}

void QgsOpenVectorLayerDialog::setConnectionListPosition()
{
  QSettings settings;
  // If possible, set the item currently displayed database

  QString toSelect = settings.value( "/" + cmbDatabaseTypes->currentText() + "/connections/selected" ).toString();
  // Does toSelect exist in cmbConnections?
  bool set = false;
  for ( int i = 0; i < cmbConnections->count(); ++i )
    if ( cmbConnections->itemText( i ) == toSelect )
    {
      cmbConnections->setCurrentIndex( i );
      set = true;
      break;
    }
  // If we couldn't find the stored item, but there are some,
  // default to the last item (this makes some sense when deleting
  // items as it allows the user to repeatidly click on delete to
  // remove a whole lot of items).
  if ( !set && cmbConnections->count() > 0 )
  {
    // If toSelect is null, then the selected connection wasn't found
    // by QSettings, which probably means that this is the first time
    // the user has used qgis with database connections, so default to
    // the first in the list of connetions. Otherwise default to the last.
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsOpenVectorLayerDialog::setConnectionTypeListPosition()
{
  QSettings settings;

  QString toSelect = settings.value( "/ogr/connections/selectedtype" ).toString();
  for ( int i = 0; i < cmbDatabaseTypes->count(); ++i )
    if ( cmbDatabaseTypes->itemText( i ) == toSelect )
    {
      cmbDatabaseTypes->setCurrentIndex( i );
      break;
    }
}

void QgsOpenVectorLayerDialog::setSelectedConnectionType()
{
  QSettings settings;
  QString baseKey = "/ogr/connections/";
  settings.setValue( baseKey + "selectedtype", cmbDatabaseTypes->currentText() );
  QgsDebugMsg( "Setting selected type to" + cmbDatabaseTypes->currentText() );
}

void QgsOpenVectorLayerDialog::setSelectedConnection()
{
  QSettings settings;
  settings.setValue( "/" + cmbDatabaseTypes->currentText() + "/connections/selected", cmbConnections->currentText() );
  QgsDebugMsg( "Setting selected connection to " + cmbConnections->currentText() );
}


void QgsOpenVectorLayerDialog::on_buttonSelectSrc_clicked()
{
  if ( radioSrcFile->isChecked() )
  {
    inputSrcDataset->setText( openFile().join( ";" ) );
  }
  else if ( radioSrcDirectory->isChecked() )
  {
    inputSrcDataset->setText( openDirectory() );
  }
  else if ( !radioSrcDatabase->isChecked() )
  {
    Q_ASSERT( !"SHOULD NEVER GET HERE" );
  }
}



//********************auto connected slots *****************/
void QgsOpenVectorLayerDialog::accept()
{
  QSettings settings;
  QgsDebugMsg( "dialog button accepted" );

  mDataSources.clear();

  if ( radioSrcDatabase->isChecked() )
  {
    if ( !settings.contains( "/" + cmbDatabaseTypes->currentText()
                             + "/connections/" + cmbConnections->currentText()
                             + "/host" ) )
    {
      QMessageBox::information( this,
                                tr( "Add vector layer" ),
                                tr( "No database selected." ) );
      return;
    }

    QString baseKey = "/" + cmbDatabaseTypes->currentText() + "/connections/";
    baseKey += cmbConnections->currentText();
    QString host = settings.value( baseKey + "/host" ).toString();
    QString database = settings.value( baseKey + "/database" ).toString();
    QString port = settings.value( baseKey + "/port" ).toString();
    QString user = settings.value( baseKey + "/username" ).toString();
    QString pass = settings.value( baseKey + "/password" ).toString();

    bool makeConnection = false;
    if ( pass.isEmpty() )
    {
      pass = QInputDialog::getText( this,
                                    tr( "Password for " ) + user,
                                    tr( "Please enter your password:" ),
                                    QLineEdit::Password, QString::null,
                                    &makeConnection );
    }

    if ( makeConnection || !pass.isEmpty() )
    {
      mDataSources << createDatabaseURI(
        cmbDatabaseTypes->currentText(),
        host,
        database,
        port,
        user,
        pass
      );
    }
  }
  else if ( radioSrcProtocol->isChecked() )
  {
    if ( protocolURI->text().isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Add vector layer" ),
                                tr( "No protocol URI entered." ) );
      return;
    }

    mDataSources << createProtocolURI( cmbProtocolTypes->currentText(), protocolURI->text() );
  }
  else if ( radioSrcFile->isChecked() )
  {
    if ( inputSrcDataset->text().isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Add vector layer" ),
                                tr( "No layers selected." ) );
      return;
    }

    mDataSources << inputSrcDataset->text().split( ";" );
  }
  else if ( radioSrcDirectory->isChecked() )
  {
    if ( inputSrcDataset->text().isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Add vector layer" ),
                                tr( "No directory selected." ) );
      return;
    }

    mDataSources << inputSrcDataset->text();
  }

  // Save the used encoding
  settings.setValue( "/UI/encoding", encoding() );

  QDialog::accept();
}

void QgsOpenVectorLayerDialog::on_radioSrcFile_toggled( bool checked )
{
  if ( checked )
  {
    labelDirectoryType->hide();
    cmbDirectoryTypes->hide();
    fileGroupBox->show();
    dbGroupBox->hide();
    protocolGroupBox->hide();
    layout()->setSizeConstraint( QLayout::SetFixedSize );
    mDataSourceType = "file";
  }
}

void QgsOpenVectorLayerDialog::on_radioSrcDirectory_toggled( bool checked )
{
  if ( checked )
  {
    labelDirectoryType->show();
    cmbDirectoryTypes->show();
    fileGroupBox->show();
    dbGroupBox->hide();
    protocolGroupBox->hide();
    layout()->setSizeConstraint( QLayout::SetFixedSize );
    mDataSourceType = "directory";
  }
}

void QgsOpenVectorLayerDialog::on_radioSrcDatabase_toggled( bool checked )
{
  if ( checked )
  {
    layout()->blockSignals( true );
    fileGroupBox->hide();
    protocolGroupBox->hide();
    dbGroupBox->show();
    layout()->blockSignals( false );
    layout()->setSizeConstraint( QLayout::SetFixedSize );
    setConnectionTypeListPosition();
    populateConnectionList();
    setConnectionListPosition();
    mDataSourceType = "database";
  }
}

void QgsOpenVectorLayerDialog::on_radioSrcProtocol_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->hide();
    dbGroupBox->hide();
    protocolGroupBox->show();
    layout()->setSizeConstraint( QLayout::SetFixedSize );
    mDataSourceType = "protocol";
  }
}

// Slot for adding a new connection
void QgsOpenVectorLayerDialog::on_btnNew_clicked()
{
  addNewConnection();
}
// Slot for deleting an existing connection
void QgsOpenVectorLayerDialog::on_btnDelete_clicked()
{
  deleteConnection();
}


// Slot for editing a connection
void QgsOpenVectorLayerDialog::on_btnEdit_clicked()
{
  editConnection();
}

void QgsOpenVectorLayerDialog::on_cmbDatabaseTypes_currentIndexChanged( const QString & text )
{
  populateConnectionList();
  setSelectedConnectionType();
}

void QgsOpenVectorLayerDialog::on_cmbConnections_currentIndexChanged( const QString & text )
{
  setSelectedConnection();
}
//********************end auto connected slots *****************/


