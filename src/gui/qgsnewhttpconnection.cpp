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
#include "qgsauthsettingswidget.h"
#include "qgssettings.h"
#include "qgshelp.h"
#include "qgsgui.h"
#include "fromencodedcomponenthelper.h"
#include "qgsowsconnection.h"

#include <QMessageBox>
#include <QUrl>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QUrlQuery>

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

  cmbVersion->clear();
  cmbVersion->addItem( tr( "Maximum" ) );
  cmbVersion->addItem( tr( "1.0" ) );
  cmbVersion->addItem( tr( "1.1" ) );
  cmbVersion->addItem( tr( "2.0" ) );
  cmbVersion->addItem( tr( "OGC API - Features" ) );
  connect( cmbVersion,
           static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsNewHttpConnection::wfsVersionCurrentIndexChanged );

  connect( cbxWfsFeaturePaging, &QCheckBox::stateChanged,
           this, &QgsNewHttpConnection::wfsFeaturePagingStateChanged );

  if ( !connectionName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters

    const QgsSettings settings;

    txtName->setText( connectionName );
    txtUrl->setText( QgsOwsConnection::settingsConnectionUrl.value( {mServiceName.toLower(), connectionName} ) );
    mHttpHeaders->setFromSettings( settings, QStringLiteral( "qgis/connections-%1/%2" ).arg( mServiceName.toLower(), connectionName ) );

    updateServiceSpecificSettings();

    // Authentication
    mAuthSettings->setUsername( QgsOwsConnection::settingsConnectionUsername.value( {mServiceName, connectionName} ) );
    mAuthSettings->setPassword( QgsOwsConnection::settingsConnectionPassword.value( {mServiceName, connectionName} ) );
    mAuthSettings->setConfigId( QgsOwsConnection::settingsConnectionAuthCfg.value( {mServiceName, connectionName} ) );
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

      cmbDpiMode->setVisible( false );
      mGroupBox->layout()->removeWidget( cmbDpiMode );
      lblDpiMode->setVisible( false );
      mGroupBox->layout()->removeWidget( lblDpiMode );
    }
  }

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
  cbxWfsFeaturePaging->setEnabled( index == WFS_VERSION_MAX || index >= WFS_VERSION_2_0 );
  lblPageSize->setEnabled( cbxWfsFeaturePaging->isChecked() && ( index == WFS_VERSION_MAX || index >= WFS_VERSION_1_1 ) );
  txtPageSize->setEnabled( cbxWfsFeaturePaging->isChecked() && ( index == WFS_VERSION_MAX || index >= WFS_VERSION_1_1 ) );
  cbxWfsIgnoreAxisOrientation->setEnabled( index != WFS_VERSION_1_0 && index != WFS_VERSION_API_FEATURES_1_0 );
  cbxWfsInvertAxisOrientation->setEnabled( index != WFS_VERSION_API_FEATURES_1_0 );
  wfsUseGml2EncodingForTransactions()->setEnabled( index == WFS_VERSION_1_1 );
}

