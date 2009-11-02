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
#include "qgsencodingfiledialog.h"
#include "qgsopenvectorlayerdialog.h"
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
  // cmbEncodings->setItemText( cmbEncodings->currentIndex(), QString( QTextCodec::codecForLocale()->name() ) );
  QSettings settings;
  QString enc = settings.value( "/UI/encoding", QString("System") ).toString();
  
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

  for ( int i = 0;i < dbDrivers.count();i++ )
  {
    QString dbDriver = dbDrivers.at( i );
    if (( !dbDriver.isEmpty() ) && ( !dbDriver.isNull() ) )
      cmbDatabaseTypes->addItem( dbDriver.split( "," ).at( 0 ) );
  }

  //add directory drivers
  QStringList dirDrivers = QgsProviderRegistry::instance()->directoryDrivers().split( ";" );
  for ( int i = 0;i < dirDrivers.count();i++ )
  {
    QString dirDriver = dirDrivers.at( i );
    if (( !dirDriver.isEmpty() ) && ( !dirDriver.isNull() ) )
      cmbDirectoryTypes->addItem( dirDriver.split( "," ).at( 0 ) );
  }

  //add protocol drivers
  QStringList proDrivers = QgsProviderRegistry::instance()->protocolDrivers().split( ";" );
  for ( int i = 0;i < proDrivers.count();i++ )
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
  QString enc;
  QString title = tr( "Open an OGR Supported Vector Layer" );
  openFilesRememberingFilter( "lastVectorFileFilter", mVectorFileFilter, selectedFiles,
                              title );
  mEnc = enc;
  return selectedFiles;
}

