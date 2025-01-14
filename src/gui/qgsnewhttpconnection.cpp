/***************************************************************************
    qgsnewhttpconnection.cpp -  selector for a new HTTP server for WMS, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsnewhttpconnection.h"
#include "moc_qgsnewhttpconnection.cpp"
#include "qgsauthsettingswidget.h"
#include "qgssettings.h"
#include "qgshelp.h"
#include "qgsgui.h"
#include "fromencodedcomponenthelper.h"
#include "qgsowsconnection.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"

#include <QMessageBox>
#include <QUrl>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QUrlQuery>

const QgsSettingsEntryBool *QgsNewHttpConnection::settingsIgnoreReportedLayerExtentsDefault = new QgsSettingsEntryBool( QStringLiteral( "ignore-reported-layer-extents-default" ), sTreeHttpConnectionDialog, false );

QgsNewHttpConnection::QgsNewHttpConnection( QWidget *parent, ConnectionTypes types, const QString &serviceName, const QString &connectionName, QgsNewHttpConnection::Flags flags, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mTypes( types )
  , mServiceName( serviceName )
  , mOriginalConnName( connectionName )
{
  setupUi( this );

  // compatibility fix with former API (pre 3.26) when serviceName was a setting key instead
  if ( mServiceName.startsWith( QLatin1String( "qgis/" ) ) )
  {
    // It would be obviously much better to use mBaseKey also for credentials,
    // but for some strange reason a different hardcoded key was used instead.
    // WFS and WMS credentials were mixed with the same key WMS.
    // Only WMS and WFS providers are using QgsNewHttpConnection at this moment
    // using connection-wms and connection-wfs -> parse credential key from it.
    mServiceName = mServiceName.split( '-' ).last().toUpper();
  }

  if ( !( flags & FlagShowHttpSettings ) )
    mHttpHeaders->hide();

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsNewHttpConnection::showHelp );

  QString connectionType = mServiceName;
  if ( mServiceName == QLatin1String( "WMS" ) )
  {
    connectionType = QStringLiteral( "WMS/WMTS" );
  }
  setWindowTitle( tr( "Create a New %1 Connection" ).arg( connectionType ) );

  txtName->setValidator( new QRegularExpressionValidator( QRegularExpression( "[^\\/]+" ), txtName ) );

  cmbDpiMode->clear();
  cmbDpiMode->addItem( tr( "all" ), static_cast<int>( Qgis::DpiMode::All ) );
  cmbDpiMode->addItem( tr( "off" ), static_cast<int>( Qgis::DpiMode::Off ) );
  cmbDpiMode->addItem( tr( "QGIS" ), static_cast<int>( Qgis::DpiMode::QGIS ) );
  cmbDpiMode->addItem( tr( "UMN" ), static_cast<int>( Qgis::DpiMode::UMN ) );
  cmbDpiMode->addItem( tr( "GeoServer" ), static_cast<int>( Qgis::DpiMode::GeoServer ) );

  cmbTilePixelRatio->clear();
  cmbTilePixelRatio->addItem( tr( "Undefined (not scaled)" ), static_cast<int>( Qgis::TilePixelRatio::Undefined ) );
  cmbTilePixelRatio->addItem( tr( "Standard (96 DPI)" ), static_cast<int>( Qgis::TilePixelRatio::StandardDpi ) );
  cmbTilePixelRatio->addItem( tr( "High (192 DPI)" ), static_cast<int>( Qgis::TilePixelRatio::HighDpi ) );

  cmbVersion->clear();
  cmbVersion->addItem( tr( "Maximum" ) );
  cmbVersion->addItem( tr( "1.0" ) );
  cmbVersion->addItem( tr( "1.1" ) );
  cmbVersion->addItem( tr( "2.0" ) );
  cmbVersion->addItem( tr( "OGC API - Features" ) );
  connect( cmbVersion, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewHttpConnection::wfsVersionCurrentIndexChanged );

  cmbFeaturePaging->clear();
  cmbFeaturePaging->addItem( tr( "Default (trust server capabilities)" ) );
  cmbFeaturePaging->addItem( tr( "Enabled" ) );
  cmbFeaturePaging->addItem( tr( "Disabled" ) );
  connect( cmbFeaturePaging, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewHttpConnection::wfsFeaturePagingCurrentIndexChanged );

  if ( !connectionName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters

    txtName->setText( connectionName );
    const QStringList detailParameters { mServiceName.toLower(), connectionName };
    txtUrl->setText( QgsOwsConnection::settingsUrl->value( detailParameters ) );
    mHttpHeaders->setHeaders( QgsHttpHeaders( QgsOwsConnection::settingsHeaders->value( detailParameters ) ) );

    updateServiceSpecificSettings();

    // Authentication
    mAuthSettings->setUsername( QgsOwsConnection::settingsUsername->value( detailParameters ) );
    mAuthSettings->setPassword( QgsOwsConnection::settingsPassword->value( detailParameters ) );
    mAuthSettings->setConfigId( QgsOwsConnection::settingsAuthCfg->value( detailParameters ) );
  }
  mWfsVersionDetectButton->setDisabled( txtUrl->text().isEmpty() );

  if ( !( mTypes & ConnectionWms ) && !( mTypes & ConnectionWcs ) )
  {
    mWmsOptionsGroupBox->setVisible( false );
    mGroupBox->layout()->removeWidget( mWmsOptionsGroupBox );
  }
  if ( !( mTypes & ConnectionWfs ) )
  {
    mWfsOptionsGroupBox->setVisible( false );
    mGroupBox->layout()->removeWidget( mWfsOptionsGroupBox );
  }
  else
  {
    txtUrl->setToolTip( tr( "HTTP address of the WFS service, or landing page of a OGC API service<br>(an ending slash might be needed for some OGC API servers)" ) );
  }

  if ( mTypes & ConnectionWcs )
  {
    cbxIgnoreGetMapURI->setText( tr( "Ignore GetCoverage URI reported in capabilities" ) );
    cbxWmsIgnoreAxisOrientation->setText( tr( "Ignore axis orientation" ) );
    if ( !( mTypes & ConnectionWms ) )
    {
      mWmsOptionsGroupBox->setTitle( tr( "WCS Options" ) );

      cbxIgnoreGetFeatureInfoURI->setVisible( false );
      mGroupBox->layout()->removeWidget( cbxIgnoreGetFeatureInfoURI );

      sbFeatureCount->setVisible( false );
      mGroupBox->layout()->removeWidget( sbFeatureCount );
      lblFeatureCount->setVisible( false );
      mGroupBox->layout()->removeWidget( lblFeatureCount );

      cmbDpiMode->setVisible( false );
      mGroupBox->layout()->removeWidget( cmbDpiMode );
      lblDpiMode->setVisible( false );
      mGroupBox->layout()->removeWidget( lblDpiMode );
      cmbTilePixelRatio->setVisible( false );
      mGroupBox->layout()->removeWidget( cmbTilePixelRatio );
      lblTilePixelRatio->setVisible( false );
      mGroupBox->layout()->removeWidget( lblTilePixelRatio );
    }
  }

  cbxWmsIgnoreReportedLayerExtents->setChecked( settingsIgnoreReportedLayerExtentsDefault->value() );

  if ( !( flags & FlagShowTestConnection ) )
  {
    mTestConnectionButton->hide();
    mGroupBox->layout()->removeWidget( mTestConnectionButton );
  }

  if ( flags & FlagHideAuthenticationGroup )
  {
    mAuthGroupBox->hide();
    mGroupBox->layout()->removeWidget( mAuthGroupBox );
  }
  // Adjust height
  const int w = width();
  adjustSize();
  resize( w, height() );

  connect( txtName, &QLineEdit::textChanged, this, &QgsNewHttpConnection::nameChanged );
  connect( txtUrl, &QLineEdit::textChanged, this, &QgsNewHttpConnection::urlChanged );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( txtName, &QLineEdit::textChanged, this, &QgsNewHttpConnection::updateOkButtonState );
  connect( txtUrl, &QLineEdit::textChanged, this, &QgsNewHttpConnection::updateOkButtonState );

  nameChanged( connectionName );
}

void QgsNewHttpConnection::wfsVersionCurrentIndexChanged( int index )
{
  // For now 2019-06-06, leave paging checkable for some WFS version 1.1 servers with support
  const bool pagingOptionsEnabled = ( index == WFS_VERSION_MAX || index >= WFS_VERSION_1_1 );
  cmbFeaturePaging->setEnabled( pagingOptionsEnabled );
  lblPageSize->setEnabled( pagingOptionsEnabled );
  txtPageSize->setEnabled( pagingOptionsEnabled );
  cbxWfsIgnoreAxisOrientation->setEnabled( index != WFS_VERSION_1_0 && index != WFS_VERSION_API_FEATURES_1_0 );
  cbxWfsInvertAxisOrientation->setEnabled( index != WFS_VERSION_API_FEATURES_1_0 );
  wfsUseGml2EncodingForTransactions()->setEnabled( index == WFS_VERSION_1_1 );
}

void QgsNewHttpConnection::wfsFeaturePagingCurrentIndexChanged( int index )
{
  const bool pagingNotDisabled = index != static_cast<int>( QgsNewHttpConnection::WfsFeaturePagingIndex::DISABLED );
  lblPageSize->setEnabled( pagingNotDisabled );
  txtPageSize->setEnabled( pagingNotDisabled );
}

QString QgsNewHttpConnection::name() const
{
  return txtName->text();
}

QString QgsNewHttpConnection::url() const
{
  return txtUrl->text();
}

void QgsNewHttpConnection::nameChanged( const QString &text )
{
  Q_UNUSED( text )
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( txtName->text().isEmpty() || txtUrl->text().isEmpty() );
}

void QgsNewHttpConnection::urlChanged( const QString &text )
{
  Q_UNUSED( text )
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( txtName->text().isEmpty() || txtUrl->text().isEmpty() );
  mWfsVersionDetectButton->setDisabled( txtUrl->text().isEmpty() );
}

void QgsNewHttpConnection::updateOkButtonState()
{
  const bool enabled = !txtName->text().isEmpty() && !txtUrl->text().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

bool QgsNewHttpConnection::validate()
{
  const QString newConnectionName = txtName->text();

  bool urlExists = QgsOwsConnection::settingsUrl->exists( { mServiceName.toLower(), newConnectionName } );

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( newConnectionName, Qt::CaseInsensitive ) != 0 ) && urlExists && QMessageBox::question( this, tr( "Save Connection" ), tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return false;
  }

  if ( !mAuthSettings->password().isEmpty() && QMessageBox::question( this, tr( "Saving Passwords" ), tr( "WARNING: You have entered a password. It will be stored in unsecured plain text in your project files and your home directory (Unix-like OS) or user profile (Windows). If you want to avoid this, press Cancel and either:\n\na) Don't provide a password in the connection settings — it will be requested interactively when needed;\nb) Use the Configuration tab to add your credentials in an HTTP Basic Authentication method and store them in an encrypted database." ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return false;
  }

  return true;
}

QPushButton *QgsNewHttpConnection::testConnectButton()
{
  return mTestConnectionButton;
}

QgsAuthSettingsWidget *QgsNewHttpConnection::authSettingsWidget()
{
  return mAuthSettings;
}

QPushButton *QgsNewHttpConnection::wfsVersionDetectButton()
{
  return mWfsVersionDetectButton;
}

QComboBox *QgsNewHttpConnection::wfsVersionComboBox()
{
  return cmbVersion;
}

QComboBox *QgsNewHttpConnection::wfsPagingComboBox()
{
  return cmbFeaturePaging;
}

QCheckBox *QgsNewHttpConnection::wfsUseGml2EncodingForTransactions()
{
  return cbxWfsUseGml2EncodingForTransactions;
}

QLineEdit *QgsNewHttpConnection::wfsPageSizeLineEdit()
{
  return txtPageSize;
}

QString QgsNewHttpConnection::wfsSettingsKey( const QString &base, const QString &connectionName ) const
{
  return base + connectionName;
}

QString QgsNewHttpConnection::wmsSettingsKey( const QString &base, const QString &connectionName ) const
{
  return base + connectionName;
}

void QgsNewHttpConnection::updateServiceSpecificSettings()
{
  QStringList detailsParameters = { mServiceName.toLower(), mOriginalConnName };

  cbxIgnoreGetMapURI->setChecked( QgsOwsConnection::settingsIgnoreGetMapURI->value( detailsParameters ) );
  cbxWmsIgnoreReportedLayerExtents->setChecked( QgsOwsConnection::settingsReportedLayerExtents->value( detailsParameters ) );
  cbxWfsIgnoreAxisOrientation->setChecked( QgsOwsConnection::settingsIgnoreAxisOrientation->value( detailsParameters ) );
  cbxWfsInvertAxisOrientation->setChecked( QgsOwsConnection::settingsInvertAxisOrientation->value( detailsParameters ) );
  cbxWfsUseGml2EncodingForTransactions->setChecked( QgsOwsConnection::settingsPreferCoordinatesForWfsT11->value( detailsParameters ) );

  cbxWmsIgnoreAxisOrientation->setChecked( QgsOwsConnection::settingsIgnoreAxisOrientation->value( detailsParameters ) );
  cbxWmsInvertAxisOrientation->setChecked( QgsOwsConnection::settingsInvertAxisOrientation->value( detailsParameters ) );
  cbxIgnoreGetFeatureInfoURI->setChecked( QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->value( detailsParameters ) );
  cbxSmoothPixmapTransform->setChecked( QgsOwsConnection::settingsSmoothPixmapTransform->value( detailsParameters ) );

  Qgis::DpiMode dpiMode = QgsOwsConnection::settingsDpiMode->value( detailsParameters );
  cmbDpiMode->setCurrentIndex( cmbDpiMode->findData( static_cast<int>( dpiMode ) ) );
  Qgis::TilePixelRatio tilePixelRatio = QgsOwsConnection::settingsTilePixelRatio->value( detailsParameters );
  cmbTilePixelRatio->setCurrentIndex( cmbTilePixelRatio->findData( static_cast<int>( tilePixelRatio ) ) );

  sbFeatureCount->setValue( QgsOwsConnection::settingsFeatureCount->value( detailsParameters ) );

  const QString version = QgsOwsConnection::settingsVersion->value( detailsParameters );
  int versionIdx = WFS_VERSION_MAX; // AUTO
  if ( version == QLatin1String( "1.0.0" ) )
    versionIdx = WFS_VERSION_1_0;
  else if ( version == QLatin1String( "1.1.0" ) )
    versionIdx = WFS_VERSION_1_1;
  else if ( version == QLatin1String( "2.0.0" ) )
    versionIdx = WFS_VERSION_2_0;
  else if ( version == QLatin1String( "OGC_API_FEATURES" ) )
    versionIdx = WFS_VERSION_API_FEATURES_1_0;
  cmbVersion->setCurrentIndex( versionIdx );

  // Enable/disable these items per WFS versions
  wfsVersionCurrentIndexChanged( versionIdx );

  mHttpHeaders->setHeaders( QgsHttpHeaders( QgsOwsConnection::settingsHeaders->value( { mServiceName.toLower(), mOriginalConnName } ) ) );

  txtMaxNumFeatures->setText( QgsOwsConnection::settingsMaxNumFeatures->value( detailsParameters ) );

  // Only default to paging enabled if WFS 2.0.0 or higher
  const QString pagingEnabled = QgsOwsConnection::settingsPagingEnabled->value( detailsParameters );
  if ( pagingEnabled == QLatin1String( "enabled" ) )
    cmbFeaturePaging->setCurrentIndex( static_cast<int>( QgsNewHttpConnection::WfsFeaturePagingIndex::ENABLED ) );
  else if ( pagingEnabled == QLatin1String( "disabled" ) )
    cmbFeaturePaging->setCurrentIndex( static_cast<int>( QgsNewHttpConnection::WfsFeaturePagingIndex::DISABLED ) );
  else
    cmbFeaturePaging->setCurrentIndex( static_cast<int>( QgsNewHttpConnection::WfsFeaturePagingIndex::DEFAULT ) );

  txtPageSize->setText( QgsOwsConnection::settingsPagesize->value( detailsParameters ) );
}

QUrl QgsNewHttpConnection::urlTrimmed() const
{
  QUrl url( txtUrl->text().trimmed() );
  QUrlQuery query( url );
  const QList<QPair<QString, QString>> items = query.queryItems( QUrl::FullyEncoded );
  QHash<QString, QPair<QString, QString>> params;
  for ( const QPair<QString, QString> &it : items )
  {
    params.insert( it.first.toUpper(), it );
  }

  if ( params[QStringLiteral( "SERVICE" )].second.toUpper() == "WMS" || params[QStringLiteral( "SERVICE" )].second.toUpper() == "WFS" || params[QStringLiteral( "SERVICE" )].second.toUpper() == "WCS" )
  {
    query.removeQueryItem( params.value( QStringLiteral( "SERVICE" ) ).first );
    query.removeQueryItem( params.value( QStringLiteral( "REQUEST" ) ).first );
    query.removeQueryItem( params.value( QStringLiteral( "FORMAT" ) ).first );
  }

  url.setQuery( query );

  if ( url.path( QUrl::FullyEncoded ).isEmpty() )
  {
    url.setPath( fromEncodedComponent_helper( "/" ) );
  }
  return url;
}

void QgsNewHttpConnection::accept()
{
  const QString newConnectionName = txtName->text();

  if ( !validate() )
    return;

  QgsSettings settings;

  // on rename delete original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != newConnectionName )
  {
    QgsOwsConnection::sTreeOwsConnections->deleteItem( mOriginalConnName, { mServiceName.toLower() } );
    settings.sync();
  }

  QStringList detailsParameters = { mServiceName.toLower(), newConnectionName };

  const QUrl url( urlTrimmed() );
  QgsOwsConnection::settingsUrl->setValue( url.toString(), detailsParameters );

  if ( mTypes & ConnectionWfs )
  {
    QgsOwsConnection::settingsIgnoreAxisOrientation->setValue( cbxWfsIgnoreAxisOrientation->isChecked(), detailsParameters );
    QgsOwsConnection::settingsInvertAxisOrientation->setValue( cbxWfsInvertAxisOrientation->isChecked(), detailsParameters );
    QgsOwsConnection::settingsPreferCoordinatesForWfsT11->setValue( cbxWfsUseGml2EncodingForTransactions->isChecked(), detailsParameters );
  }
  if ( mTypes & ConnectionWms || mTypes & ConnectionWcs )
  {
    QgsOwsConnection::settingsIgnoreAxisOrientation->setValue( cbxWmsIgnoreAxisOrientation->isChecked(), detailsParameters );
    QgsOwsConnection::settingsInvertAxisOrientation->setValue( cbxWmsInvertAxisOrientation->isChecked(), detailsParameters );

    QgsOwsConnection::settingsReportedLayerExtents->setValue( cbxWmsIgnoreReportedLayerExtents->isChecked(), detailsParameters );
    QgsOwsConnection::settingsIgnoreGetMapURI->setValue( cbxIgnoreGetMapURI->isChecked(), detailsParameters );
    QgsOwsConnection::settingsSmoothPixmapTransform->setValue( cbxSmoothPixmapTransform->isChecked(), detailsParameters );

    Qgis::DpiMode dpiMode = cmbDpiMode->currentData().value<Qgis::DpiMode>();
    QgsOwsConnection::settingsDpiMode->setValue( dpiMode, detailsParameters );
    Qgis::TilePixelRatio tilePixelRatio = cmbTilePixelRatio->currentData().value<Qgis::TilePixelRatio>();
    QgsOwsConnection::settingsTilePixelRatio->setValue( tilePixelRatio, detailsParameters );

    QgsOwsConnection::settingsHeaders->setValue( mHttpHeaders->httpHeaders().headers(), detailsParameters );
  }
  if ( mTypes & ConnectionWms )
  {
    QgsOwsConnection::settingsIgnoreGetFeatureInfoURI->setValue( cbxIgnoreGetFeatureInfoURI->isChecked(), detailsParameters );
    QgsOwsConnection::settingsFeatureCount->setValue( sbFeatureCount->value(), detailsParameters );
  }
  if ( mTypes & ConnectionWfs )
  {
    QString version = QStringLiteral( "auto" );
    switch ( cmbVersion->currentIndex() )
    {
      case WFS_VERSION_MAX:
        version = QStringLiteral( "auto" );
        break;
      case WFS_VERSION_1_0:
        version = QStringLiteral( "1.0.0" );
        break;
      case WFS_VERSION_1_1:
        version = QStringLiteral( "1.1.0" );
        break;
      case WFS_VERSION_2_0:
        version = QStringLiteral( "2.0.0" );
        break;
      case WFS_VERSION_API_FEATURES_1_0:
        version = QStringLiteral( "OGC_API_FEATURES" );
        break;
    }
    QgsOwsConnection::settingsVersion->setValue( version, detailsParameters );
    QgsOwsConnection::settingsMaxNumFeatures->setValue( txtMaxNumFeatures->text(), detailsParameters );
    QgsOwsConnection::settingsPagesize->setValue( txtPageSize->text(), detailsParameters );

    QString pagingEnabled = QStringLiteral( "default" );
    switch ( cmbFeaturePaging->currentIndex() )
    {
      case static_cast<int>( QgsNewHttpConnection::WfsFeaturePagingIndex::DEFAULT ):
        pagingEnabled = QStringLiteral( "default" );
        break;
      case static_cast<int>( QgsNewHttpConnection::WfsFeaturePagingIndex::ENABLED ):
        pagingEnabled = QStringLiteral( "enabled" );
        break;
      case static_cast<int>( QgsNewHttpConnection::WfsFeaturePagingIndex::DISABLED ):
        pagingEnabled = QStringLiteral( "disabled" );
        break;
    }
    QgsOwsConnection::settingsPagingEnabled->setValue( pagingEnabled, detailsParameters );
  }

  QStringList credentialsParameters = { mServiceName.toLower(), newConnectionName };
  QgsOwsConnection::settingsUsername->setValue( mAuthSettings->username(), credentialsParameters );
  QgsOwsConnection::settingsPassword->setValue( mAuthSettings->password(), credentialsParameters );
  QgsOwsConnection::settingsAuthCfg->setValue( mAuthSettings->configId(), credentialsParameters );

  if ( mHttpHeaders->isVisible() )
    QgsOwsConnection::settingsHeaders->setValue( mHttpHeaders->httpHeaders().headers(), credentialsParameters );

  QgsOwsConnection::sTreeOwsConnections->setSelectedItem( newConnectionName, { mServiceName.toLower() } );

  QDialog::accept();
}

void QgsNewHttpConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_ogc/index.html" ) );
}
