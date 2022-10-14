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

QgsCreateRasterAttributeTableDialog::QgsCreateRasterAttributeTableDialog( QgsRasterLayer *rasterLayer, QWidget *parent )
  : QDialog( parent )
  , mRasterLayer( rasterLayer )
{
  Q_ASSERT( mRasterLayer );
  Q_ASSERT( mRasterLayer->canCreateRasterAttributeTable() );

  setupUi( this );

  const bool nativeRatSupported { mRasterLayer->dataProvider()->providerCapabilities().testFlag( QgsRasterDataProvider::ProviderCapability::NativeRasterAttributeTable ) };

  connect( mNativeRadioButton, &QRadioButton::toggled, this, &QgsCreateRasterAttributeTableDialog::updateButtons );
  connect( mDbfRadioButton, &QRadioButton::toggled, this, &QgsCreateRasterAttributeTableDialog::updateButtons );

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

  // Check for existing rats
  QStringList existingRatsInfo;
  if ( mRasterLayer->attributeTableCount() > 0 )
  {
    for ( int bandNo = 1; bandNo <= mRasterLayer->bandCount(); ++bandNo )
    {
      if ( QgsRasterAttributeTable *rat = mRasterLayer->attributeTable( bandNo ) )
      {
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
  }

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
}

QString QgsCreateRasterAttributeTableDialog::filePath() const
{
  return mDbfPathWidget->filePath();
}

bool QgsCreateRasterAttributeTableDialog::openWhenDone() const
{
  return mOpenRat->isChecked();
}

void QgsCreateRasterAttributeTableDialog::updateButtons()
{
  mDbfPathWidget->setEnabled( mDbfRadioButton->isChecked() );
}
