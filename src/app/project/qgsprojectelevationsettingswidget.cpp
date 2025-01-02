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
#include "moc_qgsprojectelevationsettingswidget.cpp"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsterrainprovider.h"
#include "qgsprojectelevationproperties.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgselevationshadingrenderersettingswidget.h"
#include "qgsprojectionselectionwidget.h"

QgsProjectElevationSettingsWidget::QgsProjectElevationSettingsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  mElevationLowerSpin->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  mElevationUpperSpin->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );

  mFlatHeightSpinBox->setClearValue( 0.0 );

  mDemOffsetSpinBox->setClearValue( 0.0 );
  mDemScaleSpinBox->setClearValue( 1.0 );
  mComboDemLayer->setFilters( Qgis::LayerFilter::RasterLayer );

  mMeshOffsetSpinBox->setClearValue( 0.0 );
  mMeshScaleSpinBox->setClearValue( 1.0 );
  mComboMeshLayer->setFilters( Qgis::LayerFilter::MeshLayer );

  mComboTerrainType->addItem( tr( "Flat Terrain" ), QStringLiteral( "flat" ) );
  mComboTerrainType->addItem( tr( "DEM (Raster Layer)" ), QStringLiteral( "raster" ) );
  mComboTerrainType->addItem( tr( "Mesh" ), QStringLiteral( "mesh" ) );

  mVerticalCrsStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );

  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  mVerticalCrsWidget = new QgsProjectionSelectionWidget( nullptr, QgsCoordinateReferenceSystemProxyModel::FilterVertical );
  mVerticalCrsWidget->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, true );
  mVerticalCrsWidget->setNotSetText( tr( "Not set" ) );
  mVerticalCrsWidget->setDialogTitle( tr( "Project Vertical CRS" ) );
  vl->addWidget( mVerticalCrsWidget );
  mCrsPageEnabled->setLayout( vl );

  mStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );

  mStackedWidget->setCurrentWidget( mPageFlat );
  connect( mComboTerrainType, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=] {
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
  QgsProjectElevationProperties *elevationProperties = QgsProject::instance()->elevationProperties();
  const QgsAbstractTerrainProvider *provider = elevationProperties->terrainProvider();
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
    mComboDemLayer->setLayer( qgis::down_cast<const QgsRasterDemTerrainProvider *>( provider )->layer() );
  }
  else if ( provider->type() == QLatin1String( "mesh" ) )
  {
    mStackedWidget->setCurrentWidget( mPageMesh );
    mMeshOffsetSpinBox->setValue( provider->offset() );
    mMeshScaleSpinBox->setValue( provider->scale() );
    mComboMeshLayer->setLayer( qgis::down_cast<const QgsMeshTerrainProvider *>( provider )->layer() );
  }

  connect( mComboDemLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsProjectElevationSettingsWidget::validate );
  connect( mComboMeshLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsProjectElevationSettingsWidget::validate );

  if ( elevationProperties->elevationRange().lower() != std::numeric_limits<double>::lowest() )
    whileBlocking( mElevationLowerSpin )->setValue( elevationProperties->elevationRange().lower() );
  else
    whileBlocking( mElevationLowerSpin )->clear();
  if ( elevationProperties->elevationRange().upper() != std::numeric_limits<double>::max() )
    whileBlocking( mElevationUpperSpin )->setValue( elevationProperties->elevationRange().upper() );
  else
    whileBlocking( mElevationUpperSpin )->clear();

  connect( mElevationLowerSpin, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsProjectElevationSettingsWidget::validate );
  connect( mElevationUpperSpin, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsProjectElevationSettingsWidget::validate );

  updateVerticalCrsOptions();
  connect( QgsProject::instance(), &QgsProject::crsChanged, this, &QgsProjectElevationSettingsWidget::updateVerticalCrsOptions );

  validate();

  mElevationShadingSettingsWidget = new QgsElevationShadingRendererSettingsWidget( nullptr, nullptr, this );
  mElevationShadingSettingsLayout->addWidget( mElevationShadingSettingsWidget );
}

