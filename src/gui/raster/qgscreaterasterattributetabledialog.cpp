/***************************************************************************
  qgscreaterasterattributetabledialog.cpp - QgsCreateRasterAttributeTableDialog

 ---------------------
 begin                : 13.10.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscreaterasterattributetabledialog.h"
#include "qgsrasterlayer.h"
#include "qgsmessagebar.h"
#include "qgsgui.h"
#include <QMessageBox>

QgsCreateRasterAttributeTableDialog::QgsCreateRasterAttributeTableDialog( QgsRasterLayer *rasterLayer, QWidget *parent )
  : QDialog( parent )
  , mRasterLayer( rasterLayer )
{
  Q_ASSERT( mRasterLayer );
  Q_ASSERT( mRasterLayer->canCreateRasterAttributeTable() );

  setupUi( this );

  // Apparently, some drivers (HFA) ignore Min/Max fields and set them to generic,
  // for this reason we disable the native support for thematic RATs (later in the loop)
  bool nativeRatSupported { mRasterLayer->dataProvider()->providerCapabilities().testFlag( QgsRasterDataProvider::ProviderCapability::NativeRasterAttributeTable ) };

  connect( mNativeRadioButton, &QRadioButton::toggled, this, &QgsCreateRasterAttributeTableDialog::updateButtons );
  connect( mDbfRadioButton, &QRadioButton::toggled, this, &QgsCreateRasterAttributeTableDialog::updateButtons );


  // Check for existing RATs
  QStringList existingRatsInfo;
  if ( mRasterLayer->attributeTableCount() > 0 )
  {
    for ( int bandNo = 1; bandNo <= mRasterLayer->bandCount(); ++bandNo )
    {
      if ( QgsRasterAttributeTable *rat = mRasterLayer->attributeTable( bandNo ) )
      {
        // disable the native support for thematic RATs
        if ( nativeRatSupported && rat->type() != Qgis::RasterAttributeTableType::Athematic )
        {
          nativeRatSupported = false;
          existingRatsInfo.push_back( tr( "The data provider supports attribute table storage but some drivers do not support 'thematic' types, for this reason the option is disabled." ) );
        }
        if ( ! rat->filePath().isEmpty() )
        {
          existingRatsInfo.push_back( tr( "Raster band %1 already has an associated attribute table at %2." ).arg( QString::number( bandNo ), rat->filePath() ) );
        }
        else
        {
          existingRatsInfo.push_back( tr( "Raster band %1 already has an associated attribute table." ).arg( bandNo ) );
        }
      }
    }
  }

  if ( ! existingRatsInfo.isEmpty() )
  {
    mCreateInfoLabel->setText( mCreateInfoLabel->text().append( QStringLiteral( "<br><ul><li>" ) + existingRatsInfo.join( QStringLiteral( "</li><li>" ) ) ).append( QStringLiteral( "</ul>" ) ) );
    mCreateInfoLabel->adjustSize();
    mCreateInfoLabel->show();
  }

  if ( ! nativeRatSupported )
  {
    mNativeRadioButton->setEnabled( false );
    mDbfRadioButton->setChecked( true );
  }
  else
  {
    mDbfPathWidget->setFilter( QStringLiteral( "VAT DBF Files (*.vat.dbf)" ) );
    if ( QFile::exists( mRasterLayer->dataProvider()->dataSourceUri( ) ) )
    {
      mDbfPathWidget->setFilePath( mRasterLayer->dataProvider()->dataSourceUri( ) + ".vat.dbf" );
    }
  }

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  updateButtons();

  QgsGui::enableAutoGeometryRestore( this );
}

QString QgsCreateRasterAttributeTableDialog::filePath() const
{
  return mDbfPathWidget->filePath();
}

bool QgsCreateRasterAttributeTableDialog::saveToFile() const
{
  return mDbfRadioButton->isChecked();
}

bool QgsCreateRasterAttributeTableDialog::openWhenDone() const
{
  return mOpenRat->isChecked();
}

void QgsCreateRasterAttributeTableDialog::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

void QgsCreateRasterAttributeTableDialog::setOpenWhenDoneVisible( bool visible )
{
  if ( ! visible )
  {
    mOpenRat->setChecked( false );
  }

  mOpenRat->setVisible( visible );
}

void QgsCreateRasterAttributeTableDialog::accept()
{
  QString errorMessage;
  int bandNumber { 0 };
  QgsRasterAttributeTable *rat { QgsRasterAttributeTable::createFromRaster( mRasterLayer, &bandNumber ) };
  bool success { false };

  if ( ! rat )
  {
    notify( tr( "Error Creating Raster Attribute Table" ),
            tr( "The raster attribute table could not be created." ),
            Qgis::MessageLevel::Critical );
  }
  else
  {
    mRasterLayer->dataProvider()->setAttributeTable( bandNumber, rat );

    // Save it
    const bool storageIsFile { saveToFile() };
    if ( storageIsFile )
    {
      const QString destinationPath { filePath() };
      if ( QFile::exists( destinationPath ) && QMessageBox::warning( nullptr, tr( "Confirm Overwrite" ), tr( "Are you sure you want to overwrite the existing attribute table at '%1'?" ).arg( destinationPath ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
      {
        success = rat->writeToFile( destinationPath, &errorMessage );
        if ( ! success )
        {
          notify( tr( "Error Saving Raster Attribute Table" ),
                  errorMessage,
                  Qgis::MessageLevel::Critical );
          mRasterLayer->dataProvider()->setAttributeTable( bandNumber, nullptr );
        }
      }
    }
    else
    {
      success = mRasterLayer->dataProvider()->writeNativeAttributeTable( &errorMessage ); //#spellok
      if ( ! success )
      {
        notify( tr( "Error Saving Raster Attribute Table" ),
                errorMessage,
                Qgis::MessageLevel::Critical );
        mRasterLayer->dataProvider()->setAttributeTable( bandNumber, nullptr );

      }
    }
  }

  if ( success )
  {
    notify( tr( "Raster Attribute Table Saved" ),
            tr( "The new Raster Attribute Table was successfully created." ),
            Qgis::MessageLevel::Success );
  }

  QDialog::accept();
}

void QgsCreateRasterAttributeTableDialog::notify( const QString &title, const QString &message, Qgis::MessageLevel level )
{
  if ( mMessageBar )
  {
    mMessageBar->pushMessage( message, level );
  }
  else
  {
    switch ( level )
    {
      case Qgis::MessageLevel::Info:
      case Qgis::MessageLevel::Success:
      case Qgis::MessageLevel::NoLevel:
      {
        QMessageBox::information( nullptr, title, message );
        break;
      }
      case Qgis::MessageLevel::Warning:
      {
        QMessageBox::warning( nullptr, title, message );
        break;
      }
      case Qgis::MessageLevel::Critical:
      {
        QMessageBox::critical( nullptr, title, message );
        break;
      }
    }
  }
}

void QgsCreateRasterAttributeTableDialog::updateButtons()
{
  mDbfPathWidget->setEnabled( mDbfRadioButton->isChecked() );
}
