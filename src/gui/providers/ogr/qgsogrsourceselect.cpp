/***************************************************************************
                          qgsogrsourceselect.cpp
 Dialog to select the type and source for ogr vectors, supports
 file, database, directory and protocol sources.
                             -------------------
    ---------------------
    begin                : Aug 05, 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsogrsourceselect.h"
///@cond PRIVATE

#include <QMessageBox>
#include <QInputDialog>
#include <QTextCodec>

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsvectordataprovider.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"
#include "ogr/qgsnewogrconnection.h"
#include "ogr/qgsogrhelperfunctions.h"
#include "qgsgui.h"

#include <gdal.h>

QgsOgrSourceSelect::QgsOgrSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  QgsGui::instance()->enableAutoGeometryRestore( this );

  connect( radioSrcFile, &QRadioButton::toggled, this, &QgsOgrSourceSelect::radioSrcFile_toggled );
  connect( radioSrcDirectory, &QRadioButton::toggled, this, &QgsOgrSourceSelect::radioSrcDirectory_toggled );
  connect( radioSrcDatabase, &QRadioButton::toggled, this, &QgsOgrSourceSelect::radioSrcDatabase_toggled );
  connect( radioSrcProtocol, &QRadioButton::toggled, this, &QgsOgrSourceSelect::radioSrcProtocol_toggled );
  connect( btnNew, &QPushButton::clicked, this, &QgsOgrSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsOgrSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsOgrSourceSelect::btnDelete_clicked );
  connect( cmbDatabaseTypes, &QComboBox::currentTextChanged, this, &QgsOgrSourceSelect::cmbDatabaseTypes_currentIndexChanged );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsOgrSourceSelect::cmbConnections_currentIndexChanged );
  connect( cmbProtocolTypes, &QComboBox::currentTextChanged, this, &QgsOgrSourceSelect::cmbProtocolTypes_currentIndexChanged );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsOgrSourceSelect::showHelp );

  if ( mWidgetMode != QgsProviderRegistry::WidgetMode::None )
  {
    this->layout()->setSizeConstraint( QLayout::SetNoConstraint );
  }

  cmbDatabaseTypes->blockSignals( true );
  cmbConnections->blockSignals( true );
  radioSrcFile->setChecked( true );
  mDataSourceType = QStringLiteral( "file" );

  //set encoding
  cmbEncodings->addItems( QgsVectorDataProvider::availableEncodings() );

  QgsSettings settings;
  QString enc = settings.value( QStringLiteral( "UI/encoding" ), "System" ).toString();

  // The specified decoding is added if not existing already, and then set current.
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
  QStringList dbDrivers = QgsProviderRegistry::instance()->databaseDrivers().split( ';' );

  for ( int i = 0; i < dbDrivers.count(); i++ )
  {
    QString dbDriver = dbDrivers.at( i );
    if ( ( !dbDriver.isEmpty() ) && ( !dbDriver.isNull() ) )
      cmbDatabaseTypes->addItem( dbDriver.split( ',' ).at( 0 ) );
  }

  //add directory drivers
  QStringList dirDrivers = QgsProviderRegistry::instance()->directoryDrivers().split( ';' );
  for ( int i = 0; i < dirDrivers.count(); i++ )
  {
    QString dirDriver = dirDrivers.at( i );
    if ( ( !dirDriver.isEmpty() ) && ( !dirDriver.isNull() ) )
      cmbDirectoryTypes->addItem( dirDriver.split( ',' ).at( 0 ) );
  }

  //add protocol drivers
  QStringList protocolTypes = QStringLiteral( "HTTP/HTTPS/FTP,vsicurl;AWS S3,vsis3;Google Cloud Storage,vsigs;" ).split( ';' );
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,3,0)
  protocolTypes += QStringLiteral( "Microsoft Azure Blob,vsiaz;Alibaba Cloud OSS,vsioss;OpenStack Swift Object Storage,vsiswift;WFS3 (experimental),WFS3" ).split( ';' );
#endif
  protocolTypes += QgsProviderRegistry::instance()->protocolDrivers().split( ';' );
  for ( int i = 0; i < protocolTypes.count(); i++ )
  {
    QString protocolType = protocolTypes.at( i );
    if ( ( !protocolType.isEmpty() ) && ( !protocolType.isNull() ) )
      cmbProtocolTypes->addItem( protocolType.split( ',' ).at( 0 ) );
  }
  cmbDatabaseTypes->blockSignals( false );
  cmbConnections->blockSignals( false );

  mAuthWarning->setText( tr( " Additional credential options are required as documented <a href=\"%1\">here</a>." ).arg( QStringLiteral( "http://gdal.org/gdal_virtual_file_systems.html#gdal_virtual_file_systems_network" ) ) );

  mFileWidget->setDialogTitle( tr( "Open OGR Supported Vector Dataset(s)" ) );
  mFileWidget->setFilter( mVectorFileFilter );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );

  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mVectorPath = path;
    if ( radioSrcFile->isChecked() || radioSrcDirectory->isChecked() )
      emit enableButtons( ! mVectorPath.isEmpty() );
  } );

  connect( protocolURI, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    if ( radioSrcProtocol->isChecked() )
      emit enableButtons( !text.isEmpty() );
  } );
  connect( mBucket, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    if ( radioSrcProtocol->isChecked() )
      emit enableButtons( !text.isEmpty() && !mKey->text().isEmpty() );
  } );
  connect( mKey, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    if ( radioSrcProtocol->isChecked() )
      emit enableButtons( !text.isEmpty() && !mBucket->text().isEmpty() );
  } );
  // Set filter for ogr compatible auth methods
  mAuthSettingsProtocol->setDataprovider( QStringLiteral( "ogr" ) );
}

QStringList QgsOgrSourceSelect::dataSources()
{
  return mDataSources;
}

QString QgsOgrSourceSelect::encoding()
{
  return cmbEncodings->currentText();
}

QString QgsOgrSourceSelect::dataSourceType()
{
  return mDataSourceType;
}

bool QgsOgrSourceSelect::isProtocolCloudType()
{
  return ( cmbProtocolTypes->currentText() == QStringLiteral( "AWS S3" ) ||
           cmbProtocolTypes->currentText() == QStringLiteral( "Google Cloud Storage" ) ||
           cmbProtocolTypes->currentText() == QStringLiteral( "Microsoft Azure Blob" ) ||
           cmbProtocolTypes->currentText() == QStringLiteral( "Alibaba Cloud OSS" ) ||
           cmbProtocolTypes->currentText() == QStringLiteral( "OpenStack Swift Object Storage" ) );
}

void QgsOgrSourceSelect::addNewConnection()
{
  QgsNewOgrConnection *nc = new QgsNewOgrConnection( this );
  nc->exec();
  delete nc;

  populateConnectionList();
}

void QgsOgrSourceSelect::editConnection()
{
  QgsNewOgrConnection *nc = new QgsNewOgrConnection( this, cmbDatabaseTypes->currentText(), cmbConnections->currentText() );
  nc->exec();
  delete nc;

  populateConnectionList();
}

void QgsOgrSourceSelect::deleteConnection()
{
  QgsSettings settings;
  QString key = '/' + cmbDatabaseTypes->currentText() + "/connections/" + cmbConnections->currentText();
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No );
  if ( result == QMessageBox::Yes )
  {
    settings.remove( key + "/host" );
    settings.remove( key + "/database" );
    settings.remove( key + "/username" );
    settings.remove( key + "/password" );
    settings.remove( key + "/port" );
    settings.remove( key + "/save" );
    settings.remove( key + "/autchcfg" );
    settings.remove( key );
    cmbConnections->removeItem( cmbConnections->currentIndex() );  // populateConnectionList();
    setConnectionListPosition();
  }
}

void QgsOgrSourceSelect::populateConnectionList()
{
  QgsSettings settings;
  settings.beginGroup( '/' + cmbDatabaseTypes->currentText() + "/connections" );
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

void QgsOgrSourceSelect::setConnectionListPosition()
{
  QgsSettings settings;
  // If possible, set the item currently displayed database

  QString toSelect = settings.value( '/' + cmbDatabaseTypes->currentText() + "/connections/selected" ).toString();
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
    // by QgsSettings, which probably means that this is the first time
    // the user has used qgis with database connections, so default to
    // the first in the list of connetions. Otherwise default to the last.
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsOgrSourceSelect::setConnectionTypeListPosition()
{
  QgsSettings settings;

  QString toSelect = settings.value( QStringLiteral( "ogr/connections/selectedtype" ) ).toString();
  for ( int i = 0; i < cmbDatabaseTypes->count(); ++i )
    if ( cmbDatabaseTypes->itemText( i ) == toSelect )
    {
      cmbDatabaseTypes->setCurrentIndex( i );
      break;
    }
}

void QgsOgrSourceSelect::setSelectedConnectionType()
{
  QgsSettings settings;
  QString baseKey = QStringLiteral( "/ogr/connections/" );
  settings.setValue( baseKey + "selectedtype", cmbDatabaseTypes->currentText() );
  QgsDebugMsg( "Setting selected type to" + cmbDatabaseTypes->currentText() );
}

void QgsOgrSourceSelect::setSelectedConnection()
{
  QgsSettings settings;
  settings.setValue( '/' + cmbDatabaseTypes->currentText() + "/connections/selected", cmbConnections->currentText() );
  QgsDebugMsg( "Setting selected connection to " + cmbConnections->currentText() );
}

void QgsOgrSourceSelect::setProtocolWidgetsVisibility()
{
  if ( isProtocolCloudType() )
  {
    labelProtocolURI->hide();
    protocolURI->hide();
    mAuthGroupBox->hide();
    labelBucket->show();
    mBucket->show();
    labelKey->show();
    mKey->show();
    mAuthWarning->show();
  }
  else
  {
    labelProtocolURI->show();
    protocolURI->show();
    mAuthGroupBox->show();
    labelBucket->hide();
    mBucket->hide();
    labelKey->hide();
    mKey->hide();
    mAuthWarning->hide();
  }
}

void QgsOgrSourceSelect::addButtonClicked()
{
  QgsSettings settings;
  mDataSources.clear();

  if ( radioSrcDatabase->isChecked() )
  {
    if ( !settings.contains( '/' + cmbDatabaseTypes->currentText()
                             + "/connections/" + cmbConnections->currentText()
                             + "/host" ) )
    {
      QMessageBox::information( this,
                                tr( "Add vector layer" ),
                                tr( "No database selected." ) );
      return;
    }

    QString baseKey = '/' + cmbDatabaseTypes->currentText() + "/connections/";
    baseKey += cmbConnections->currentText();
    QString host = settings.value( baseKey + "/host" ).toString();
    QString database = settings.value( baseKey + "/database" ).toString();
    QString port = settings.value( baseKey + "/port" ).toString();
    QString user = settings.value( baseKey + "/username" ).toString();
    QString pass = settings.value( baseKey + "/password" ).toString();
    QString configid = settings.value( baseKey + "/configid" ).toString();

    bool makeConnection = false;
    if ( pass.isEmpty() && configid.isEmpty( ) )
    {
      if ( cmbDatabaseTypes->currentText() == QLatin1String( "MSSQL" ) )
        makeConnection = true;
      else
        pass = QInputDialog::getText( this,
                                      tr( "Password for " ) + user,
                                      tr( "Please enter your password:" ),
                                      QLineEdit::Password, QString(),
                                      &makeConnection );
    }

    if ( makeConnection || !( pass.isEmpty() && configid.isEmpty( ) ) )
    {
      mDataSources << createDatabaseURI(
                     cmbDatabaseTypes->currentText(),
                     host,
                     database,
                     port,
                     configid,
                     user,
                     pass
                   );
    }
  }
  else if ( radioSrcProtocol->isChecked() )
  {
    bool cloudType = isProtocolCloudType();
    if ( !cloudType && protocolURI->text().isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Add vector layer" ),
                                tr( "No protocol URI entered." ) );
      return;
    }
    else if ( cloudType && ( mBucket->text().isEmpty() || mKey->text().isEmpty() ) )
    {
      QMessageBox::information( this,
                                tr( "Add vector layer" ),
                                tr( "No protocol bucket and/or key entered." ) );
      return;
    }

    QString uri;
    if ( cloudType )
    {
      uri = QStringLiteral( "%1/%2" ).arg( mBucket->text(), mKey->text() );
    }
    else
    {
      uri = protocolURI->text();
    }

    mDataSources << createProtocolURI( cmbProtocolTypes->currentText(),
                                       uri,
                                       mAuthSettingsProtocol->configId(),
                                       mAuthSettingsProtocol->username(),
                                       mAuthSettingsProtocol->password() );
  }
  else if ( radioSrcFile->isChecked() )
  {
    if ( mVectorPath.isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Add vector layer" ),
                                tr( "No layers selected." ) );
      return;
    }

    mDataSources << QgsFileWidget::splitFilePaths( mVectorPath );
  }
  else if ( radioSrcDirectory->isChecked() )
  {
    if ( mVectorPath.isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Add vector layer" ),
                                tr( "No directory selected." ) );
      return;
    }

    //process path if it is grass
    if ( cmbDirectoryTypes->currentText() == QLatin1String( "Grass Vector" ) )
    {
#ifdef Q_OS_WIN
      //replace backslashes with forward slashes
      mVectorPath.replace( '\\', '/' );
#endif
      mVectorPath = mVectorPath + "/head";
    }

    mDataSources << mVectorPath;
  }

  // Save the used encoding
  settings.setValue( QStringLiteral( "UI/encoding" ), encoding() );

  if ( ! mDataSources.isEmpty() )
  {
    emit addVectorLayers( mDataSources, encoding(), dataSourceType() );
  }
}

//********************auto connected slots *****************/

