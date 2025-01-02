/***************************************************************************
  qgs3dmapconfigwidget.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmapconfigwidget.h"
#include "moc_qgs3dmapconfigwidget.cpp"

#include "qgs3dmapsettings.h"
#include "qgsdemterrainsettings.h"
#include "qgsflatterrainsettings.h"
#include "qgsonlinedemterrainsettings.h"
#include "qgsmeshterrainsettings.h"
#include "qgs3dutils.h"
#include "qgsguiutils.h"
#include "qgsmapcanvas.h"
#include "qgsquantizedmeshterrainsettings.h"
#include "qgsrasterlayer.h"
#include "qgsmeshlayer.h"
#include "qgsproject.h"
#include "qgsmesh3dsymbolwidget.h"
#include "qgssettings.h"
#include "qgsskyboxrenderingsettingswidget.h"
#include "qgsshadowrenderingsettingswidget.h"
#include "qgsambientocclusionsettingswidget.h"
#include "qgs3dmapcanvas.h"
#include "qgsterraingenerator.h"
#include "qgstiledscenelayer.h"
#include "qgsabstractterrainsettings.h"

Qgs3DMapConfigWidget::Qgs3DMapConfigWidget( Qgs3DMapSettings *map, QgsMapCanvas *mainCanvas, Qgs3DMapCanvas *mapCanvas3D, QWidget *parent )
  : QWidget( parent )
  , mMap( map )
  , mMainCanvas( mainCanvas )
{
  Q_UNUSED( mapCanvas3D )
  setupUi( this );

  Q_ASSERT( map );

  const QgsSettings settings;

  const int iconSize = QgsGuiUtils::scaleIconSize( 20 );
  m3DOptionsListWidget->setIconSize( QSize( iconSize, iconSize ) );

  mCameraNavigationModeCombo->addItem( tr( "Terrain Based" ), QVariant::fromValue( Qgis::NavigationMode::TerrainBased ) );
  mCameraNavigationModeCombo->addItem( tr( "Walk Mode (First Person)" ), QVariant::fromValue( Qgis::NavigationMode::Walk ) );

  // get rid of annoying outer focus rect on Mac
  m3DOptionsListWidget->setAttribute( Qt::WA_MacShowFocusRect, false );
  m3DOptionsListWidget->setCurrentRow( settings.value( QStringLiteral( "Windows/3DMapConfig/Tab" ), 0 ).toInt() );
  connect( m3DOptionsListWidget, &QListWidget::currentRowChanged, this, [=]( int index ) { m3DOptionsStackedWidget->setCurrentIndex( index ); } );
  m3DOptionsStackedWidget->setCurrentIndex( m3DOptionsListWidget->currentRow() );

  if ( !settings.contains( QStringLiteral( "Windows/3DMapConfig/OptionsSplitState" ) ) )
  {
    // set left list widget width on initial showing
    QList<int> splitsizes;
    splitsizes << 115;
    m3DOptionsSplitter->setSizes( splitsizes );
  }
  m3DOptionsSplitter->restoreState( settings.value( QStringLiteral( "Windows/3DMapConfig/OptionsSplitState" ) ).toByteArray() );

  mMeshSymbolWidget = new QgsMesh3DSymbolWidget( nullptr, groupMeshTerrainShading );
  mMeshSymbolWidget->configureForTerrain();

  cboCameraProjectionType->addItem( tr( "Perspective Projection" ), Qt3DRender::QCameraLens::PerspectiveProjection );
  cboCameraProjectionType->addItem( tr( "Orthogonal Projection" ), Qt3DRender::QCameraLens::OrthographicProjection );
  connect( cboCameraProjectionType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [=]() {
    spinCameraFieldOfView->setEnabled( cboCameraProjectionType->currentIndex() == cboCameraProjectionType->findData( Qt3DRender::QCameraLens::PerspectiveProjection ) );
  } );

  mCameraMovementSpeed->setClearValue( 4 );
  spinCameraFieldOfView->setClearValue( 45.0 );
  spinTerrainScale->setClearValue( 1.0 );
  spinTerrainResolution->setClearValue( 16 );
  spinTerrainSkirtHeight->setClearValue( 10 );
  spinMapResolution->setClearValue( 512 );
  spinScreenError->setClearValue( 3 );
  spinGroundError->setClearValue( 1 );
  terrainElevationOffsetSpinBox->setClearValue( 0.0 );
  edlStrengthSpinBox->setClearValue( 1000 );
  edlDistanceSpinBox->setClearValue( 1 );

  cboTerrainLayer->setAllowEmptyLayer( true );
  cboTerrainLayer->setFilters( Qgis::LayerFilter::RasterLayer );

  cboTerrainType->addItem( tr( "Flat Terrain" ), QgsTerrainGenerator::Flat );
  cboTerrainType->addItem( tr( "DEM (Raster Layer)" ), QgsTerrainGenerator::Dem );
  cboTerrainType->addItem( tr( "Online" ), QgsTerrainGenerator::Online );
  cboTerrainType->addItem( tr( "Mesh" ), QgsTerrainGenerator::Mesh );
  cboTerrainType->addItem( tr( "Quantized Mesh" ), QgsTerrainGenerator::QuantizedMesh );

  groupTerrain->setChecked( mMap->terrainRenderingEnabled() );

  const QgsAbstractTerrainSettings *terrainSettings = mMap->terrainSettings();
  if ( terrainSettings )
  {
    // common properties
    terrainElevationOffsetSpinBox->setValue( terrainSettings->elevationOffset() );
    spinTerrainScale->setValue( terrainSettings->verticalScale() );
    spinMapResolution->setValue( terrainSettings->mapTileResolution() );
    spinScreenError->setValue( terrainSettings->maximumScreenError() );
    spinGroundError->setValue( terrainSettings->maximumGroundError() );
  }

  if ( terrainSettings && terrainSettings->type() == QLatin1String( "dem" ) )
  {
    cboTerrainType->setCurrentIndex( cboTerrainType->findData( QgsTerrainGenerator::Dem ) );
    const QgsDemTerrainSettings *demTerrainSettings = qgis::down_cast<const QgsDemTerrainSettings *>( terrainSettings );
    spinTerrainResolution->setValue( demTerrainSettings->resolution() );
    spinTerrainSkirtHeight->setValue( demTerrainSettings->skirtHeight() );
    cboTerrainLayer->setFilters( Qgis::LayerFilter::RasterLayer );
    cboTerrainLayer->setLayer( demTerrainSettings->layer() );
  }
  else if ( terrainSettings && terrainSettings->type() == QLatin1String( "online" ) )
  {
    cboTerrainType->setCurrentIndex( cboTerrainType->findData( QgsTerrainGenerator::Online ) );
    const QgsOnlineDemTerrainSettings *demTerrainSettings = qgis::down_cast<const QgsOnlineDemTerrainSettings *>( terrainSettings );
    spinTerrainResolution->setValue( demTerrainSettings->resolution() );
    spinTerrainSkirtHeight->setValue( demTerrainSettings->skirtHeight() );
  }
  else if ( terrainSettings && terrainSettings->type() == QLatin1String( "mesh" ) )
  {
    cboTerrainType->setCurrentIndex( cboTerrainType->findData( QgsTerrainGenerator::Mesh ) );
    const QgsMeshTerrainSettings *meshTerrainSettings = qgis::down_cast<const QgsMeshTerrainSettings *>( terrainSettings );
    cboTerrainLayer->setFilters( Qgis::LayerFilter::MeshLayer );
    cboTerrainLayer->setLayer( meshTerrainSettings->layer() );
    mMeshSymbolWidget->setLayer( meshTerrainSettings->layer(), false );
    mMeshSymbolWidget->setSymbol( meshTerrainSettings->symbol() );
    spinTerrainScale->setValue( meshTerrainSettings->symbol()->verticalScale() );
  }
  else if ( terrainSettings && terrainSettings->type() == QLatin1String( "quantizedmesh" ) )
  {
    cboTerrainType->setCurrentIndex( cboTerrainType->findData( QgsTerrainGenerator::QuantizedMesh ) );
    const QgsQuantizedMeshTerrainSettings *quantizedMeshTerrainSettings = qgis::down_cast<const QgsQuantizedMeshTerrainSettings *>( terrainSettings );
    cboTerrainLayer->setFilters( Qgis::LayerFilter::TiledSceneLayer );
    cboTerrainLayer->setLayer( quantizedMeshTerrainSettings->layer() );
  }
  else if ( terrainSettings && terrainSettings->type() == QLatin1String( "flat" ) )
  {
    cboTerrainType->setCurrentIndex( cboTerrainType->findData( QgsTerrainGenerator::Flat ) );
    cboTerrainLayer->setLayer( nullptr );
    spinTerrainResolution->setValue( 16 );
    spinTerrainSkirtHeight->setValue( 10 );
  }

  spinCameraFieldOfView->setValue( mMap->fieldOfView() );
  cboCameraProjectionType->setCurrentIndex( cboCameraProjectionType->findData( mMap->projectionType() ) );
  mCameraNavigationModeCombo->setCurrentIndex( mCameraNavigationModeCombo->findData( QVariant::fromValue( mMap->cameraNavigationMode() ) ) );
  mCameraMovementSpeed->setValue( mMap->cameraMovementSpeed() );

  chkShowLabels->setChecked( mMap->showLabels() );
  mFpsCounterCheckBox->setChecked( mMap->isFpsCounterEnabled() );
  chkShowDebugPanel->setChecked( mMap->showDebugPanel() );

  groupTerrainShading->setChecked( mMap->isTerrainShadingEnabled() );
  widgetTerrainMaterial->setTechnique( QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture );
  QgsPhongMaterialSettings terrainShadingMaterial = mMap->terrainShadingMaterial();
  widgetTerrainMaterial->setSettings( &terrainShadingMaterial, nullptr );

  widgetLights->setLights( mMap->lightSources() );

  connect( cboTerrainType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::onTerrainTypeChanged );
  connect( cboTerrainLayer, static_cast<void ( QComboBox::* )( int )>( &QgsMapLayerComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::onTerrainLayerChanged );
  connect( spinMapResolution, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &Qgs3DMapConfigWidget::updateMaxZoomLevel );
  connect( spinGroundError, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DMapConfigWidget::updateMaxZoomLevel );

  groupMeshTerrainShading->layout()->addWidget( mMeshSymbolWidget );

  // ==================
  // Page: Skybox
  mSkyboxSettingsWidget = new QgsSkyboxRenderingSettingsWidget( this );
  mSkyboxSettingsWidget->setSkyboxSettings( map->skyboxSettings() );
  groupSkyboxSettings->layout()->addWidget( mSkyboxSettingsWidget );
  groupSkyboxSettings->setChecked( mMap->isSkyboxEnabled() );

  // ==================
  // Page: Shadows
  mShadowSettingsWidget = new QgsShadowRenderingSettingsWidget( this );
  mShadowSettingsWidget->onDirectionalLightsCountChanged( widgetLights->directionalLightCount() );
  mShadowSettingsWidget->setShadowSettings( map->shadowSettings() );
  groupShadowRendering->layout()->addWidget( mShadowSettingsWidget );
  connect( widgetLights, &QgsLightsWidget::directionalLightsCountChanged, mShadowSettingsWidget, &QgsShadowRenderingSettingsWidget::onDirectionalLightsCountChanged );

  connect( widgetLights, &QgsLightsWidget::lightsAdded, this, &Qgs3DMapConfigWidget::validate );
  connect( widgetLights, &QgsLightsWidget::lightsRemoved, this, &Qgs3DMapConfigWidget::validate );

  groupShadowRendering->setChecked( map->shadowSettings().renderShadows() );

  // ==================
  // Page: 3D axis
  mCbo3dAxisType->addItem( tr( "Coordinate Reference System" ), static_cast<int>( Qgs3DAxisSettings::Mode::Crs ) );
  mCbo3dAxisType->addItem( tr( "Cube" ), static_cast<int>( Qgs3DAxisSettings::Mode::Cube ) );

  mCbo3dAxisHorizPos->addItem( tr( "Left" ), static_cast<int>( Qt::AnchorPoint::AnchorLeft ) );
  mCbo3dAxisHorizPos->addItem( tr( "Center" ), static_cast<int>( Qt::AnchorPoint::AnchorHorizontalCenter ) );
  mCbo3dAxisHorizPos->addItem( tr( "Right" ), static_cast<int>( Qt::AnchorPoint::AnchorRight ) );

  mCbo3dAxisVertPos->addItem( tr( "Top" ), static_cast<int>( Qt::AnchorPoint::AnchorTop ) );
  mCbo3dAxisVertPos->addItem( tr( "Middle" ), static_cast<int>( Qt::AnchorPoint::AnchorVerticalCenter ) );
  mCbo3dAxisVertPos->addItem( tr( "Bottom" ), static_cast<int>( Qt::AnchorPoint::AnchorBottom ) );

  init3DAxisPage();

  // ==================
  // Page: 2D/3D canvas sync
  mSync2DTo3DCheckbox->setChecked( map->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync2DTo3D ) );
  mSync3DTo2DCheckbox->setChecked( map->viewSyncMode().testFlag( Qgis::ViewSyncModeFlag::Sync3DTo2D ) );
  mVisualizeExtentCheckBox->setChecked( map->viewFrustumVisualizationEnabled() );

  // ==================
  // Page: Advanced

  // EyeDomeLight
  edlGroupBox->setChecked( map->eyeDomeLightingEnabled() );
  edlStrengthSpinBox->setValue( map->eyeDomeLightingStrength() );
  edlDistanceSpinBox->setValue( map->eyeDomeLightingDistance() );

  // Ambient occlusion
  mAmbientOcclusionSettingsWidget->setAmbientOcclusionSettings( map->ambientOcclusionSettings() );

  // ==================
  // Page: General

  groupExtent->setOutputCrs( mMap->crs() );
  groupExtent->setCurrentExtent( mMap->extent(), mMap->crs() );
  groupExtent->setOutputExtentFromCurrent();
  if ( mMainCanvas )
  {
    groupExtent->setMapCanvas( mMainCanvas );
  }

  // checkbox to display the extent in the 2D Map View
  mShowExtentIn2DViewCheckbox = new QCheckBox( tr( "Show in 2D map view" ) );
  mShowExtentIn2DViewCheckbox->setChecked( map->showExtentIn2DView() );
  groupExtent->layout()->addWidget( mShowExtentIn2DViewCheckbox );

  onTerrainTypeChanged();
}

Qgs3DMapConfigWidget::~Qgs3DMapConfigWidget()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/3DMapConfig/OptionsSplitState" ), m3DOptionsSplitter->saveState() );
  settings.setValue( QStringLiteral( "Windows/3DMapConfig/Tab" ), m3DOptionsListWidget->currentRow() );
}

void Qgs3DMapConfigWidget::apply()
{
  mMap->setExtent( groupExtent->outputExtent() );
  mMap->setShowExtentIn2DView( mShowExtentIn2DViewCheckbox->isChecked() );

  const QgsTerrainGenerator::Type terrainType = static_cast<QgsTerrainGenerator::Type>( cboTerrainType->currentData().toInt() );

  mMap->setTerrainRenderingEnabled( groupTerrain->isChecked() );
  std::unique_ptr<QgsAbstractTerrainSettings> terrainSettings;
  switch ( terrainType )
  {
    case QgsTerrainGenerator::Flat:
    {
      terrainSettings = std::make_unique<QgsFlatTerrainSettings>();
      break;
    }

    case QgsTerrainGenerator::Dem:
    {
      std::unique_ptr<QgsDemTerrainSettings> demTerrainSettings = std::make_unique<QgsDemTerrainSettings>();
      demTerrainSettings->setLayer( qobject_cast<QgsRasterLayer *>( cboTerrainLayer->currentLayer() ) );
      demTerrainSettings->setResolution( spinTerrainResolution->value() );
      demTerrainSettings->setSkirtHeight( spinTerrainSkirtHeight->value() );
      terrainSettings = std::move( demTerrainSettings );
      break;
    }

    case QgsTerrainGenerator::Online:
    {
      std::unique_ptr<QgsOnlineDemTerrainSettings> onlineTerrainSettings = std::make_unique<QgsOnlineDemTerrainSettings>();
      onlineTerrainSettings->setResolution( spinTerrainResolution->value() );
      onlineTerrainSettings->setSkirtHeight( spinTerrainSkirtHeight->value() );
      terrainSettings = std::move( onlineTerrainSettings );
      break;
    }

    case QgsTerrainGenerator::Mesh:
    {
      std::unique_ptr<QgsMeshTerrainSettings> meshTerrainSettings = std::make_unique<QgsMeshTerrainSettings>();
      meshTerrainSettings->setLayer( qobject_cast<QgsMeshLayer *>( cboTerrainLayer->currentLayer() ) );

      std::unique_ptr<QgsMesh3DSymbol> symbol = mMeshSymbolWidget->symbol();
      symbol->setVerticalScale( spinTerrainScale->value() );
      meshTerrainSettings->setSymbol( symbol.release() );

      terrainSettings = std::move( meshTerrainSettings );
      break;
    }

    case QgsTerrainGenerator::QuantizedMesh:
    {
      std::unique_ptr<QgsQuantizedMeshTerrainSettings> meshTerrainSettings = std::make_unique<QgsQuantizedMeshTerrainSettings>();
      meshTerrainSettings->setLayer( qobject_cast<QgsTiledSceneLayer *>( cboTerrainLayer->currentLayer() ) );

      terrainSettings = std::move( meshTerrainSettings );
      break;
    }
  }

  if ( terrainSettings )
  {
    // set common terrain settings
    terrainSettings->setVerticalScale( spinTerrainScale->value() );
    terrainSettings->setMapTileResolution( spinMapResolution->value() );
    terrainSettings->setMaximumScreenError( spinScreenError->value() );
    terrainSettings->setMaximumGroundError( spinGroundError->value() );
    terrainSettings->setElevationOffset( terrainElevationOffsetSpinBox->value() );
    mMap->setTerrainSettings( terrainSettings.release() );
  }

  mMap->setFieldOfView( spinCameraFieldOfView->value() );
  mMap->setProjectionType( cboCameraProjectionType->currentData().value<Qt3DRender::QCameraLens::ProjectionType>() );
  mMap->setCameraNavigationMode( mCameraNavigationModeCombo->currentData().value<Qgis::NavigationMode>() );
  mMap->setCameraMovementSpeed( mCameraMovementSpeed->value() );
  mMap->setShowLabels( chkShowLabels->isChecked() );
  mMap->setIsFpsCounterEnabled( mFpsCounterCheckBox->isChecked() );
  mMap->setShowDebugPanel( chkShowDebugPanel->isChecked() );
  mMap->setTerrainShadingEnabled( groupTerrainShading->isChecked() );

  const std::unique_ptr<QgsAbstractMaterialSettings> terrainMaterial( widgetTerrainMaterial->settings() );
  if ( QgsPhongMaterialSettings *phongMaterial = dynamic_cast<QgsPhongMaterialSettings *>( terrainMaterial.get() ) )
    mMap->setTerrainShadingMaterial( *phongMaterial );

  mMap->setLightSources( widgetLights->lightSources() );
  mMap->setIsSkyboxEnabled( groupSkyboxSettings->isChecked() );
  mMap->setSkyboxSettings( mSkyboxSettingsWidget->toSkyboxSettings() );
  QgsShadowSettings shadowSettings = mShadowSettingsWidget->toShadowSettings();
  shadowSettings.setRenderShadows( groupShadowRendering->isChecked() );
  mMap->setShadowSettings( shadowSettings );

  mMap->setEyeDomeLightingEnabled( edlGroupBox->isChecked() );
  mMap->setEyeDomeLightingStrength( edlStrengthSpinBox->value() );
  mMap->setEyeDomeLightingDistance( edlDistanceSpinBox->value() );

  mMap->setAmbientOcclusionSettings( mAmbientOcclusionSettingsWidget->toAmbientOcclusionSettings() );

  Qgis::ViewSyncModeFlags viewSyncMode;
  viewSyncMode.setFlag( Qgis::ViewSyncModeFlag::Sync2DTo3D, mSync2DTo3DCheckbox->isChecked() );
  viewSyncMode.setFlag( Qgis::ViewSyncModeFlag::Sync3DTo2D, mSync3DTo2DCheckbox->isChecked() );
  mMap->setViewSyncMode( viewSyncMode );
  mMap->setViewFrustumVisualizationEnabled( mVisualizeExtentCheckBox->isChecked() );
}

void Qgs3DMapConfigWidget::onTerrainTypeChanged()
{
  const QgsTerrainGenerator::Type genType = static_cast<QgsTerrainGenerator::Type>( cboTerrainType->currentData().toInt() );

  labelTerrainResolution->setVisible( !( genType == QgsTerrainGenerator::Flat || genType == QgsTerrainGenerator::Mesh || genType == QgsTerrainGenerator::QuantizedMesh ) );
  spinTerrainResolution->setVisible( !( genType == QgsTerrainGenerator::Flat || genType == QgsTerrainGenerator::Mesh || genType == QgsTerrainGenerator::QuantizedMesh ) );
  labelTerrainScale->setVisible( !( genType == QgsTerrainGenerator::Flat || genType == QgsTerrainGenerator::QuantizedMesh ) );
  spinTerrainScale->setVisible( !( genType == QgsTerrainGenerator::Flat || genType == QgsTerrainGenerator::QuantizedMesh ) );
  terrainElevationOffsetSpinBox->setVisible( genType != QgsTerrainGenerator::QuantizedMesh );
  labelterrainElevationOffset->setVisible( genType != QgsTerrainGenerator::QuantizedMesh );
  labelTerrainSkirtHeight->setVisible( !( genType == QgsTerrainGenerator::Flat || genType == QgsTerrainGenerator::Mesh || genType == QgsTerrainGenerator::QuantizedMesh ) );
  spinTerrainSkirtHeight->setVisible( !( genType == QgsTerrainGenerator::Flat || genType == QgsTerrainGenerator::Mesh || genType == QgsTerrainGenerator::QuantizedMesh ) );
  labelTerrainLayer->setVisible( genType == QgsTerrainGenerator::Dem || genType == QgsTerrainGenerator::Mesh || genType == QgsTerrainGenerator::QuantizedMesh );
  cboTerrainLayer->setVisible( genType == QgsTerrainGenerator::Dem || genType == QgsTerrainGenerator::Mesh || genType == QgsTerrainGenerator::QuantizedMesh );
  groupMeshTerrainShading->setVisible( genType == QgsTerrainGenerator::Mesh );
  groupTerrainShading->setVisible( genType != QgsTerrainGenerator::Mesh );

  QgsMapLayer *oldTerrainLayer = cboTerrainLayer->currentLayer();
  if ( cboTerrainType->currentData() == QgsTerrainGenerator::Dem )
  {
    cboTerrainLayer->setFilters( Qgis::LayerFilter::RasterLayer );
  }
  else if ( cboTerrainType->currentData() == QgsTerrainGenerator::Mesh )
  {
    cboTerrainLayer->setFilters( Qgis::LayerFilter::MeshLayer );
  }
  else if ( cboTerrainType->currentData() == QgsTerrainGenerator::QuantizedMesh )
  {
    cboTerrainLayer->setFilters( Qgis::LayerFilter::TiledSceneLayer );
  }

  if ( cboTerrainLayer->currentLayer() != oldTerrainLayer )
    onTerrainLayerChanged();

  updateMaxZoomLevel();
  validate();
}

void Qgs3DMapConfigWidget::onTerrainLayerChanged()
{
  updateMaxZoomLevel();

  if ( cboTerrainType->currentData() == QgsTerrainGenerator::Mesh )
  {
    QgsMeshLayer *meshLayer = qobject_cast<QgsMeshLayer *>( cboTerrainLayer->currentLayer() );
    if ( meshLayer )
    {
      QgsMeshLayer *oldLayer = mMeshSymbolWidget->meshLayer();

      mMeshSymbolWidget->setLayer( meshLayer, false );
      if ( oldLayer != meshLayer )
        mMeshSymbolWidget->reloadColorRampShaderMinMax();
    }
  }
  validate();
}

void Qgs3DMapConfigWidget::updateMaxZoomLevel()
{
  const QgsRectangle te = groupExtent->outputExtent();

  const double tile0width = std::max( te.width(), te.height() );
  const int zoomLevel = Qgs3DUtils::maxZoomLevel( tile0width, spinMapResolution->value(), spinGroundError->value() );
  labelZoomLevels->setText( QStringLiteral( "0 - %1" ).arg( zoomLevel ) );
}

void Qgs3DMapConfigWidget::validate()
{
  mMessageBar->clearWidgets();

  bool valid = true;
  switch ( static_cast<QgsTerrainGenerator::Type>( cboTerrainType->currentData().toInt() ) )
  {
    case QgsTerrainGenerator::Dem:
      if ( !cboTerrainLayer->currentLayer() )
      {
        valid = false;
        mMessageBar->pushMessage( tr( "An elevation layer must be selected for a DEM terrain" ), Qgis::MessageLevel::Critical );
      }
      break;

    case QgsTerrainGenerator::Mesh:
      if ( !cboTerrainLayer->currentLayer() )
      {
        valid = false;
        mMessageBar->pushMessage( tr( "An elevation layer must be selected for a mesh terrain" ), Qgis::MessageLevel::Critical );
      }
      break;

    case QgsTerrainGenerator::QuantizedMesh:
      if ( !cboTerrainLayer->currentLayer() )
      {
        valid = false;
        mMessageBar->pushMessage( tr( "An elevation layer must be selected for a quantized mesh terrain" ), Qgis::MessageLevel::Critical );
      }
      break;

    case QgsTerrainGenerator::Online:
    case QgsTerrainGenerator::Flat:
      break;
  }

  if ( valid && widgetLights->lightSourceCount() == 0 )
  {
    mMessageBar->pushMessage( tr( "No lights exist in the scene" ), Qgis::MessageLevel::Warning );
  }

  emit isValidChanged( valid );
}

void Qgs3DMapConfigWidget::init3DAxisPage()
{
  connect( mGroupBox3dAxis, &QGroupBox::toggled, this, &Qgs3DMapConfigWidget::on3DAxisChanged );
  connect( mCbo3dAxisType, qOverload<int>( &QComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::on3DAxisChanged );
  connect( mCbo3dAxisHorizPos, qOverload<int>( &QComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::on3DAxisChanged );
  connect( mCbo3dAxisVertPos, qOverload<int>( &QComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::on3DAxisChanged );

  Qgs3DAxisSettings s = mMap->get3DAxisSettings();

  if ( s.mode() == Qgs3DAxisSettings::Mode::Off )
    mGroupBox3dAxis->setChecked( false );
  else
  {
    mGroupBox3dAxis->setChecked( true );
    mCbo3dAxisType->setCurrentIndex( mCbo3dAxisType->findData( static_cast<int>( s.mode() ) ) );
  }

  mCbo3dAxisHorizPos->setCurrentIndex( mCbo3dAxisHorizPos->findData( static_cast<int>( s.horizontalPosition() ) ) );
  mCbo3dAxisVertPos->setCurrentIndex( mCbo3dAxisVertPos->findData( static_cast<int>( s.verticalPosition() ) ) );
}

void Qgs3DMapConfigWidget::on3DAxisChanged()
{
  Qgs3DAxisSettings s = mMap->get3DAxisSettings();
  Qgs3DAxisSettings::Mode m;

  if ( mGroupBox3dAxis->isChecked() )
    m = static_cast<Qgs3DAxisSettings::Mode>( mCbo3dAxisType->currentData().toInt() );
  else
    m = Qgs3DAxisSettings::Mode::Off;

  if ( s.mode() != m )
  {
    s.setMode( m );
  }
  else
  {
    const Qt::AnchorPoint hPos = static_cast<Qt::AnchorPoint>( mCbo3dAxisHorizPos->currentData().toInt() );
    const Qt::AnchorPoint vPos = static_cast<Qt::AnchorPoint>( mCbo3dAxisVertPos->currentData().toInt() );

    if ( s.horizontalPosition() != hPos || s.verticalPosition() != vPos )
    {
      s.setHorizontalPosition( hPos );
      s.setVerticalPosition( vPos );
    }
  }

  mMap->set3DAxisSettings( s );
}
