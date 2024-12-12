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
#include "moc_qgsogrsourceselect.cpp"
///@cond PRIVATE

#include <QMessageBox>
#include <QInputDialog>
#include <QTextCodec>

#include "qgslogger.h"
#include "qgsvectordataprovider.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"
#include "ogr/qgsnewogrconnection.h"
#include "qgsgdalguiutils.h"
#include "qgsgui.h"
#include "qgsgdalutils.h"
#include "qgsspinbox.h"
#include "qgsdoublespinbox.h"
#include "qgsgdalcredentialoptionswidget.h"
#include "qgshelp.h"

#include <gdal.h>
#include <cpl_minixml.h>

QgsOgrSourceSelect::QgsOgrSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mOpenOptionsGroupBox->setCollapsed( false );

  connect( radioSrcFile, &QRadioButton::toggled, this, &QgsOgrSourceSelect::radioSrcFile_toggled );
  connect( radioSrcOgcApi, &QRadioButton::toggled, this, &QgsOgrSourceSelect::radioSrcOgcApi_toggled );
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

  if ( mWidgetMode != QgsProviderRegistry::WidgetMode::Standalone )
  {
    this->layout()->setSizeConstraint( QLayout::SetNoConstraint );
  }

  cmbDatabaseTypes->blockSignals( true );
  cmbConnections->blockSignals( true );
  radioSrcFile->setChecked( true );
  mDataSourceType = QStringLiteral( "file" );

  //set encoding
  cmbEncodings->addItems( QgsVectorDataProvider::availableEncodings() );

  cmbEncodings->insertItem( 0, tr( "Automatic" ), QString() );
  cmbEncodings->setCurrentIndex( 0 );

  //add database drivers
  QgsDebugMsgLevel( "Database drivers :" + QgsProviderRegistry::instance()->databaseDrivers(), 2 );
  QStringList dbDrivers = QgsProviderRegistry::instance()->databaseDrivers().split( ';' );

  for ( int i = 0; i < dbDrivers.count(); i++ )
  {
    QString dbDriver = dbDrivers.at( i );
    if ( !dbDriver.isEmpty() )
      cmbDatabaseTypes->addItem( dbDriver.split( ',' ).at( 0 ) );
  }

  //add directory drivers
  QStringList dirDrivers = QgsProviderRegistry::instance()->directoryDrivers().split( ';' );
  for ( int i = 0; i < dirDrivers.count(); i++ )
  {
    QString dirDriver = dirDrivers.at( i );
    if ( !dirDriver.isEmpty() )
      cmbDirectoryTypes->addItem( dirDriver.split( ',' ).at( 0 ) );
  }

  //add protocol drivers
  QList<QgsGdalUtils::VsiNetworkFileSystemDetails> vsiDetails = QgsGdalUtils::vsiNetworkFileSystems();
  std::sort( vsiDetails.begin(), vsiDetails.end(), []( const QgsGdalUtils::VsiNetworkFileSystemDetails &a, const QgsGdalUtils::VsiNetworkFileSystemDetails &b ) {
    return QString::localeAwareCompare( a.name, b.name ) < 0;
  } );
  for ( const QgsGdalUtils::VsiNetworkFileSystemDetails &vsiDetail : std::as_const( vsiDetails ) )
  {
    cmbProtocolTypes->addItem( vsiDetail.name, vsiDetail.identifier );
  }
  cmbProtocolTypes->addItem( QObject::tr( "WFS3 (Experimental)" ), QStringLiteral( "WFS3" ) );

  cmbDatabaseTypes->blockSignals( false );
  cmbConnections->blockSignals( false );

  mFileWidget->setDialogTitle( tr( "Open OGR Supported Vector Dataset(s)" ) );
  mVectorFileFilter = QgsProviderRegistry::instance()->fileVectorFilters();
  mFileWidget->setFilter( mVectorFileFilter );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  mFileWidget->setOptions( QFileDialog::HideNameFilterDetails );

  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [=]( const QString &path ) {
    mVectorPath = path;
    if ( radioSrcFile->isChecked() || radioSrcDirectory->isChecked() || radioSrcOgcApi->isChecked() )
      emit enableButtons( !mVectorPath.isEmpty() );
    fillOpenOptions();
  } );

  connect( protocolURI, &QLineEdit::textChanged, this, [=]( const QString &text ) {
    if ( radioSrcProtocol->isChecked() )
      emit enableButtons( !text.isEmpty() );
  } );
  connect( mBucket, &QLineEdit::textChanged, this, [=]( const QString &text ) {
    if ( radioSrcProtocol->isChecked() )
      emit enableButtons( !text.isEmpty() && !mKey->text().isEmpty() );
  } );
  connect( mKey, &QLineEdit::textChanged, this, [=]( const QString &text ) {
    if ( radioSrcProtocol->isChecked() )
      emit enableButtons( !text.isEmpty() && !mBucket->text().isEmpty() );
  } );

  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsOgrSourceSelect::fillOpenOptions );

  // Set filter for ogr compatible auth methods
  mAuthSettingsProtocol->setDataprovider( QStringLiteral( "ogr" ) );

  mOpenOptionsGroupBox->setVisible( false );

  mCredentialsWidget = new QgsGdalCredentialOptionsWidget();
  mCredentialOptionsLayout->addWidget( mCredentialsWidget );
  mCredentialOptionsGroupBox->setVisible( false );

  connect( mCredentialsWidget, &QgsGdalCredentialOptionsWidget::optionsChanged, this, &QgsOgrSourceSelect::credentialOptionsChanged );
}

