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

#include <QMessageBox>

#include "qgspointcloudsourceselect.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgshelp.h"

///@cond PRIVATE

QgsPointCloudSourceSelect::QgsPointCloudSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, widgetMode )
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
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mPath = path;
    emit enableButtons( ! mPath.isEmpty() );
  } );

  connect( protocolURI, &QLineEdit::textChanged, this, [ = ]( const QString & path )
  {
    mPath = path;
    emit enableButtons( ! mPath.isEmpty() );
  } );


  const QStringList protocolTypes = QStringLiteral( "HTTP/HTTPS/FTP,vsicurl" ).split( ';' );
  for ( int i = 0; i < protocolTypes.count(); i++ )
  {
    const QString protocolType = protocolTypes.at( i );
    if ( ( !protocolType.isEmpty() ) && ( !protocolType.isNull() ) )
      cmbProtocolTypes->addItem( protocolType.split( ',' ).at( 0 ) );
  }
}

void QgsPointCloudSourceSelect::addButtonClicked()
{
  if ( mDataSourceType == QLatin1String( "file" ) )
  {
    if ( mPath.isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Add Point Cloud Layers" ),
                                tr( "No layers selected." ) );
      return;
    }

    for ( const QString &path : QgsFileWidget::splitFilePaths( mPath ) )
    {
      // maybe we should raise an assert if preferredProviders size is 0 or >1? Play it safe for now...
      const QList< QgsProviderRegistry::ProviderCandidateDetails > preferredProviders = QgsProviderRegistry::instance()->preferredProvidersForUri( path );
      // if no preferred providers we can still give pdal a try
      const QString providerKey = preferredProviders.empty() ? QStringLiteral( "pdal" ) : preferredProviders.first().metadata()->key();
      Q_NOWARN_DEPRECATED_PUSH
      emit addPointCloudLayer( path, QFileInfo( path ).baseName(), providerKey ) ;
      Q_NOWARN_DEPRECATED_POP
      emit addLayer( Qgis::LayerType::PointCloud, path, QFileInfo( path ).baseName(), providerKey );
    }
  }
  else if ( mDataSourceType == QLatin1String( "remote" ) )
  {
    if ( mPath.isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Add Point Cloud Layers" ),
                                tr( "No layers selected." ) );
      return;
    }

    QUrl url = QUrl::fromUserInput( mPath );
    QString fileName = url.fileName();

    if ( fileName.compare( QLatin1String( "ept.json" ), Qt::CaseInsensitive ) != 0 && !fileName.endsWith( QLatin1String( ".copc.laz" ), Qt::CaseInsensitive ) )
    {
      QMessageBox::information( this,
                                tr( "Add Point Cloud Layers" ),
                                tr( "Invalid point cloud URL \"%1\", please make sure your URL ends with /ept.json or .copc.laz" ).arg( mPath ) );
      return;
    }

    // auto determine preferred provider for each path
    const QList< QgsProviderRegistry::ProviderCandidateDetails > preferredProviders = QgsProviderRegistry::instance()->preferredProvidersForUri( mPath );
    // maybe we should raise an assert if preferredProviders size is 0 or >1? Play it safe for now...
    if ( !preferredProviders.empty() )
    {
      QString baseName = QStringLiteral( "remote ept layer" );
      if ( mPath.endsWith( QLatin1String( "/ept.json" ), Qt::CaseInsensitive ) )
      {
        QStringList separatedPath = mPath.split( '/' );
        if ( separatedPath.size() >= 2 )
          baseName = separatedPath[ separatedPath.size() - 2 ];
      }
      if ( mPath.endsWith( QLatin1String( ".copc.laz" ), Qt::CaseInsensitive ) )
      {
        baseName = QFileInfo( mPath ).baseName();
      }
      Q_NOWARN_DEPRECATED_PUSH
      emit addPointCloudLayer( mPath, baseName, preferredProviders.at( 0 ).metadata()->key() ) ;
      Q_NOWARN_DEPRECATED_POP
      emit addLayer( Qgis::LayerType::PointCloud, mPath, baseName, preferredProviders.at( 0 ).metadata()->key() );
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

    mDataSourceType = QStringLiteral( "file" );

    emit enableButtons( ! mFileWidget->filePath().isEmpty() );
  }
}

void QgsPointCloudSourceSelect::radioSrcProtocol_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->hide();
    protocolGroupBox->show();

    mDataSourceType = QStringLiteral( "remote" );

    setProtocolWidgetsVisibility();

    emit enableButtons( ! protocolURI->text().isEmpty() );
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
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#loading-a-layer-from-a-file" ) );
}

///@endcond