void QgsOgrSourceSelect::radioSrcFile_toggled( bool checked )
{
  if ( checked )
  {
    labelDirectoryType->hide();
    cmbDirectoryTypes->hide();
    fileGroupBox->show();
    dbGroupBox->hide();
    protocolGroupBox->hide();

    mFileWidget->setDialogTitle( tr( "Open an OGR Supported Vector Layer" ) );
    mFileWidget->setFilter( mVectorFileFilter );
    mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
    mFileWidget->setFilePath( QString() );

    mDataSourceType = QStringLiteral( "file" );

    emit enableButtons( ! mFileWidget->filePath().isEmpty() );
  }
}

void QgsOgrSourceSelect::radioSrcDirectory_toggled( bool checked )
{
  if ( checked )
  {
    labelDirectoryType->show();
    cmbDirectoryTypes->show();
    fileGroupBox->show();
    dbGroupBox->hide();
    protocolGroupBox->hide();

    mFileWidget->setDialogTitle( tr( "Open Directory" ) );
    mFileWidget->setStorageMode( QgsFileWidget::GetDirectory );
    mFileWidget->setFilePath( QString() );

    mDataSourceType = QStringLiteral( "directory" );

    emit enableButtons( ! mFileWidget->filePath().isEmpty() );
  }
}