QString QgsOpenVectorLayerDialog::openDirectory()
{
  QSettings settings;

  bool haveLastUsedDir = settings.contains( "/UI/LastUsedDirectory" );
  QString lastUsedDir = settings.value( "/UI/LastUsedDirectory",
                                        QVariant( QString::null ) ).toString();
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

void QgsOpenVectorLayerDialog::helpInfo()
{
  QgsContextHelp::run( context_id );
}

QString QgsOpenVectorLayerDialog::dataSourceType()
{
  return mDataSourceType;
}

void QgsOpenVectorLayerDialog::addNewConnection()
{
  QgsNewOgrConnection *nc = new QgsNewOgrConnection( this );
  if ( nc->exec() )
  {
    populateConnectionList();
  }
}

void QgsOpenVectorLayerDialog::editConnection()
{
  QgsNewOgrConnection *nc = new QgsNewOgrConnection( this, cmbDatabaseTypes->currentText(), cmbConnections->currentText() );

  if ( nc->exec() )
  {
    nc->saveConnection();
  }
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
  bool set = false;
  for ( int i = 0; i < cmbDatabaseTypes->count(); ++i )
    if ( cmbDatabaseTypes->itemText( i ) == toSelect )
    {
      cmbDatabaseTypes->setCurrentIndex( i );
      set = true;
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
  QSettings settings;
  QString filepath;

  mDataSources.clear();

  if ( radioSrcFile->isChecked() )
  {
    //file

    //mType="file";
    mDataSources = openFile();
    filepath = "";
    for ( int i = 0;i < mDataSources.count();i++ )
      filepath += mDataSources.at( i ) + ";";
    inputSrcDataset->setText( filepath );
  }
  else if ( radioSrcDirectory->isChecked() )
  {

    filepath = openDirectory();
    mDataSources.append( filepath );
    inputSrcDataset->setText( filepath );
    //mType="directory";
  }
  else if ( radioSrcDatabase->isChecked() )
  {
    //mType="database";
    //src = inputSrcDataset->text();
  }
  else
  {
    Q_ASSERT( !"SHOULD NEVER GET HERE" );
  }



}

/**
  Open files, preferring to have the default file selector be the
  last one used, if any; also, prefer to start in the last directory
  associated with filterName.

  @param filterName the name of the filter; used for persistent store
  key
  @param filters    the file filters used for QFileDialog

  @param selectedFiles string list of selected files; will be empty
  if none selected
  @param title      the title for the dialog
  @note

  Stores persistent settings under /UI/.  The sub-keys will be
  filterName and filterName + "Dir".

  Opens dialog on last directory associated with the filter name, or
  the current working directory if this is the first time invoked
  with the current filter name.

*/
void QgsOpenVectorLayerDialog::openFilesRememberingFilter( QString const &filterName,
    QString const &filters, QStringList & selectedFiles, QString &title )
{

  bool haveLastUsedFilter = false; // by default, there is no last
  // used filter

  QSettings settings;         // where we keep last used filter in

  // persistant state

  haveLastUsedFilter = settings.contains( "/UI/" + filterName );
  QString lastUsedFilter = settings.value( "/UI/" + filterName,
                           QVariant( QString::null ) ).toString();

  QString lastUsedDir = settings.value( "/UI/" + filterName + "Dir", "." ).toString();

  //QString lastUsedEncoding = settings.value( "/UI/encoding" ).toString();

  QgsDebugMsg( "Opening file dialog with filters: " + filters );

  QFileDialog* openFileDialog = new QFileDialog( 0,
      title, lastUsedDir, filters );

  // allow for selection of more than one file
  openFileDialog->setFileMode( QFileDialog::ExistingFiles );

  if ( haveLastUsedFilter )     // set the filter to the last one used
  {
    openFileDialog->selectFilter( lastUsedFilter );
  }

  if ( openFileDialog->exec() == QDialog::Accepted )
  {
    selectedFiles = openFileDialog->selectedFiles();
    //enc = openFileDialog->encoding();
    // Fix by Tim - getting the dirPath from the dialog
    // directly truncates the last node in the dir path.
    // This is a workaround for that
    QString myFirstFileName = selectedFiles.first();
    QFileInfo myFI( myFirstFileName );
    QString myPath = myFI.path();

    QgsDebugMsg( "Writing last used dir: " + myPath );

    settings.setValue( "/UI/" + filterName, openFileDialog->selectedFilter() );
    settings.setValue( "/UI/" + filterName + "Dir", myPath );
    //settings.setValue( "/UI/encoding", openFileDialog->encoding() );
  }

  delete openFileDialog;

}   // openFilesRememberingFilter_



//********************auto connected slots *****************/
void QgsOpenVectorLayerDialog::on_buttonBox_accepted()
{
  QSettings settings;
  QgsDebugMsg( "dialog button accepted" );
  if ( radioSrcDatabase->isChecked() )
  {
    mDataSources.clear();
    QString baseKey = "/" + cmbDatabaseTypes->currentText() + "/connections/";
    baseKey += cmbConnections->currentText();
    QString host = settings.value( baseKey + "/host" ).toString();
    QString database = settings.value( baseKey + "/database" ).toString();
    QString port = settings.value( baseKey + "/port" ).toString();
    QString user = settings.value( baseKey + "/username" ).toString();
    QString pass = settings.value( baseKey + "/password" ).toString();
    bool makeConnection = false;
    if ( pass.isEmpty() )
      pass = QInputDialog::getText( this, tr( "Password for " ) + user,
                                    tr( "Please enter your password:" ),
                                    QLineEdit::Password, QString::null, &makeConnection );
    if ( makeConnection || ( !pass.isEmpty() ) )
      mDataSources.append( createDatabaseURI(
                             cmbDatabaseTypes->currentText(),
                             host,
                             database,
                             port,
                             user,
                             pass
                           ) );
  }
  else if ( radioSrcProtocol->isChecked() )
  {
    mDataSources.clear();
    mDataSources.append( createProtocolURI(
                           cmbProtocolTypes->currentText(),
                           protocolURI->text()
                         ) );
  }
  // Save the used encoding
  settings.setValue( "/UI/encoding", encoding() );

  accept();
}

void QgsOpenVectorLayerDialog::on_btnHelp_clicked()
{
  helpInfo();
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