QStringList QgsOgrSourceSelect::dataSources()
{
  return mDataSources;
}

QString QgsOgrSourceSelect::encoding()
{
  return cmbEncodings->currentData().isValid() ? cmbEncodings->currentData().toString() : cmbEncodings->currentText();
}

QString QgsOgrSourceSelect::dataSourceType()
{
  return mDataSourceType;
}

void QgsOgrSourceSelect::addNewConnection()
{
  QgsNewOgrConnection *nc = new QgsNewOgrConnection( this, cmbDatabaseTypes->currentText() );
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
    cmbConnections->removeItem( cmbConnections->currentIndex() ); // populateConnectionList();
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

  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );

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
  QgsDebugMsgLevel( "Setting selected type to" + cmbDatabaseTypes->currentText(), 3 );
}

void QgsOgrSourceSelect::setSelectedConnection()
{
  QgsSettings settings;
  settings.setValue( '/' + cmbDatabaseTypes->currentText() + "/connections/selected", cmbConnections->currentText() );
  QgsDebugMsgLevel( "Setting selected connection to " + cmbConnections->currentText(), 3 );
}

void QgsOgrSourceSelect::setProtocolWidgetsVisibility()
{
  if ( QgsGdalUtils::vsiHandlerType( cmbProtocolTypes->currentData().toString() ) == Qgis::VsiHandlerType::Cloud )
  {
    labelProtocolURI->hide();
    protocolURI->hide();
    mAuthGroupBox->hide();
    labelBucket->show();
    mBucket->show();
    labelKey->show();
    mKey->show();
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
  }
}

