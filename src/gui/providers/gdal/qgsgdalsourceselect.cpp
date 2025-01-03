/***************************************************************************
                          qgsgdalsourceselect.h
                             -------------------
    begin                : August 05 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalsourceselect.h"
#include "moc_qgsgdalsourceselect.cpp"
///@cond PRIVATE

#include <QMessageBox>

#include "qgsproviderregistry.h"
#include "qgsgdalguiutils.h"
#include "qgsgdalutils.h"
#include "qgsgdalcredentialoptionswidget.h"
#include "qgsspinbox.h"
#include "qgsdoublespinbox.h"

#include <gdal.h>
#include <cpl_minixml.h>
#include "qgshelp.h"

QgsGdalSourceSelect::QgsGdalSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  mOpenOptionsGroupBox->setCollapsed( false );

  connect( radioSrcFile, &QRadioButton::toggled, this, &QgsGdalSourceSelect::radioSrcFile_toggled );
  connect( radioSrcOgcApi, &QRadioButton::toggled, this, &QgsGdalSourceSelect::radioSrcOgcApi_toggled );
  connect( radioSrcProtocol, &QRadioButton::toggled, this, &QgsGdalSourceSelect::radioSrcProtocol_toggled );
  connect( cmbProtocolTypes, &QComboBox::currentTextChanged, this, &QgsGdalSourceSelect::cmbProtocolTypes_currentIndexChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsGdalSourceSelect::showHelp );

  whileBlocking( radioSrcFile )->setChecked( true );
  protocolGroupBox->hide();

  QList<QgsGdalUtils::VsiNetworkFileSystemDetails> vsiDetails = QgsGdalUtils::vsiNetworkFileSystems();
  std::sort( vsiDetails.begin(), vsiDetails.end(), []( const QgsGdalUtils::VsiNetworkFileSystemDetails &a, const QgsGdalUtils::VsiNetworkFileSystemDetails &b ) {
    return QString::localeAwareCompare( a.name, b.name ) < 0;
  } );
  for ( const QgsGdalUtils::VsiNetworkFileSystemDetails &vsiDetail : std::as_const( vsiDetails ) )
  {
    cmbProtocolTypes->addItem( vsiDetail.name, vsiDetail.identifier );
  }

  connect( protocolURI, &QLineEdit::textChanged, this, [=]( const QString &text ) {
    if ( radioSrcProtocol->isChecked() )
    {
      emit enableButtons( !text.isEmpty() );
    }
  } );
  connect( mBucket, &QLineEdit::textChanged, this, [=]( const QString &text ) {
    if ( radioSrcProtocol->isChecked() )
    {
      emit enableButtons( !text.isEmpty() && !mKey->text().isEmpty() );
      fillOpenOptions();
    }
  } );
  connect( mKey, &QLineEdit::textChanged, this, [=]( const QString &text ) {
    if ( radioSrcProtocol->isChecked() )
    {
      emit enableButtons( !text.isEmpty() && !mBucket->text().isEmpty() );
      fillOpenOptions();
    }
  } );

  mFileWidget->setDialogTitle( tr( "Open GDAL Supported Raster Dataset(s)" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileRasterFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  mFileWidget->setOptions( QFileDialog::HideNameFilterDetails );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [=]( const QString &path ) {
    mRasterPath = mIsOgcApi ? QStringLiteral( "OGCAPI:%1" ).arg( path ) : path;
    emit enableButtons( !mRasterPath.isEmpty() );
    fillOpenOptions();
  } );
  mOpenOptionsGroupBox->setVisible( false );

  mCredentialsWidget = new QgsGdalCredentialOptionsWidget();
  mCredentialOptionsLayout->addWidget( mCredentialsWidget );
  mCredentialOptionsGroupBox->setVisible( false );

  connect( mCredentialsWidget, &QgsGdalCredentialOptionsWidget::optionsChanged, this, &QgsGdalSourceSelect::credentialOptionsChanged );

  mAuthSettingsProtocol->setDataprovider( QStringLiteral( "gdal" ) );
}

void QgsGdalSourceSelect::setProtocolWidgetsVisibility()
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

void QgsGdalSourceSelect::radioSrcFile_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->show();
    protocolGroupBox->hide();
    clearOpenOptions();
    updateProtocolOptions();

    emit enableButtons( !mFileWidget->filePath().isEmpty() );
  }
}

void QgsGdalSourceSelect::radioSrcOgcApi_toggled( bool checked )
{
  mIsOgcApi = checked;
  radioSrcFile_toggled( checked );
  if ( checked )
  {
    rasterDatasetLabel->setText( tr( "OGC API Endpoint" ) );
    const QString vectorPath = mFileWidget->filePath();
    emit enableButtons( !vectorPath.isEmpty() );
    if ( mRasterPath.isEmpty() )
    {
      mRasterPath = QStringLiteral( "OGCAPI:" );
    }
    fillOpenOptions();
  }
  else
  {
    rasterDatasetLabel->setText( tr( "Raster dataset(s)" ) );
  }
}

void QgsGdalSourceSelect::radioSrcProtocol_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->hide();
    protocolGroupBox->show();
    setProtocolWidgetsVisibility();
    clearOpenOptions();
    updateProtocolOptions();

    emit enableButtons( !protocolURI->text().isEmpty() );
  }
}

void QgsGdalSourceSelect::cmbProtocolTypes_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text )
  setProtocolWidgetsVisibility();
  clearOpenOptions();
  updateProtocolOptions();
}

void QgsGdalSourceSelect::addButtonClicked()
{
  computeDataSources();

  if ( mDataSources.isEmpty() )
  {
    QMessageBox::information( this, tr( "Add Raster Layer" ), tr( "No layers selected." ) );
    return;
  }

  // validate sources
  QStringList sources;
  enum class PromoteToVsiCurlStatus
  {
    NotAsked,
    AutoPromote,
    DontPromote
  };

  PromoteToVsiCurlStatus promoteToVsiCurlStatus = PromoteToVsiCurlStatus::NotAsked;

  for ( const QString &originalSource : std::as_const( mDataSources ) )
  {
    QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), originalSource );

    const QString vsiPrefix = parts.value( QStringLiteral( "vsiPrefix" ) ).toString();
    const QString scheme = QUrl( parts.value( QStringLiteral( "path" ) ).toString() ).scheme();
    const bool isRemoteNonVsiCurlUrl = vsiPrefix.isEmpty() && ( scheme.startsWith( QLatin1String( "http" ) ) || scheme == QLatin1String( "ftp" ) );
    if ( isRemoteNonVsiCurlUrl )
    {
      if ( promoteToVsiCurlStatus == PromoteToVsiCurlStatus::NotAsked )
      {
        if ( QMessageBox::warning( this, tr( "Add Raster Layer" ), tr( "Directly adding HTTP(S) or FTP sources can be very slow, as it requires a full download of the dataset.\n\n"
                                                                       "Would you like to use a streaming method to access this dataset instead (recommended)?" ),
                                   QMessageBox::Button::Yes | QMessageBox::Button::No, QMessageBox::Button::Yes )
             == QMessageBox::Yes )
        {
          promoteToVsiCurlStatus = PromoteToVsiCurlStatus::AutoPromote;
        }
        else
        {
          promoteToVsiCurlStatus = PromoteToVsiCurlStatus::DontPromote;
        }
      }

      if ( promoteToVsiCurlStatus == PromoteToVsiCurlStatus::AutoPromote )
      {
        parts.insert( QStringLiteral( "vsiPrefix" ), QStringLiteral( "/vsicurl/" ) );
      }
    }

    sources << QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts );
  }

  emit addRasterLayers( sources );
}

bool QgsGdalSourceSelect::configureFromUri( const QString &uri )
{
  mDataSources.clear();
  mDataSources.append( uri );
  const QVariantMap decodedUri = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri );
  const QString layerName { decodedUri.value( QStringLiteral( "layerName" ) ).toString() };
  mFileWidget->setFilePath( decodedUri.value( QStringLiteral( "path" ), QString() ).toString() );
  QVariantMap openOptions = decodedUri.value( QStringLiteral( "openOptions" ) ).toMap();
  // layerName becomes TABLE in some driver opening options (e.g. GPKG)
  if ( !layerName.isEmpty() )
  {
    openOptions.insert( QStringLiteral( "TABLE" ), layerName );
  }

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
        else if ( QLineEdit *le = qobject_cast<QLineEdit *>( *widget ) )
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

void QgsGdalSourceSelect::computeDataSources()
{
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

  if ( radioSrcFile->isChecked() || radioSrcOgcApi->isChecked() )
  {
    for ( const auto &filePath : QgsFileWidget::splitFilePaths( mRasterPath ) )
    {
      QVariantMap parts;
      if ( !openOptions.isEmpty() )
        parts.insert( QStringLiteral( "openOptions" ), openOptions );
      parts.insert( QStringLiteral( "path" ), filePath );
      mDataSources << QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts );
    }
  }
  else if ( radioSrcProtocol->isChecked() )
  {
    const bool cloudType = QgsGdalUtils::vsiHandlerType( cmbProtocolTypes->currentData().toString() ) == Qgis::VsiHandlerType::Cloud;
    if ( !cloudType && protocolURI->text().isEmpty() )
    {
      return;
    }
    else if ( cloudType && ( mBucket->text().isEmpty() || mKey->text().isEmpty() ) )
    {
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
    mDataSources << QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts );
  }
}

void QgsGdalSourceSelect::clearOpenOptions()
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

void QgsGdalSourceSelect::fillOpenOptions()
{
  clearOpenOptions();
  computeDataSources();
  if ( mDataSources.isEmpty() )
    return;

  const QString firstDataSource = mDataSources.at( 0 );
  const QString vsiPrefix = QgsGdalUtils::vsiPrefixForPath( firstDataSource );
  const QString scheme = QUrl( firstDataSource ).scheme();
  const bool isRemoteNonVsiCurlUrl = vsiPrefix.isEmpty() && ( scheme.startsWith( QLatin1String( "http" ) ) || scheme == QLatin1String( "ftp" ) );
  if ( isRemoteNonVsiCurlUrl )
  {
    // it can be very expensive to determine open options for non /vsicurl/ http uris -- it may require a full download of the remote dataset,
    // so just be safe and don't show any open options. Users can always manually append the /vsicurl/ prefix if they desire these, OR
    // correctly use the HTTP "Protocol" option instead.
    return;
  }

  QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), firstDataSource );
  const QVariantMap credentialOptions = parts.value( QStringLiteral( "credentialOptions" ) ).toMap();
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

  const QString gdalUri = QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts );
  GDALDriverH hDriver;
  hDriver = GDALIdentifyDriverEx( gdalUri.toUtf8().toStdString().c_str(), GDAL_OF_RASTER, nullptr, nullptr );
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

  for ( const QgsGdalOption &option : options )
  {
    // Exclude options that are not of raster scope
    if ( !option.scope.isEmpty()
         && option.scope.compare( QLatin1String( "raster" ), Qt::CaseInsensitive ) != 0 )
      continue;

    QWidget *control = QgsGdalGuiUtils::createWidgetForOption( option, nullptr, true );
    if ( !control )
      continue;

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION( 3, 8, 0 )
    if ( QString( GDALGetDriverShortName( hDriver ) ).compare( QLatin1String( "BAG" ) ) == 0
         && option.name == QLatin1String( "MODE" ) && option.options.contains( QLatin1String( "INTERPOLATED" ) ) )
    {
      gdal::dataset_unique_ptr hSrcDS( GDALOpen( gdalUri.toUtf8().constData(), GA_ReadOnly ) );
      if ( hSrcDS && QString { GDALGetMetadataItem( hSrcDS.get(), "HAS_SUPERGRIDS", nullptr ) } == QLatin1String( "TRUE" ) )
      {
        if ( QComboBox *combo = qobject_cast<QComboBox *>( control ) )
        {
          combo->setCurrentIndex( combo->findText( QLatin1String( "INTERPOLATED" ) ) );
        }
      }
    }
#endif

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

void QgsGdalSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#loading-a-layer-from-a-file" ) );
}

void QgsGdalSourceSelect::updateProtocolOptions()
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

void QgsGdalSourceSelect::credentialOptionsChanged()
{
  const QVariantMap newCredentialOptions = mCredentialsWidget->credentialOptions();
  if ( newCredentialOptions == mCredentialOptions )
    return;

  mCredentialOptions = newCredentialOptions;
  fillOpenOptions();
}

///@endcond
