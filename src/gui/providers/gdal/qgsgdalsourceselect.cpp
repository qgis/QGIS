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
///@cond PRIVATE

#include <QMessageBox>

#include "qgsproviderregistry.h"
#include "ogr/qgsogrhelperfunctions.h"

#include <gdal.h>
#include <cpl_minixml.h>

QgsGdalSourceSelect::QgsGdalSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );

  connect( radioSrcFile, &QRadioButton::toggled, this, &QgsGdalSourceSelect::radioSrcFile_toggled );
  connect( radioSrcProtocol, &QRadioButton::toggled, this, &QgsGdalSourceSelect::radioSrcProtocol_toggled );
  connect( cmbProtocolTypes, &QComboBox::currentTextChanged, this, &QgsGdalSourceSelect::cmbProtocolTypes_currentIndexChanged );

  whileBlocking( radioSrcFile )->setChecked( true );
  protocolGroupBox->hide();

  QStringList protocolTypes = QStringLiteral( "HTTP/HTTPS/FTP,vsicurl;AWS S3,vsis3;Google Cloud Storage,vsigs" ).split( ';' );
  protocolTypes += QStringLiteral( "Microsoft Azure Blob,vsiaz;Alibaba Cloud OSS,vsioss;OpenStack Swift Object Storage,vsiswift" ).split( ';' );
  for ( int i = 0; i < protocolTypes.count(); i++ )
  {
    QString protocol = protocolTypes.at( i );
    if ( ( !protocol.isEmpty() ) && ( !protocol.isNull() ) )
      cmbProtocolTypes->addItem( protocol.split( ',' ).at( 0 ) );
  }

  mAuthWarning->setText( tr( " Additional credential options are required as documented <a href=\"%1\">here</a>." ).arg( QLatin1String( "https://gdal.org/user/virtual_file_systems.html#drivers-supporting-virtual-file-systems" ) ) );

  connect( protocolURI, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    if ( radioSrcProtocol->isChecked() )
    {
      emit enableButtons( !text.isEmpty() );
    }
  } );
  connect( mBucket, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    if ( radioSrcProtocol->isChecked() )
    {
      emit enableButtons( !text.isEmpty() && !mKey->text().isEmpty() );
      fillOpenOptions();
    }
  } );
  connect( mKey, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
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
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    mRasterPath = path;
    emit enableButtons( ! mRasterPath.isEmpty() );
    fillOpenOptions();
  } );
  mOpenOptionsGroupBox->setVisible( false );
  mAuthSettingsProtocol->setDataprovider( QStringLiteral( "gdal" ) );
}

bool QgsGdalSourceSelect::isProtocolCloudType()
{
  return ( cmbProtocolTypes->currentText() == QLatin1String( "AWS S3" ) ||
           cmbProtocolTypes->currentText() == QLatin1String( "Google Cloud Storage" ) ||
           cmbProtocolTypes->currentText() == QLatin1String( "Microsoft Azure Blob" ) ||
           cmbProtocolTypes->currentText() == QLatin1String( "Alibaba Cloud OSS" ) ||
           cmbProtocolTypes->currentText() == QLatin1String( "OpenStack Swift Object Storage" ) );
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
    clearOpenOptions();

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
    clearOpenOptions();

    emit enableButtons( !protocolURI->text().isEmpty() );
  }
}

void QgsGdalSourceSelect::cmbProtocolTypes_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text )
  setProtocolWidgetsVisibility();
  clearOpenOptions();
}