void QgsOgrSourceSelect::radioSrcDatabase_toggled( bool checked )
{
  if ( checked )
  {
    layout()->blockSignals( true );
    fileGroupBox->hide();
    protocolGroupBox->hide();
    dbGroupBox->show();
    layout()->blockSignals( false );
    setConnectionTypeListPosition();
    populateConnectionList();
    setConnectionListPosition();
    mDataSourceType = QStringLiteral( "database" );

    emit enableButtons( true );
  }
}

void QgsOgrSourceSelect::radioSrcProtocol_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->hide();
    dbGroupBox->hide();
    protocolGroupBox->show();
    mDataSourceType = QStringLiteral( "protocol" );

    setProtocolWidgetsVisibility();

    emit enableButtons( ! protocolURI->text().isEmpty() );
  }
}

// Slot for adding a new connection
void QgsOgrSourceSelect::btnNew_clicked()
{
  addNewConnection();
}
// Slot for deleting an existing connection
void QgsOgrSourceSelect::btnDelete_clicked()
{
  deleteConnection();
}


// Slot for editing a connection
void QgsOgrSourceSelect::btnEdit_clicked()
{
  editConnection();
}

void QgsOgrSourceSelect::cmbDatabaseTypes_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text )
  populateConnectionList();
  setSelectedConnectionType();
}

void QgsOgrSourceSelect::cmbConnections_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text )
  setSelectedConnection();
}

void QgsOgrSourceSelect::cmbProtocolTypes_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text )
  setProtocolWidgetsVisibility();
}
//********************end auto connected slots *****************/

void QgsOgrSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#loading-a-layer-from-a-file" ) );
}

///@endcond