void QgsOgrSourceSelect::computeDataSources( bool interactive )
{
  QgsSettings settings;
  mDataSources.clear();

  QStringList openOptions;
  for ( QWidget *control : mOpenOptionsWidgets )
  {
    QString value;
    if ( QComboBox *cb = qobject_cast<QComboBox *>( control ) )
    {
      value = cb->itemData( cb->currentIndex() ).toString();
    }
    else if ( QLineEdit *le = qobject_cast<QLineEdit *>( control ) )
    {
      value = le->text();
    }
    else if ( QgsSpinBox *intSpin = qobject_cast<QgsSpinBox *>( control ) )
    {
      if ( intSpin->value() != intSpin->clearValue() )
      {
        value = QString::number( intSpin->value() );
      }
    }
    else if ( QgsDoubleSpinBox *doubleSpin = qobject_cast<QgsDoubleSpinBox *>( control ) )
    {
      if ( doubleSpin->value() != doubleSpin->clearValue() )
      {
        value = QString::number( doubleSpin->value() );
      }
    }
    if ( !value.isEmpty() )
    {
      openOptions << QStringLiteral( "%1=%2" ).arg( control->objectName(), value );
    }
  }

  const QVariantMap credentialOptions = !mCredentialOptionsGroupBox->isHidden() ? mCredentialOptions : QVariantMap();

  if ( radioSrcDatabase->isChecked() )
  {
    if ( !settings.contains( '/' + cmbDatabaseTypes->currentText() + "/connections/" + cmbConnections->currentText() + "/host" ) )
    {
      if ( interactive )
      {
        QMessageBox::information( this, tr( "Add vector layer" ), tr( "No database selected." ) );
      }
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
    if ( pass.isEmpty() && configid.isEmpty() )
    {
      if ( cmbDatabaseTypes->currentText() == QLatin1String( "MSSQL" ) )
        makeConnection = true;
      else if ( interactive )
        pass = QInputDialog::getText( this, tr( "Password for " ) + user, tr( "Please enter your password:" ), QLineEdit::Password, QString(), &makeConnection );
      else
        makeConnection = true;
    }

    if ( makeConnection || !( pass.isEmpty() && configid.isEmpty() ) )
    {
      QVariantMap parts;
      if ( !openOptions.isEmpty() )
        parts.insert( QStringLiteral( "openOptions" ), openOptions );
      parts.insert( QStringLiteral( "path" ), QgsGdalGuiUtils::createDatabaseURI( cmbDatabaseTypes->currentText(), host, database, port, configid, user, pass ) );
      mDataSources << QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts );
    }
  }
  else if ( radioSrcProtocol->isChecked() )
  {
    const bool cloudType = QgsGdalUtils::vsiHandlerType( cmbProtocolTypes->currentData().toString() ) == Qgis::VsiHandlerType::Cloud;
    if ( !cloudType && protocolURI->text().isEmpty() )
    {
      if ( interactive )
      {
        QMessageBox::information( this, tr( "Add vector layer" ), tr( "No protocol URI entered." ) );
      }
      return;
    }
    else if ( cloudType && ( mBucket->text().isEmpty() || mKey->text().isEmpty() ) )
    {
      if ( interactive )
      {
        QMessageBox::information( this, tr( "Add vector layer" ), tr( "No protocol bucket and/or key entered." ) );
      }
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

    QVariantMap parts;
    if ( !openOptions.isEmpty() )
      parts.insert( QStringLiteral( "openOptions" ), openOptions );
    if ( !credentialOptions.isEmpty() )
      parts.insert( QStringLiteral( "credentialOptions" ), credentialOptions );
    parts.insert( QStringLiteral( "path" ), QgsGdalGuiUtils::createProtocolURI( cmbProtocolTypes->currentData().toString(), uri, mAuthSettingsProtocol->configId(), mAuthSettingsProtocol->username(), mAuthSettingsProtocol->password() ) );
    mDataSources << QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts );
  }
  else if ( radioSrcFile->isChecked() || radioSrcOgcApi->isChecked() )
  {
    if ( mVectorPath.isEmpty() )
    {
      if ( mIsOgcApi )
      {
        mDataSources.push_back( QStringLiteral( "OGCAPI:" ) );
        return;
      }

      if ( interactive )
      {
        QMessageBox::information( this, tr( "Add vector layer" ), tr( "No layers selected." ) );
      }
      return;
    }

    for ( const auto &filePath : QgsFileWidget::splitFilePaths( mVectorPath ) )
    {
      QVariantMap parts;
      if ( !openOptions.isEmpty() )
        parts.insert( QStringLiteral( "openOptions" ), openOptions );
      parts.insert( QStringLiteral( "path" ), mIsOgcApi ? QStringLiteral( "OGCAPI:%1" ).arg( filePath ) : filePath );
      mDataSources << QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts );
    }
  }
  else if ( radioSrcDirectory->isChecked() )
  {
    if ( mVectorPath.isEmpty() )
    {
      if ( interactive )
      {
        QMessageBox::information( this, tr( "Add vector layer" ), tr( "No directory selected." ) );
      }
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

    QVariantMap parts;
    if ( !openOptions.isEmpty() )
      parts.insert( QStringLiteral( "openOptions" ), openOptions );
    parts.insert( QStringLiteral( "path" ), mVectorPath );
    mDataSources << QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts );
  }
}

void QgsOgrSourceSelect::addButtonClicked()
{
  computeDataSources( true );
  if ( !mDataSources.isEmpty() )
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
    clearOpenOptions();
    updateProtocolOptions();

    mFileWidget->setDialogTitle( tr( "Open an OGR Supported Vector Layer" ) );
    mFileWidget->setFilter( mVectorFileFilter );
    mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
    mFileWidget->setFilePath( QString() );

    mDataSourceType = QStringLiteral( "file" );

    emit enableButtons( !mFileWidget->filePath().isEmpty() );
  }
}