void QgsNewHttpConnection::wfsFeaturePagingStateChanged( int state )
{
  lblPageSize->setEnabled( state == Qt::Checked );
  txtPageSize->setEnabled( state == Qt::Checked );
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

  bool urlExists = QgsOwsConnection::settingsConnectionUrl.exists( {mServiceName.toLower(), newConnectionName} );

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( newConnectionName, Qt::CaseInsensitive ) != 0 ) &&
       urlExists &&
       QMessageBox::question( this,
                              tr( "Save Connection" ),
                              tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return false;
  }

  if ( ! mAuthSettings->password().isEmpty() &&
       QMessageBox::question( this,
                              tr( "Saving Passwords" ),
                              tr( "WARNING: You have entered a password. It will be stored in unsecured plain text in your project files and your home directory (Unix-like OS) or user profile (Windows). If you want to avoid this, press Cancel and either:\n\na) Don't provide a password in the connection settings — it will be requested interactively when needed;\nb) Use the Configuration tab to add your credentials in an HTTP Basic Authentication method and store them in an encrypted database." ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
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

QCheckBox *QgsNewHttpConnection::wfsPagingEnabledCheckBox()
{
  return cbxWfsFeaturePaging;
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
  QStringList detailsParameters = {mServiceName.toLower(), mOriginalConnName};

  cbxIgnoreGetMapURI->setChecked( QgsOwsConnection::settingsConnectionIgnoreGetMapURI.value( detailsParameters ) );
  cbxWmsIgnoreReportedLayerExtents->setChecked( QgsOwsConnection::settingsConnectionReportedLayerExtents.value( detailsParameters ) );
  cbxWfsIgnoreAxisOrientation->setChecked( QgsOwsConnection::settingsConnectionIgnoreAxisOrientation.value( detailsParameters ) );
  cbxWfsInvertAxisOrientation->setChecked( QgsOwsConnection::settingsConnectionInvertAxisOrientation.value( detailsParameters ) );
  cbxWfsUseGml2EncodingForTransactions->setChecked( QgsOwsConnection::settingsConnectionPreferCoordinatesForWfsT11.value( detailsParameters ) );

  cbxWmsIgnoreAxisOrientation->setChecked( QgsOwsConnection::settingsConnectionIgnoreAxisOrientation.value( detailsParameters ) );
  cbxWmsInvertAxisOrientation->setChecked( QgsOwsConnection::settingsConnectionInvertAxisOrientation.value( detailsParameters ) );
  cbxIgnoreGetFeatureInfoURI->setChecked( QgsOwsConnection::settingsConnectionIgnoreGetFeatureInfoURI.value( detailsParameters ) );
  cbxSmoothPixmapTransform->setChecked( QgsOwsConnection::settingsConnectionSmoothPixmapTransform.value( detailsParameters ) );

  Qgis::DpiMode dpiMode = QgsOwsConnection::settingsConnectionDpiMode.value( detailsParameters );
  cmbDpiMode->setCurrentIndex( cmbDpiMode->findData( static_cast<int>( dpiMode ) ) );

  const QString version = QgsOwsConnection::settingsConnectionVersion.value( detailsParameters );
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

  mHttpHeaders->setFromSettings( QgsSettings(), QStringLiteral( "qgis/connections-%1/%2" ).arg( mServiceName.toLower(), mOriginalConnName ) );

  txtMaxNumFeatures->setText( QgsOwsConnection::settingsConnectionMaxNumFeatures.value( detailsParameters ) );

  // Only default to paging enabled if WFS 2.0.0 or higher
  const bool pagingEnabled = QgsOwsConnection::settingsConnectionPagingEnabled.valueWithDefaultOverride( versionIdx == WFS_VERSION_MAX || versionIdx >= WFS_VERSION_2_0, detailsParameters );
  txtPageSize->setText( QgsOwsConnection::settingsConnectionPagesize.value( detailsParameters ) );
  cbxWfsFeaturePaging->setChecked( pagingEnabled );
}

QUrl QgsNewHttpConnection::urlTrimmed() const
{
  QUrl url( txtUrl->text().trimmed() );
  QUrlQuery query( url );
  const QList<QPair<QString, QString> > items = query.queryItems( QUrl::FullyEncoded );
  QHash< QString, QPair<QString, QString> > params;
  for ( const QPair<QString, QString> &it : items )
  {
    params.insert( it.first.toUpper(), it );
  }

  if ( params[QStringLiteral( "SERVICE" )].second.toUpper() == "WMS" ||
       params[QStringLiteral( "SERVICE" )].second.toUpper() == "WFS" ||
       params[QStringLiteral( "SERVICE" )].second.toUpper() == "WCS" )
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
    QgsOwsConnection::settingsServiceConnectionDetailsGroup.removeAllChildrenSettings( {mServiceName.toLower(), mOriginalConnName} );
    QgsOwsConnection::settingsServiceConnectionCredentialsGroup.removeAllChildrenSettings( {mServiceName, mOriginalConnName} );
    settings.sync();
  }

  QStringList detailsParameters = {mServiceName.toLower(), newConnectionName};

  const QUrl url( urlTrimmed() );
  QgsOwsConnection::settingsConnectionUrl.setValue( url.toString(), detailsParameters );

  if ( mTypes & ConnectionWfs )
  {
    QgsOwsConnection::settingsConnectionIgnoreAxisOrientation.setValue( cbxWfsIgnoreAxisOrientation->isChecked(), detailsParameters );
    QgsOwsConnection::settingsConnectionInvertAxisOrientation.setValue( cbxWfsInvertAxisOrientation->isChecked(), detailsParameters );
    QgsOwsConnection::settingsConnectionPreferCoordinatesForWfsT11.setValue( cbxWfsUseGml2EncodingForTransactions->isChecked(), detailsParameters );
  }
  if ( mTypes & ConnectionWms || mTypes & ConnectionWcs )
  {
    QgsOwsConnection::settingsConnectionIgnoreAxisOrientation.setValue( cbxWmsIgnoreAxisOrientation->isChecked(), detailsParameters );
    QgsOwsConnection::settingsConnectionInvertAxisOrientation.setValue( cbxWmsInvertAxisOrientation->isChecked(), detailsParameters );

    QgsOwsConnection::settingsConnectionReportedLayerExtents.setValue( cbxWmsIgnoreReportedLayerExtents->isChecked(), detailsParameters );
    QgsOwsConnection::settingsConnectionIgnoreGetMapURI.setValue( cbxIgnoreGetMapURI->isChecked(), detailsParameters );
    QgsOwsConnection::settingsConnectionSmoothPixmapTransform.setValue( cbxSmoothPixmapTransform->isChecked(), detailsParameters );

    Qgis::DpiMode dpiMode = cmbDpiMode->currentData().value<Qgis::DpiMode>();
    QgsOwsConnection::settingsConnectionDpiMode.setValue( dpiMode, detailsParameters );

    mHttpHeaders->updateSettings( settings, QStringLiteral( "qgis/connections-%1/%2" ).arg( mServiceName.toLower(), newConnectionName ) );
  }
  if ( mTypes & ConnectionWms )
  {
    QgsOwsConnection::settingsConnectionIgnoreGetFeatureInfoURI.setValue( cbxIgnoreGetFeatureInfoURI->isChecked(), detailsParameters );
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
    QgsOwsConnection::settingsConnectionVersion.setValue( version, detailsParameters );
    QgsOwsConnection::settingsConnectionMaxNumFeatures.setValue( txtMaxNumFeatures->text(), detailsParameters );
    QgsOwsConnection::settingsConnectionPagesize.setValue( txtPageSize->text(), detailsParameters );
    QgsOwsConnection::settingsConnectionPagingEnabled.setValue( cbxWfsFeaturePaging->isChecked(), detailsParameters );
  }

  QStringList credentialsParameters = {mServiceName, newConnectionName};
  QgsOwsConnection::settingsConnectionUsername.setValue( mAuthSettings->username(), credentialsParameters );
  QgsOwsConnection::settingsConnectionPassword.setValue( mAuthSettings->password(), credentialsParameters );
  QgsOwsConnection::settingsConnectionAuthCfg.setValue( mAuthSettings->configId(), credentialsParameters );

  if ( mHttpHeaders->isVisible() )
    mHttpHeaders->updateSettings( settings, QStringLiteral( "qgis/connections-%1/%2" ).arg( mServiceName.toLower(), newConnectionName ) ); // why is it done twice (see just above)?

  QgsOwsConnection::settingsConnectionSelected.setValue( newConnectionName, mServiceName );

  QDialog::accept();
}

void QgsNewHttpConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_ogc/index.html" ) );
}
