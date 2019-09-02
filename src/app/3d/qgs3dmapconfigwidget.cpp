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

#include "qgs3dmapsettings.h"
#include "qgsdemterraingenerator.h"
#include "qgsflatterraingenerator.h"
#include "qgsonlineterraingenerator.h"
#include "qgs3dutils.h"

#include "qgsmapcanvas.h"
#include "qgsmapthemecollection.h"
#include "qgsrasterlayer.h"
#include "qgsproject.h"

Qgs3DMapConfigWidget::Qgs3DMapConfigWidget( Qgs3DMapSettings *map, QgsMapCanvas *mainCanvas, QWidget *parent )
  : QWidget( parent )
  , mMap( map )
  , mMainCanvas( mainCanvas )
{
  setupUi( this );

  Q_ASSERT( map );
  Q_ASSERT( mainCanvas );

  spinCameraFieldOfView->setClearValue( 45.0 );
  spinTerrainScale->setClearValue( 1.0 );
  spinTerrainResolution->setClearValue( 16 );
  spinTerrainSkirtHeight->setClearValue( 10 );
  spinMapResolution->setClearValue( 512 );
  spinScreenError->setClearValue( 3 );
  spinGroundError->setClearValue( 1 );

  cboTerrainLayer->setAllowEmptyLayer( true );
  cboTerrainLayer->setFilters( QgsMapLayerProxyModel::RasterLayer );

  cboTerrainType->addItem( tr( "Flat terrain" ), QgsTerrainGenerator::Flat );
  cboTerrainType->addItem( tr( "DEM (Raster layer)" ), QgsTerrainGenerator::Dem );
  cboTerrainType->addItem( tr( "Online" ), QgsTerrainGenerator::Online );

  QgsTerrainGenerator *terrainGen = mMap->terrainGenerator();
  if ( terrainGen && terrainGen->type() == QgsTerrainGenerator::Dem )
  {
    cboTerrainType->setCurrentIndex( cboTerrainType->findData( QgsTerrainGenerator::Dem ) );
    QgsDemTerrainGenerator *demTerrainGen = static_cast<QgsDemTerrainGenerator *>( terrainGen );
    spinTerrainResolution->setValue( demTerrainGen->resolution() );
    spinTerrainSkirtHeight->setValue( demTerrainGen->skirtHeight() );
    cboTerrainLayer->setLayer( demTerrainGen->layer() );
  }
  else if ( terrainGen && terrainGen->type() == QgsTerrainGenerator::Online )
  {
    cboTerrainType->setCurrentIndex( cboTerrainType->findData( QgsTerrainGenerator::Online ) );
    QgsOnlineTerrainGenerator *onlineTerrainGen = static_cast<QgsOnlineTerrainGenerator *>( terrainGen );
    spinTerrainResolution->setValue( onlineTerrainGen->resolution() );
    spinTerrainSkirtHeight->setValue( onlineTerrainGen->skirtHeight() );
  }
  else
  {
    cboTerrainType->setCurrentIndex( cboTerrainType->findData( QgsTerrainGenerator::Flat ) );
    cboTerrainLayer->setLayer( nullptr );
    spinTerrainResolution->setValue( 16 );
    spinTerrainSkirtHeight->setValue( 10 );
  }

  spinCameraFieldOfView->setValue( mMap->fieldOfView() );
  spinTerrainScale->setValue( mMap->terrainVerticalScale() );
  spinMapResolution->setValue( mMap->mapTileResolution() );
  spinScreenError->setValue( mMap->maxTerrainScreenError() );
  spinGroundError->setValue( mMap->maxTerrainGroundError() );
  chkShowLabels->setChecked( mMap->showLabels() );
  chkShowTileInfo->setChecked( mMap->showTerrainTilesInfo() );
  chkShowBoundingBoxes->setChecked( mMap->showTerrainBoundingBoxes() );
  chkShowCameraViewCenter->setChecked( mMap->showCameraViewCenter() );

  groupTerrainShading->setChecked( mMap->isTerrainShadingEnabled() );
  widgetTerrainMaterial->setDiffuseVisible( false );
  widgetTerrainMaterial->setMaterial( mMap->terrainShadingMaterial() );

  // populate combo box with map themes
  const QStringList mapThemeNames = QgsProject::instance()->mapThemeCollection()->mapThemes();
  cboTerrainMapTheme->addItem( tr( "(none)" ) );  // item for no map theme
  for ( QString themeName : mapThemeNames )
    cboTerrainMapTheme->addItem( themeName );

  cboTerrainMapTheme->setCurrentText( mMap->terrainMapTheme() );

  widgetLights->setPointLights( mMap->pointLights() );

  connect( cboTerrainType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::onTerrainTypeChanged );
  connect( cboTerrainLayer, static_cast<void ( QComboBox::* )( int )>( &QgsMapLayerComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::onTerrainLayerChanged );
  connect( spinMapResolution, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &Qgs3DMapConfigWidget::updateMaxZoomLevel );
  connect( spinGroundError, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DMapConfigWidget::updateMaxZoomLevel );

  onTerrainTypeChanged();
}

void Qgs3DMapConfigWidget::apply()
{
  bool needsUpdateOrigin = false;

  QgsTerrainGenerator::Type terrainType = static_cast<QgsTerrainGenerator::Type>( cboTerrainType->currentData().toInt() );

  if ( terrainType == QgsTerrainGenerator::Dem )  // DEM from raster layer
  {
    QgsRasterLayer *demLayer = qobject_cast<QgsRasterLayer *>( cboTerrainLayer->currentLayer() );

    bool tGenNeedsUpdate = true;
    if ( mMap->terrainGenerator()->type() == QgsTerrainGenerator::Dem )
    {
      // if we already have a DEM terrain generator, check whether there was actually any change
      QgsDemTerrainGenerator *oldDemTerrainGen = static_cast<QgsDemTerrainGenerator *>( mMap->terrainGenerator() );
      if ( oldDemTerrainGen->layer() == demLayer &&
           oldDemTerrainGen->resolution() == spinTerrainResolution->value() &&
           oldDemTerrainGen->skirtHeight() == spinTerrainSkirtHeight->value() )
        tGenNeedsUpdate = false;
    }

    if ( tGenNeedsUpdate )
    {
      QgsDemTerrainGenerator *demTerrainGen = new QgsDemTerrainGenerator;
      demTerrainGen->setCrs( mMap->crs(), QgsProject::instance()->transformContext() );
      demTerrainGen->setLayer( demLayer );
      demTerrainGen->setResolution( spinTerrainResolution->value() );
      demTerrainGen->setSkirtHeight( spinTerrainSkirtHeight->value() );
      mMap->setTerrainGenerator( demTerrainGen );
      needsUpdateOrigin = true;
    }
  }
  else if ( terrainType == QgsTerrainGenerator::Online )
  {
    bool tGenNeedsUpdate = true;
    if ( mMap->terrainGenerator()->type() == QgsTerrainGenerator::Online )
    {
      QgsOnlineTerrainGenerator *oldOnlineTerrainGen = static_cast<QgsOnlineTerrainGenerator *>( mMap->terrainGenerator() );
      if ( oldOnlineTerrainGen->resolution() == spinTerrainResolution->value() &&
           oldOnlineTerrainGen->skirtHeight() == spinTerrainSkirtHeight->value() )
        tGenNeedsUpdate = false;
    }

    if ( tGenNeedsUpdate )
    {
      QgsOnlineTerrainGenerator *onlineTerrainGen = new QgsOnlineTerrainGenerator;
      onlineTerrainGen->setCrs( mMap->crs(), QgsProject::instance()->transformContext() );
      onlineTerrainGen->setExtent( mMainCanvas->fullExtent() );
      onlineTerrainGen->setResolution( spinTerrainResolution->value() );
      onlineTerrainGen->setSkirtHeight( spinTerrainSkirtHeight->value() );
      mMap->setTerrainGenerator( onlineTerrainGen );
      needsUpdateOrigin = true;
    }
  }
  else if ( terrainType == QgsTerrainGenerator::Flat )
  {
    QgsFlatTerrainGenerator *flatTerrainGen = new QgsFlatTerrainGenerator;
    flatTerrainGen->setCrs( mMap->crs() );
    flatTerrainGen->setExtent( mMainCanvas->fullExtent() );
    mMap->setTerrainGenerator( flatTerrainGen );
    needsUpdateOrigin = true;
  }

  if ( needsUpdateOrigin )
  {
    // reproject terrain's extent to map CRS
    QgsRectangle te = mMap->terrainGenerator()->extent();
    QgsCoordinateTransform terrainToMapTransform( mMap->terrainGenerator()->crs(), mMap->crs(), QgsProject::instance() );
    te = terrainToMapTransform.transformBoundingBox( te );

    QgsPointXY center = te.center();
    mMap->setOrigin( QgsVector3D( center.x(), center.y(), 0 ) );
  }

  mMap->setFieldOfView( spinCameraFieldOfView->value() );
  mMap->setTerrainVerticalScale( spinTerrainScale->value() );
  mMap->setMapTileResolution( spinMapResolution->value() );
  mMap->setMaxTerrainScreenError( spinScreenError->value() );
  mMap->setMaxTerrainGroundError( spinGroundError->value() );
  mMap->setShowLabels( chkShowLabels->isChecked() );
  mMap->setShowTerrainTilesInfo( chkShowTileInfo->isChecked() );
  mMap->setShowTerrainBoundingBoxes( chkShowBoundingBoxes->isChecked() );
  mMap->setShowCameraViewCenter( chkShowCameraViewCenter->isChecked() );

  mMap->setTerrainShadingEnabled( groupTerrainShading->isChecked() );
  mMap->setTerrainShadingMaterial( widgetTerrainMaterial->material() );

  mMap->setTerrainMapTheme( cboTerrainMapTheme->currentText() );

  mMap->setPointLights( widgetLights->pointLights() );
}

void Qgs3DMapConfigWidget::onTerrainTypeChanged()
{
  bool isFlat = cboTerrainType->currentIndex() == 0;
  bool isDem = cboTerrainType->currentIndex() == 1;
  labelTerrainResolution->setVisible( !isFlat );
  spinTerrainResolution->setVisible( !isFlat );
  labelTerrainSkirtHeight->setVisible( !isFlat );
  spinTerrainSkirtHeight->setVisible( !isFlat );
  labelTerrainLayer->setVisible( isDem );
  cboTerrainLayer->setVisible( isDem );

  updateMaxZoomLevel();
}

void Qgs3DMapConfigWidget::onTerrainLayerChanged()
{
  updateMaxZoomLevel();
}

void Qgs3DMapConfigWidget::updateMaxZoomLevel()
{
  QgsRectangle te;
  QgsTerrainGenerator::Type terrainType = static_cast<QgsTerrainGenerator::Type>( cboTerrainType->currentData().toInt() );
  if ( terrainType == QgsTerrainGenerator::Dem )
  {
    if ( QgsRasterLayer *demLayer = qobject_cast<QgsRasterLayer *>( cboTerrainLayer->currentLayer() ) )
    {
      te = demLayer->extent();
      QgsCoordinateTransform terrainToMapTransform( demLayer->crs(), mMap->crs(), QgsProject::instance()->transformContext() );
      te = terrainToMapTransform.transformBoundingBox( te );
    }
  }
  else  // flat or online
  {
    te = mMainCanvas->fullExtent();
  }

  double tile0width = std::max( te.width(), te.height() );
  int zoomLevel = Qgs3DUtils::maxZoomLevel( tile0width, spinMapResolution->value(), spinGroundError->value() );
  labelZoomLevels->setText( QStringLiteral( "0 - %1" ).arg( zoomLevel ) );
}
