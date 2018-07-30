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
#include "qgs3dutils.h"

#include "qgsmapcanvas.h"
#include "qgsrasterlayer.h"
//#include "qgsproject.h"

Qgs3DMapConfigWidget::Qgs3DMapConfigWidget( Qgs3DMapSettings *map, QgsMapCanvas *mainCanvas, QWidget *parent )
  : QWidget( parent )
  , mMap( map )
  , mMainCanvas( mainCanvas )
{
  setupUi( this );

  Q_ASSERT( map );
  Q_ASSERT( mainCanvas );

  spinTerrainScale->setClearValue( 1.0 );
  spinTerrainResolution->setClearValue( 16 );
  spinTerrainSkirtHeight->setClearValue( 10 );
  spinMapResolution->setClearValue( 512 );
  spinScreenError->setClearValue( 3 );
  spinGroundError->setClearValue( 1 );

  cboTerrainLayer->setAllowEmptyLayer( true );
  cboTerrainLayer->setFilters( QgsMapLayerProxyModel::RasterLayer );

  QgsTerrainGenerator *terrainGen = mMap->terrainGenerator();
  if ( terrainGen && terrainGen->type() == QgsTerrainGenerator::Dem )
  {
    QgsDemTerrainGenerator *demTerrainGen = static_cast<QgsDemTerrainGenerator *>( terrainGen );
    spinTerrainResolution->setValue( demTerrainGen->resolution() );
    spinTerrainSkirtHeight->setValue( demTerrainGen->skirtHeight() );
    cboTerrainLayer->setLayer( demTerrainGen->layer() );
  }
  else
  {
    cboTerrainLayer->setLayer( nullptr );
    spinTerrainResolution->setEnabled( false );
    spinTerrainResolution->setValue( 16 );
    spinTerrainSkirtHeight->setValue( 10 );
  }

  spinTerrainScale->setValue( mMap->terrainVerticalScale() );
  spinMapResolution->setValue( mMap->mapTileResolution() );
  spinScreenError->setValue( mMap->maxTerrainScreenError() );
  spinGroundError->setValue( mMap->maxTerrainGroundError() );
  chkShowLabels->setChecked( mMap->showLabels() );
  chkShowTileInfo->setChecked( mMap->showTerrainTilesInfo() );
  chkShowBoundingBoxes->setChecked( mMap->showTerrainBoundingBoxes() );
  chkShowCameraViewCenter->setChecked( mMap->showCameraViewCenter() );

  connect( cboTerrainLayer, static_cast<void ( QComboBox::* )( int )>( &QgsMapLayerComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::onTerrainLayerChanged );
  connect( spinMapResolution, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &Qgs3DMapConfigWidget::updateMaxZoomLevel );
  connect( spinGroundError, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DMapConfigWidget::updateMaxZoomLevel );

  updateMaxZoomLevel();
}

void Qgs3DMapConfigWidget::apply()
{
  QgsRasterLayer *demLayer = qobject_cast<QgsRasterLayer *>( cboTerrainLayer->currentLayer() );

  bool needsUpdateOrigin = false;

  if ( demLayer )
  {
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
  else if ( !demLayer && mMap->terrainGenerator()->type() != QgsTerrainGenerator::Flat )
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

  mMap->setTerrainVerticalScale( spinTerrainScale->value() );
  mMap->setMapTileResolution( spinMapResolution->value() );
  mMap->setMaxTerrainScreenError( spinScreenError->value() );
  mMap->setMaxTerrainGroundError( spinGroundError->value() );
  mMap->setShowLabels( chkShowLabels->isChecked() );
  mMap->setShowTerrainTilesInfo( chkShowTileInfo->isChecked() );
  mMap->setShowTerrainBoundingBoxes( chkShowBoundingBoxes->isChecked() );
  mMap->setShowCameraViewCenter( chkShowCameraViewCenter->isChecked() );
}

void Qgs3DMapConfigWidget::onTerrainLayerChanged()
{
  spinTerrainResolution->setEnabled( cboTerrainLayer->currentLayer() );
}

void Qgs3DMapConfigWidget::updateMaxZoomLevel()
{
  // TODO: tidy up, less duplication with apply()
  std::unique_ptr<QgsTerrainGenerator> tGen;
  QgsRasterLayer *demLayer = qobject_cast<QgsRasterLayer *>( cboTerrainLayer->currentLayer() );
  if ( demLayer )
  {
    QgsDemTerrainGenerator *demTerrainGen = new QgsDemTerrainGenerator;
    demTerrainGen->setCrs( mMap->crs(), QgsProject::instance()->transformContext() );
    demTerrainGen->setLayer( demLayer );
    demTerrainGen->setResolution( spinTerrainResolution->value() );
    tGen.reset( demTerrainGen );
  }
  else
  {
    QgsFlatTerrainGenerator *flatTerrainGen = new QgsFlatTerrainGenerator;
    flatTerrainGen->setCrs( mMap->crs() );
    flatTerrainGen->setExtent( mMainCanvas->fullExtent() );
    tGen.reset( flatTerrainGen );
  }

  double tile0width = tGen->extent().width();
  int zoomLevel = Qgs3DUtils::maxZoomLevel( tile0width, spinMapResolution->value(), spinGroundError->value() );
  labelZoomLevels->setText( QString( "0 - %1" ).arg( zoomLevel ) );
}