void QgsGdalSourceSelect::addButtonClicked()
{
  computeDataSources();

  if ( mDataSources.isEmpty() )
  {
    QMessageBox::information( this,
                              tr( "Add raster layer" ),
                              tr( "No layers selected." ) );
    return;
  }

  emit addRasterLayers( mDataSources );
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
    if ( !value.isEmpty() )
    {
      openOptions << QStringLiteral( "%1=%2" ).arg( control->objectName() ).arg( value );
    }
  }

  if ( radioSrcFile->isChecked() )
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
    bool cloudType = isProtocolCloudType();
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
    parts.insert( QStringLiteral( "path" ),
                  createProtocolURI( cmbProtocolTypes->currentText(),
                                     uri,
                                     mAuthSettingsProtocol->configId(),
                                     mAuthSettingsProtocol->username(),
                                     mAuthSettingsProtocol->password() ) );
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

  GDALDriverH hDriver;
  hDriver = GDALIdentifyDriverEx( mDataSources[0].toUtf8().toStdString().c_str(), GDAL_OF_RASTER, nullptr, nullptr );
  if ( hDriver == nullptr )
    return;

  const char *pszOpenOptionList = GDALGetMetadataItem( hDriver, GDAL_DMD_OPENOPTIONLIST, nullptr );
  if ( pszOpenOptionList == nullptr )
    return;

  CPLXMLNode *psDoc = CPLParseXMLString( pszOpenOptionList );
  if ( psDoc == nullptr )
    return;
  CPLXMLNode *psOpenOptionList = CPLGetXMLNode( psDoc, "=OpenOptionList" );
  if ( psOpenOptionList == nullptr )
  {
    CPLDestroyXMLNode( psDoc );
    return;
  }

  for ( auto psItem = psOpenOptionList->psChild; psItem != nullptr; psItem = psItem->psNext )
  {
    if ( psItem->eType != CXT_Element || !EQUAL( psItem->pszValue, "Option" ) )
      continue;

    const char *pszOptionName = CPLGetXMLValue( psItem, "name", nullptr );
    if ( pszOptionName == nullptr )
      continue;

    // Exclude options that are not of raster scope
    const char *pszScope = CPLGetXMLValue( psItem, "scope", nullptr );
    if ( pszScope != nullptr && strstr( pszScope, "raster" ) == nullptr )
      continue;

    const char *pszType = CPLGetXMLValue( psItem, "type", nullptr );
    QStringList options;
    if ( pszType && EQUAL( pszType, "string-select" ) )
    {
      for ( auto psOption = psItem->psChild; psOption != nullptr; psOption = psOption->psNext )
      {
        if ( psOption->eType != CXT_Element ||
             !EQUAL( psOption->pszValue, "Value" ) ||
             psOption->psChild == nullptr )
        {
          continue;
        }
        options << psOption->psChild->pszValue;
      }
    }

    QLabel *label = new QLabel( pszOptionName );
    QWidget *control = nullptr;
    if ( pszType && EQUAL( pszType, "boolean" ) )
    {
      QComboBox *cb = new QComboBox();
      cb->addItem( tr( "Yes" ), "YES" );
      cb->addItem( tr( "No" ), "NO" );
      cb->addItem( tr( "<Default>" ), QVariant( QVariant::String ) );
      int idx = cb->findData( QVariant( QVariant::String ) );
      cb->setCurrentIndex( idx );
      control = cb;
    }
    else if ( !options.isEmpty() )
    {
      QComboBox *cb = new QComboBox();
      for ( const QString &val : std::as_const( options ) )
      {
        cb->addItem( val, val );
      }
      cb->addItem( tr( "<Default>" ), QVariant( QVariant::String ) );
      int idx = cb->findData( QVariant( QVariant::String ) );
      cb->setCurrentIndex( idx );
      control = cb;
    }
    else
    {
      QLineEdit *le = new QLineEdit( );
      control = le;
    }
    control->setObjectName( pszOptionName );
    mOpenOptionsWidgets.push_back( control );

    const char *pszDescription = CPLGetXMLValue( psItem, "description", nullptr );
    if ( pszDescription )
    {
      label->setToolTip( QStringLiteral( "<p>%1</p>" ).arg( pszDescription ) );
      control->setToolTip( QStringLiteral( "<p>%1</p>" ).arg( pszDescription ) );
    }
    mOpenOptionsLayout->addRow( label, control );
  }

  CPLDestroyXMLNode( psDoc );

  // Set label to point to driver help page
  const char *pszHelpTopic = GDALGetMetadataItem( hDriver, GDAL_DMD_HELPTOPIC, nullptr );
  if ( pszHelpTopic )
  {
    mOpenOptionsLabel->setText( tr( "Consult <a href=\"https://gdal.org/%1\">%2 driver help page</a> for detailed explanations on options" ).arg( pszHelpTopic ).arg( GDALGetDriverShortName( hDriver ) ) );
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
