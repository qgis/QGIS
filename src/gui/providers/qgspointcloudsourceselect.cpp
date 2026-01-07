/***************************************************************************
                         qgspointcloudsourceselect.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudsourceselect.h"

#include "qgshelp.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"

#include <QMessageBox>

#include "moc_qgspointcloudsourceselect.cpp"

///@cond PRIVATE

QgsPointCloudSourceSelect::QgsPointCloudSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  connect( mRadioSrcFile, &QRadioButton::toggled, this, &QgsPointCloudSourceSelect::radioSrcFile_toggled );
  connect( mRadioSrcProtocol, &QRadioButton::toggled, this, &QgsPointCloudSourceSelect::radioSrcProtocol_toggled );
  connect( cmbProtocolTypes, &QComboBox::currentTextChanged, this, &QgsPointCloudSourceSelect::cmbProtocolTypes_currentIndexChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsPointCloudSourceSelect::showHelp );

  radioSrcFile_toggled( true );
  setProtocolWidgetsVisibility();

  mFileWidget->setDialogTitle( tr( "Open Point Cloud Dataset" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->filePointCloudFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [this]( const QString &path ) {
    mPath = path;
    emit enableButtons( !mPath.isEmpty() );
  } );

  connect( protocolURI, &QLineEdit::textChanged, this, [this]( const QString &path ) {
    mPath = path;
    emit enableButtons( !mPath.isEmpty() );
  } );


  const QStringList protocolTypes = u"HTTP/HTTPS/FTP,vsicurl"_s.split( ';' );
  for ( int i = 0; i < protocolTypes.count(); i++ )
  {
    const QString protocolType = protocolTypes.at( i );
    if ( ( !protocolType.isEmpty() ) && ( !protocolType.isNull() ) )
      cmbProtocolTypes->addItem( protocolType.split( ',' ).at( 0 ) );
  }
}

void QgsPointCloudSourceSelect::addButtonClicked()
{
  if ( mDataSourceType == "file"_L1 )
  {
    if ( mPath.isEmpty() )
    {
      QMessageBox::information( this, tr( "Add Point Cloud Layers" ), tr( "No layers selected." ) );
      return;
    }

    for ( const QString &path : QgsFileWidget::splitFilePaths( mPath ) )
    {
      // maybe we should raise an assert if preferredProviders size is 0 or >1? Play it safe for now...
      const QList<QgsProviderRegistry::ProviderCandidateDetails> preferredProviders = QgsProviderRegistry::instance()->preferredProvidersForUri( path );
      // if no preferred providers we can still give pdal a try
      const QString providerKey = preferredProviders.empty() ? u"pdal"_s : preferredProviders.first().metadata()->key();
      Q_NOWARN_DEPRECATED_PUSH
      emit addPointCloudLayer( path, QFileInfo( path ).baseName(), providerKey );
      Q_NOWARN_DEPRECATED_POP
      emit addLayer( Qgis::LayerType::PointCloud, path, QFileInfo( path ).baseName(), providerKey );
    }
  }
  else if ( mDataSourceType == "remote"_L1 )
  {
    if ( mPath.isEmpty() )
    {
      QMessageBox::information( this, tr( "Add Point Cloud Layers" ), tr( "No layers selected." ) );
      return;
    }

    QUrl url = QUrl::fromUserInput( mPath );
    QString fileName = url.fileName();

    if ( fileName.compare( "ept.json"_L1, Qt::CaseInsensitive ) != 0 && !fileName.endsWith( ".copc.laz"_L1, Qt::CaseInsensitive ) && !fileName.endsWith( ".vpc"_L1, Qt::CaseInsensitive ) )
    {
      QMessageBox::information( this, tr( "Add Point Cloud Layers" ), tr( "Invalid point cloud URL \"%1\", please make sure your URL ends with /ept.json or .copc.laz or .vpc" ).arg( mPath ) );
      return;
    }

    // auto determine preferred provider for each path
    const QList<QgsProviderRegistry::ProviderCandidateDetails> preferredProviders = QgsProviderRegistry::instance()->preferredProvidersForUri( mPath );
    // maybe we should raise an assert if preferredProviders size is 0 or >1? Play it safe for now...
    if ( !preferredProviders.empty() )
    {
      QString baseName = u"remote ept layer"_s;
      if ( mPath.endsWith( "/ept.json"_L1, Qt::CaseInsensitive ) )
      {
        QStringList separatedPath = mPath.split( '/' );
        if ( separatedPath.size() >= 2 )
          baseName = separatedPath[separatedPath.size() - 2];
      }
      if ( mPath.endsWith( ".copc.laz"_L1, Qt::CaseInsensitive ) )
      {
        baseName = QFileInfo( mPath ).baseName();
      }

      QVariantMap parts;
      if ( mAuthSettingsProtocol->configurationTabIsSelected() )
      {
        const QString authcfg = mAuthSettingsProtocol->configId();
        if ( !authcfg.isEmpty() )
          parts.insert( u"authcfg"_s, authcfg );
      }
      else
      {
        const QString username = mAuthSettingsProtocol->username();
        const QString password = mAuthSettingsProtocol->password();
        if ( !username.isEmpty() && !password.isEmpty() )
          mPath.replace( "://"_L1, u"://%1:%2@"_s.arg( username, password ) );
      }

      parts.insert( u"path"_s, mPath );
      const QString dsUri = preferredProviders.at( 0 ).metadata()->encodeUri( parts );

      Q_NOWARN_DEPRECATED_PUSH
      emit addPointCloudLayer( dsUri, baseName, preferredProviders.at( 0 ).metadata()->key() );
      Q_NOWARN_DEPRECATED_POP
      emit addLayer( Qgis::LayerType::PointCloud, dsUri, baseName, preferredProviders.at( 0 ).metadata()->key() );
    }
  }
}

void QgsPointCloudSourceSelect::radioSrcFile_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->show();
    protocolGroupBox->hide();

    mFileWidget->setDialogTitle( tr( "Open Point Cloud Dataset" ) );
    mFileWidget->setFilter( QgsProviderRegistry::instance()->filePointCloudFilters() );
    mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );

    mDataSourceType = u"file"_s;

    emit enableButtons( !mFileWidget->filePath().isEmpty() );
  }
}

void QgsPointCloudSourceSelect::radioSrcProtocol_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->hide();
    protocolGroupBox->show();

    mDataSourceType = u"remote"_s;

    setProtocolWidgetsVisibility();

    emit enableButtons( !protocolURI->text().isEmpty() );
  }
}

void QgsPointCloudSourceSelect::cmbProtocolTypes_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text )
  setProtocolWidgetsVisibility();
}

void QgsPointCloudSourceSelect::setProtocolWidgetsVisibility()
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

void QgsPointCloudSourceSelect::showHelp()
{
  QgsHelp::openHelp( u"managing_data_source/opening_data.html#loading-a-layer-from-a-file"_s );
}

///@endcond