void QgsOgrSourceSelect::radioSrcOgcApi_toggled( bool checked )
{
  mIsOgcApi = checked;
  radioSrcFile_toggled( checked );
  if ( checked )
  {
    labelSrcDataset->setText( tr( "OGC API Endpoint" ) );
    mVectorPath = mFileWidget->filePath();
    emit enableButtons( !mVectorPath.isEmpty() );
    fillOpenOptions();
    updateProtocolOptions();
  }
  else
  {
    labelSrcDataset->setText( tr( "Vector Dataset(s)" ) );
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
    clearOpenOptions();
    updateProtocolOptions();

    mFileWidget->setDialogTitle( tr( "Open Directory" ) );
    mFileWidget->setStorageMode( QgsFileWidget::GetDirectory );
    mFileWidget->setFilePath( QString() );

    mDataSourceType = QStringLiteral( "directory" );

    emit enableButtons( !mFileWidget->filePath().isEmpty() );
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
    clearOpenOptions();
    updateProtocolOptions();

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
    clearOpenOptions();
    updateProtocolOptions();

    mDataSourceType = QStringLiteral( "protocol" );

    setProtocolWidgetsVisibility();

    emit enableButtons( !protocolURI->text().isEmpty() );
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
  clearOpenOptions();
  updateProtocolOptions();
}
//********************end auto connected slots *****************/

void QgsOgrSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#loading-a-layer-from-a-file" ) );
}

bool QgsOgrSourceSelect::configureFromUri( const QString &uri )
{
  mDataSources.clear();
  mDataSources.append( uri );
  const QVariantMap decodedUri = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), uri );
  mFileWidget->setFilePath( decodedUri.value( QStringLiteral( "path" ), QString() ).toString() );
  const QVariantMap openOptions = decodedUri.value( QStringLiteral( "openOptions" ) ).toMap();
  if ( !openOptions.isEmpty() )
  {
    for ( auto opt = openOptions.constBegin(); opt != openOptions.constEnd(); ++opt )
    {
      const auto widget { std::find_if( mOpenOptionsWidgets.cbegin(), mOpenOptionsWidgets.cend(), [=]( QWidget *widget ) {
        return widget->objectName() == opt.key();
      } ) };

      if ( widget != mOpenOptionsWidgets.cend() )
      {
        if ( auto cb = qobject_cast<QComboBox *>( *widget ) )
        {
          const auto idx { cb->findText( opt.value().toString() ) };
          if ( idx >= 0 )
          {
            cb->setCurrentIndex( idx );
          }
        }
        else if ( auto le = qobject_cast<QLineEdit *>( *widget ) )
        {
          le->setText( opt.value().toString() );
        }
        else if ( QgsSpinBox *intSpin = qobject_cast<QgsSpinBox *>( *widget ) )
        {
          if ( opt.value().toString().isEmpty() )
          {
            intSpin->clear();
          }
          else
          {
            intSpin->setValue( opt.value().toInt() );
          }
        }
        else if ( QgsDoubleSpinBox *doubleSpin = qobject_cast<QgsDoubleSpinBox *>( *widget ) )
        {
          if ( opt.value().toString().isEmpty() )
          {
            doubleSpin->clear();
          }
          else
          {
            doubleSpin->setValue( opt.value().toDouble() );
          }
        }
      }
    }
  }

  return true;
}

void QgsOgrSourceSelect::updateProtocolOptions()
{
  const QString currentProtocol = cmbProtocolTypes->currentData().toString();
  if ( radioSrcProtocol->isChecked() && QgsGdalUtils::vsiHandlerType( currentProtocol ) == Qgis::VsiHandlerType::Cloud )
  {
    mCredentialsWidget->setHandler( currentProtocol );
    mCredentialOptionsGroupBox->setVisible( true );
  }
  else
  {
    mCredentialOptionsGroupBox->setVisible( false );
  }
}

void QgsOgrSourceSelect::credentialOptionsChanged()
{
  const QVariantMap newCredentialOptions = mCredentialsWidget->credentialOptions();
  if ( newCredentialOptions == mCredentialOptions )
    return;

  mCredentialOptions = newCredentialOptions;
  fillOpenOptions();
}

void QgsOgrSourceSelect::clearOpenOptions()
{
  mOpenOptionsWidgets.clear();
  mOpenOptionsGroupBox->setVisible( false );
  mOpenOptionsLabel->clear();
  while ( mOpenOptionsLayout->count() )
  {
    QLayoutItem *item = mOpenOptionsLayout->takeAt( 0 );
    delete item->widget();
    delete item;
  }
}

