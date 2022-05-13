/***************************************************************************
    qgsprojectelevationsettingswidget.cpp
    ---------------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectelevationsettingswidget.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsterrainprovider.h"
#include "qgsprojectelevationproperties.h"

QgsProjectElevationSettingsWidget::QgsProjectElevationSettingsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  mFlatHeightSpinBox->setClearValue( 0.0 );

  mDemOffsetSpinBox->setClearValue( 0.0 );
  mDemScaleSpinBox->setClearValue( 1.0 );
  mComboDemLayer->setFilters( QgsMapLayerProxyModel::RasterLayer );

  mMeshOffsetSpinBox->setClearValue( 0.0 );
  mMeshScaleSpinBox->setClearValue( 1.0 );
  mComboMeshLayer->setFilters( QgsMapLayerProxyModel::MeshLayer );

  mComboTerrainType->addItem( tr( "Flat Terrain" ), QStringLiteral( "flat" ) );
  mComboTerrainType->addItem( tr( "DEM (Raster Layer)" ), QStringLiteral( "raster" ) );
  mComboTerrainType->addItem( tr( "Mesh" ), QStringLiteral( "mesh" ) );

  mStackedWidget->setCurrentWidget( mPageFlat );
  connect( mComboTerrainType, qOverload< int >( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    const QString terrainType = mComboTerrainType->currentData().toString();
    if ( terrainType == QLatin1String( "flat" ) )
    {
      mStackedWidget->setCurrentWidget( mPageFlat );
    }
    else if ( terrainType == QLatin1String( "raster" ) )
    {
      mStackedWidget->setCurrentWidget( mPageRasterDem );
    }
    else if ( terrainType == QLatin1String( "mesh" ) )
    {
      mStackedWidget->setCurrentWidget( mPageMesh );
    }
    validate();
  } );

  // setup with current settings
  const QgsAbstractTerrainProvider *provider = QgsProject::instance()->elevationProperties()->terrainProvider();
  mComboTerrainType->setCurrentIndex( mComboTerrainType->findData( provider->type() ) );
  if ( provider->type() == QLatin1String( "flat" ) )
  {
    mStackedWidget->setCurrentWidget( mPageFlat );
    mFlatHeightSpinBox->setValue( provider->offset() );
  }
  else if ( provider->type() == QLatin1String( "raster" ) )
  {
    mStackedWidget->setCurrentWidget( mPageRasterDem );
    mDemOffsetSpinBox->setValue( provider->offset() );
    mDemScaleSpinBox->setValue( provider->scale() );
    mComboDemLayer->setLayer( qgis::down_cast< const QgsRasterDemTerrainProvider * >( provider )->layer() );
  }
  else if ( provider->type() == QLatin1String( "mesh" ) )
  {
    mStackedWidget->setCurrentWidget( mPageMesh );
    mMeshOffsetSpinBox->setValue( provider->offset() );
    mMeshScaleSpinBox->setValue( provider->scale() );
    mComboMeshLayer->setLayer( qgis::down_cast< const QgsMeshTerrainProvider * >( provider )->layer() );
  }

  connect( mComboDemLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsProjectElevationSettingsWidget::validate );
  connect( mComboMeshLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsProjectElevationSettingsWidget::validate );

  validate();
}

void QgsProjectElevationSettingsWidget::apply()
{
  const QString terrainType = mComboTerrainType->currentData().toString();
  std::unique_ptr< QgsAbstractTerrainProvider > provider;
  if ( terrainType == QLatin1String( "flat" ) )
  {
    provider = std::make_unique< QgsFlatTerrainProvider >();
    provider->setOffset( mFlatHeightSpinBox->value() );
    provider->setScale( 1.0 );
  }
  else if ( terrainType == QLatin1String( "raster" ) )
  {
    provider = std::make_unique< QgsRasterDemTerrainProvider >();
    provider->setOffset( mDemOffsetSpinBox->value() );
    provider->setScale( mDemScaleSpinBox->value() );
    qgis::down_cast< QgsRasterDemTerrainProvider * >( provider.get() )->setLayer( qobject_cast< QgsRasterLayer * >( mComboDemLayer->currentLayer() ) );
  }
  else if ( terrainType == QLatin1String( "mesh" ) )
  {
    provider = std::make_unique< QgsMeshTerrainProvider >();
    provider->setOffset( mMeshOffsetSpinBox->value() );
    provider->setScale( mMeshScaleSpinBox->value() );
    qgis::down_cast< QgsMeshTerrainProvider * >( provider.get() )->setLayer( qobject_cast< QgsMeshLayer * >( mComboMeshLayer->currentLayer() ) );
  }

  QgsProject::instance()->elevationProperties()->setTerrainProvider( provider.release() );
}

bool QgsProjectElevationSettingsWidget::validate()
{
  mMessageBar->clearWidgets();

  bool valid = true;
  const QString terrainType = mComboTerrainType->currentData().toString();
  if ( terrainType == QLatin1String( "raster" ) )
  {
    if ( !mComboDemLayer->currentLayer() )
    {
      valid = false;
      mMessageBar->pushMessage( tr( "An elevation layer must be selected for a DEM terrain" ), Qgis::MessageLevel::Critical );
    }
  }
  else if ( terrainType == QLatin1String( "mesh" ) )
  {
    if ( !mComboMeshLayer->currentLayer() )
    {
      valid = false;
      mMessageBar->pushMessage( tr( "An elevation layer must be selected for a mesh terrain" ), Qgis::MessageLevel::Critical );
    }
  }
  return valid;
}

bool QgsProjectElevationSettingsWidget::isValid()
{
  return validate();
}


//
// QgsProjectElevationSettingsWidgetFactory
//

QgsProjectElevationSettingsWidgetFactory::QgsProjectElevationSettingsWidgetFactory( QObject *parent )
  : QgsOptionsWidgetFactory( tr( "Terrain" ), QgsApplication::getThemeIcon( QStringLiteral( "mLayoutItem3DMap.svg" ) ) )
{
  setParent( parent );
}


QgsOptionsPageWidget *QgsProjectElevationSettingsWidgetFactory::createWidget( QWidget *parent ) const
{
  return new QgsProjectElevationSettingsWidget( parent );
}