void QgsProjectElevationSettingsWidget::apply()
{
  const QString terrainType = mComboTerrainType->currentData().toString();
  std::unique_ptr<QgsAbstractTerrainProvider> provider;
  if ( terrainType == QLatin1String( "flat" ) )
  {
    provider = std::make_unique<QgsFlatTerrainProvider>();
    provider->setOffset( mFlatHeightSpinBox->value() );
    provider->setScale( 1.0 );
  }
  else if ( terrainType == QLatin1String( "raster" ) )
  {
    provider = std::make_unique<QgsRasterDemTerrainProvider>();
    provider->setOffset( mDemOffsetSpinBox->value() );
    provider->setScale( mDemScaleSpinBox->value() );
    QgsRasterLayer *demLayer = qobject_cast<QgsRasterLayer *>( mComboDemLayer->currentLayer() );
    // always mark the terrain layer as a "dem" layer -- it seems odd for a user to have to manually set this after picking a terrain raster!
    qobject_cast<QgsRasterLayerElevationProperties *>( demLayer->elevationProperties() )->setEnabled( true );
    qobject_cast<QgsRasterLayerElevationProperties *>( demLayer->elevationProperties() )->setMode( Qgis::RasterElevationMode::RepresentsElevationSurface );
    qgis::down_cast<QgsRasterDemTerrainProvider *>( provider.get() )->setLayer( demLayer );
  }
  else if ( terrainType == QLatin1String( "mesh" ) )
  {
    provider = std::make_unique<QgsMeshTerrainProvider>();
    provider->setOffset( mMeshOffsetSpinBox->value() );
    provider->setScale( mMeshScaleSpinBox->value() );
    qgis::down_cast<QgsMeshTerrainProvider *>( provider.get() )->setLayer( qobject_cast<QgsMeshLayer *>( mComboMeshLayer->currentLayer() ) );
  }

  QgsProject::instance()->elevationProperties()->setTerrainProvider( provider.release() );

  double zLower = mElevationLowerSpin->value();
  if ( zLower == mElevationLowerSpin->clearValue() )
    zLower = std::numeric_limits<double>::lowest();
  double zUpper = mElevationUpperSpin->value();
  if ( zUpper == mElevationUpperSpin->clearValue() )
    zUpper = std::numeric_limits<double>::max();

  QgsProject::instance()->elevationProperties()->setElevationRange( QgsDoubleRange( zLower, zUpper ) );

  QgsProject::instance()->setVerticalCrs( mVerticalCrsWidget->crs() );

  mElevationShadingSettingsWidget->apply();
}

void QgsProjectElevationSettingsWidget::updateVerticalCrsOptions()
{
  switch ( QgsProject::instance()->crs().type() )
  {
    case Qgis::CrsType::Compound:
      mVerticalCrsStackedWidget->setCurrentWidget( mCrsPageDisabled );
      mCrsDisabledLabel->setText( tr( "Project coordinate reference system is set to a compound CRS (%1), so the project's vertical CRS is the vertical component of this CRS (%2)." ).arg( QgsProject::instance()->crs().userFriendlyIdentifier(), QgsProject::instance()->verticalCrs().userFriendlyIdentifier() ) );
      break;

    case Qgis::CrsType::Geographic3d:
      mVerticalCrsStackedWidget->setCurrentWidget( mCrsPageDisabled );
      mCrsDisabledLabel->setText( tr( "Project coordinate reference system is set to a geographic 3D CRS (%1), so the vertical CRS cannot be manually specified." ).arg( QgsProject::instance()->crs().userFriendlyIdentifier() ) );
      break;

    case Qgis::CrsType::Geocentric:
      mVerticalCrsStackedWidget->setCurrentWidget( mCrsPageDisabled );
      mCrsDisabledLabel->setText( tr( "Project coordinate reference system is set to a geocentric CRS (%1), so the vertical CRS cannot be manually specified." ).arg( QgsProject::instance()->crs().userFriendlyIdentifier() ) );
      break;

    case Qgis::CrsType::Projected:
      if ( QgsProject::instance()->crs().hasVerticalAxis() )
      {
        mVerticalCrsStackedWidget->setCurrentWidget( mCrsPageDisabled );
        mCrsDisabledLabel->setText( tr( "Project coordinate reference system is set to a projected 3D CRS (%1), so the vertical CRS cannot be manually specified." ).arg( QgsProject::instance()->crs().userFriendlyIdentifier() ) );
        break;
      }
      [[fallthrough]];

    case Qgis::CrsType::Unknown:
    case Qgis::CrsType::Geodetic:
    case Qgis::CrsType::Geographic2d:
    case Qgis::CrsType::Vertical:
    case Qgis::CrsType::Temporal:
    case Qgis::CrsType::Engineering:
    case Qgis::CrsType::Bound:
    case Qgis::CrsType::Other:
    case Qgis::CrsType::DerivedProjected:
      mVerticalCrsStackedWidget->setCurrentWidget( mCrsPageEnabled );
      mVerticalCrsWidget->setCrs( QgsProject::instance()->verticalCrs() );
      break;
  }
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

  // Show an error message if the lower value is greater than the upper one
  // However, do not show the error message if one of the values is not set
  if ( !mElevationLowerSpin->isCleared() && !mElevationUpperSpin->isCleared() && ( mElevationLowerSpin->value() >= mElevationUpperSpin->value() ) )
  {
    valid = false;
    mMessageBar->pushMessage( tr( "Upper elevation range must be greater than the lower one" ), Qgis::MessageLevel::Critical );
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
  : QgsOptionsWidgetFactory( tr( "Elevation" ), QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ), QStringLiteral( "terrain" ) )
{
  setParent( parent );
}


QgsOptionsPageWidget *QgsProjectElevationSettingsWidgetFactory::createWidget( QWidget *parent ) const
{
  return new QgsProjectElevationSettingsWidget( parent );
}
