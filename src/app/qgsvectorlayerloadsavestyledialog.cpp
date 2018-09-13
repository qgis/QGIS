/***************************************************************************
  qgsvectorlayerloadsavestyledialog.h
  --------------------------------------
  Date                 : September 2018
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QListWidgetItem>
#include <QMessageBox>

#include "qgsvectorlayerloadsavestyledialog.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgisapp.h"
#include "qgshelp.h"

QgsVectorLayerLoadSaveStyleDialog::QgsVectorLayerLoadSaveStyleDialog( QgsVectorLayer *layer, Mode mode, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
  , mMode( mode )
{
  setupUi( this );

  QString providerName = mLayer->providerType();
  if ( providerName == QLatin1String( "ogr" ) )
  {
    providerName = mLayer->dataProvider()->storageType();
    if ( providerName == QLatin1String( "GPKG" ) )
      providerName = QStringLiteral( "GeoPackage" );
  }

  //QStringLiteral( "style/lastStyleDir" )

  QgsSettings settings;
  QgsMapLayer::StyleCategories lastStyleCategories = settings.flagValue( QStringLiteral( "style/lastStyleCategories" ), QgsMapLayer::AllStyleCategories );


  switch ( mMode )
  {
    case Save:
      mModeLabel->setText( tr( "Save" ) );
      mStyleTypeComboBox->addItem( tr( "from file" ), GenericFile );
      if ( mLayer->dataProvider()->isSaveAndLoadStyleToDatabaseSupported() )
        mStyleTypeComboBox->addItem( tr( "from database (%1)" ).arg( providerName ), DB );
      break;
    case Load:
      mModeLabel->setText( tr( "Load" ) );
      mStyleTypeComboBox->addItem( tr( "as QGIS QML style file" ), QML );
      mStyleTypeComboBox->addItem( tr( "as SLD style file" ), SLD );
      if ( mLayer->dataProvider()->isSaveAndLoadStyleToDatabaseSupported() )
        mStyleTypeComboBox->addItem( tr( "in database (%1)" ).arg( providerName ), DB );
      break;
  }

  connect( mStyleTypeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int idx ) {mFileWidget->setVisible( mStyleTypeComboBox->itemData( idx ) != DB );} );

  for ( QgsMapLayer::StyleCategory category : qgsEnumMap<QgsMapLayer::StyleCategory>().keys() )
  {
    if ( category == QgsMapLayer::AllStyleCategories )
      continue;

    QgsMapLayer::ReadableStyleCategory readableCategory = QgsMapLayer::readableStyleCategory( category );

    QListWidgetItem *item = new QListWidgetItem( readableCategory.icon(), readableCategory.name(), mStyleCategoriesListWidget );
    item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
    item->setCheckState( lastStyleCategories.testFlag( category ) ? Qt::Checked : Qt::Unchecked );
    item->setData( Qt::UserRole, category );
  }
}

void QgsVectorLayerLoadSaveStyleDialog::accept()
{
  bool ok = true;
  QgsMapLayer::StyleCategories lastStyleCategories;
  for ( int row = 0; row < mStyleCategoriesListWidget->count(); ++row )
  {
    QListWidgetItem *item = mStyleCategoriesListWidget->item( row );
    if ( item->checkState() == Qt::Checked )
      lastStyleCategories |= static_cast<QgsMapLayer::StyleCategory>( item->data( Qt::UserRole ).toInt() );
  }

  if ( ok )
  {
    QgsSettings settings;
    settings.setFlagValue( QStringLiteral( "style/lastStyleCategories" ), lastStyleCategories );
    close();
  }
}