void QgsOgrSourceSelect::fillOpenOptions()
{
  clearOpenOptions();
  computeDataSources( false );
  if ( mDataSources.isEmpty() )
    return;

  const QString firstDataSource = mDataSources.at( 0 );
  QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "ogr" ), firstDataSource );
  const QVariantMap credentialOptions = parts.value( QStringLiteral( "credentialOptions" ) ).toMap();
  const QString vsiPrefix = QgsGdalUtils::vsiPrefixForPath( firstDataSource );
  parts.remove( QStringLiteral( "credentialOptions" ) );
  if ( !credentialOptions.isEmpty() && !vsiPrefix.isEmpty() )
  {
    const thread_local QRegularExpression bucketRx( QStringLiteral( "^(.*)/" ) );
    const QRegularExpressionMatch bucketMatch = bucketRx.match( parts.value( QStringLiteral( "path" ) ).toString() );
    if ( bucketMatch.hasMatch() )
    {
      QgsGdalUtils::applyVsiCredentialOptions( vsiPrefix, bucketMatch.captured( 1 ), credentialOptions );
    }
  }

  const QString ogrUri = QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "ogr" ), parts );

  GDALDriverH hDriver;
  if ( STARTS_WITH_CI( ogrUri.toUtf8().toStdString().c_str(), "PG:" ) )
    hDriver = GDALGetDriverByName( "PostgreSQL" ); // otherwise the PostgisRaster driver gets identified
  else
    hDriver = GDALIdentifyDriverEx( ogrUri.toUtf8().toStdString().c_str(), GDAL_OF_VECTOR, nullptr, nullptr );
  if ( !hDriver )
    return;

  const char *pszOpenOptionList = GDALGetMetadataItem( hDriver, GDAL_DMD_OPENOPTIONLIST, nullptr );
  if ( !pszOpenOptionList )
    return;

  CPLXMLNode *psDoc = CPLParseXMLString( pszOpenOptionList );
  if ( !psDoc )
    return;
  CPLXMLNode *psOpenOptionList = CPLGetXMLNode( psDoc, "=OpenOptionList" );
  if ( !psOpenOptionList )
  {
    CPLDestroyXMLNode( psDoc );
    return;
  }

  const QList<QgsGdalOption> options = QgsGdalOption::optionsFromXml( psOpenOptionList );
  CPLDestroyXMLNode( psDoc );

  const bool bIsGPKG = EQUAL( GDALGetDriverShortName( hDriver ), "GPKG" );

  for ( const QgsGdalOption &option : options )
  {
    // Exclude options that are not of vector scope
    if ( !option.scope.isEmpty()
         && option.scope.compare( QLatin1String( "vector" ), Qt::CaseInsensitive ) != 0 )
      continue;

    // The GPKG driver list a lot of options that are only for rasters
    if ( bIsGPKG && !strstr( pszOpenOptionList, "scope=" ) && option.name != QLatin1String( "LIST_ALL_TABLES" ) && option.name != QLatin1String( "PRELUDE_STATEMENTS" ) )
      continue;

    // The NOLOCK option is automatically set by the OGR provider. Do not
    // expose it
    if ( bIsGPKG && option.name == QLatin1String( "NOLOCK" ) )
      continue;

    // Do not list database options already asked in the database dialog
    if ( radioSrcDatabase->isChecked() && ( option.name == QLatin1String( "USER" ) || option.name == QLatin1String( "PASSWORD" ) || option.name == QLatin1String( "HOST" ) || option.name == QLatin1String( "DBNAME" ) || option.name == QLatin1String( "DATABASE" ) || option.name == QLatin1String( "PORT" ) || option.name == QLatin1String( "SERVICE" ) ) )
    {
      continue;
    }

    // QGIS data model doesn't support the OGRFeature native data concept
    // (typically used for GeoJSON "foreign" members). Hide it to avoid setting
    // wrong expectations to users (https://github.com/qgis/QGIS/issues/48004)
    if ( option.name == QLatin1String( "NATIVE_DATA" ) )
      continue;

    QWidget *control = QgsGdalGuiUtils::createWidgetForOption( option, nullptr, true );
    if ( !control )
      continue;

    control->setObjectName( option.name );
    mOpenOptionsWidgets.push_back( control );

    QLabel *label = new QLabel( option.name );
    if ( !option.description.isEmpty() )
      label->setToolTip( QStringLiteral( "<p>%1</p>" ).arg( option.description ) );

    mOpenOptionsLayout->addRow( label, control );
  }

  // Set label to point to driver help page
  const QString helpTopic = QgsGdalUtils::gdalDocumentationUrlForDriver( hDriver );
  if ( !helpTopic.isEmpty() )
  {
    mOpenOptionsLabel->setText( tr( "Consult <a href=\"%1\">%2 driver help page</a> for detailed explanations on options" ).arg( helpTopic ).arg( GDALGetDriverShortName( hDriver ) ) );
    mOpenOptionsLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
    mOpenOptionsLabel->setOpenExternalLinks( true );
    mOpenOptionsLabel->setVisible( true );
  }
  else
  {
    mOpenOptionsLabel->setVisible( false );
  }

  mOpenOptionsGroupBox->setVisible( !mOpenOptionsWidgets.empty() );
}
///@endcond
