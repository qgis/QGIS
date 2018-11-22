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

#include <QMessageBox>

#include "qgsgdalsourceselect.h"
#include "qgsproviderregistry.h"
#include "ogr/qgsogrhelperfunctions.h"

#include <gdal.h>

QgsGdalSourceSelect::QgsGdalSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  connect( radioSrcFile, &QRadioButton::toggled, this, &QgsGdalSourceSelect::radioSrcFile_toggled );
  connect( radioSrcProtocol, &QRadioButton::toggled, this, &QgsGdalSourceSelect::radioSrcProtocol_toggled );
  connect( cmbProtocolTypes, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsGdalSourceSelect::cmbProtocolTypes_currentIndexChanged );

  whileBlocking( radioSrcFile )->setChecked( true );
  protocolGroupBox->hide();

  QStringList protocolTypes = QString( "HTTP/HTTPS/FTP,vsicurl;AWS S3,vsis3;Google Cloud Storage,vsigs;" ).split( ';' );
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,3,0)
  protocolTypes += QString( "Microsoft Azure Blob,vsiaz;Alibaba Cloud OSS,vsioss;OpenStack Swift Object Storage,vsiswift;" ).split( ';' );
#endif
  for ( int i = 0; i < protocolTypes.count(); i++ )
  {
    QString protocol = protocolTypes.at( i );
    if ( ( !protocol.isEmpty() ) && ( !protocol.isNull() ) )
      cmbProtocolTypes->addItem( protocol.split( ',' ).at( 0 ) );
  }

  mAuthWarning->setText( tr( " Additional credential options are required as documented <a href=\"%1\">here</a>." ).arg( QStringLiteral( "http://gdal.org/gdal_virtual_file_systems.html#gdal_virtual_file_systems_network" ) ) );

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

  mFileWidget->setDialogTitle( tr( "Open GDAL Supported Raster Dataset(s)" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileRasterFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetMultipleFiles );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mRasterPath = path;
    emit enableButtons( ! mRasterPath.isEmpty() );
  } );
}

bool QgsGdalSourceSelect::isProtocolCloudType()
{
  return ( cmbProtocolTypes->currentText() == QStringLiteral( "AWS S3" ) ||
           cmbProtocolTypes->currentText() == QStringLiteral( "Google Cloud Storage" ) ||
           cmbProtocolTypes->currentText() == QStringLiteral( "Microsoft Azure Blob" ) ||
           cmbProtocolTypes->currentText() == QStringLiteral( "Alibaba Cloud OSS" ) ||
           cmbProtocolTypes->currentText() == QStringLiteral( "OpenStack Swift Object Storage" ) );
}

void QgsGdalSourceSelect::setProtocolWidgetsVisibility()
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

void QgsGdalSourceSelect::radioSrcFile_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->show();
    protocolGroupBox->hide();

    emit enableButtons( !mFileWidget->filePath().isEmpty() );
  }
}

void QgsGdalSourceSelect::radioSrcProtocol_toggled( bool checked )
{
  if ( checked )
  {
    fileGroupBox->hide();
    protocolGroupBox->show();

    setProtocolWidgetsVisibility();

    emit enableButtons( !protocolURI->text().isEmpty() );
  }
}

void QgsGdalSourceSelect::cmbProtocolTypes_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text );
  setProtocolWidgetsVisibility();
}

void QgsGdalSourceSelect::addButtonClicked()
{
  if ( radioSrcFile->isChecked() )
  {
    if ( mRasterPath.isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Add raster layer" ),
                                tr( "No layers selected." ) );
      return;
    }

    Q_FOREACH ( const QString &path, QgsFileWidget::splitFilePaths( mRasterPath ) )
    {
      emit addRasterLayer( path, QFileInfo( path ).completeBaseName(), QStringLiteral( "gdal" ) );
    }
  }
  else if ( radioSrcProtocol->isChecked() )
  {
    bool cloudType = isProtocolCloudType();
    if ( !cloudType && protocolURI->text().isEmpty() )
    {
      QMessageBox::information( this,
                                tr( "Add raster layer" ),
                                tr( "No protocol URI entered." ) );
      return;
    }
    else if ( cloudType && ( mBucket->text().isEmpty() || mKey->text().isEmpty() ) )
    {
      QMessageBox::information( this,
                                tr( "Add raster layer" ),
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

    QString dataSource = createProtocolURI( cmbProtocolTypes->currentText(),
                                            uri,
                                            mAuthSettingsProtocol->configId(),
                                            mAuthSettingsProtocol->username(),
                                            mAuthSettingsProtocol->password() );
    emit addRasterLayer( dataSource, dataSource, QStringLiteral( "gdal" ) );
  }
}

QGISEXTERN QgsGdalSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsGdalSourceSelect( parent, fl, widgetMode );
}
