/***************************************************************************
                         qgsrastervectorfieldrendererwidget.cpp
                         --------------------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastervectorfieldrendererwidget.h"

#include <algorithm>
#include <cmath>

#include "qgsrasterbandstats.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrastervectorfieldrenderer.h"

#include "moc_qgsrastervectorfieldrendererwidget.cpp"

QgsRasterVectorFieldRendererWidget::QgsRasterVectorFieldRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
  : QgsRasterRendererWidget( layer, extent )
{
  setupUi( this );

  mXBandComboBox->setLayer( mRasterLayer );
  mYBandComboBox->setLayer( mRasterLayer );
  mDirectionBandComboBox->setLayer( mRasterLayer );

  mSourceModeComboBox->addItem( tr( "Component Bands" ), QVariant::fromValue( Qgis::RasterVectorFieldSourceMode::CartesianComponents ) );
  mSourceModeComboBox->addItem( tr( "Encoded Direction" ), QVariant::fromValue( Qgis::RasterVectorFieldSourceMode::EncodedDirection ) );

  mDirectionEncodingComboBox->addItem( tr( "Esri (1 = E, 2 = SE, 4 = S, 8 = SW, 16 = W, 32 = NW, 64 = N, 128 = NE)" ), QVariant::fromValue( Qgis::RasterDirectionEncoding::Esri ) );
  mDirectionEncodingComboBox->addItem( tr( "GRASS (1 = NE, 2 = N, 3 = NW, 4 = W, 5 = SW, 6 = S, 7 = SE, 8 = E)" ), QVariant::fromValue( Qgis::RasterDirectionEncoding::Grass ) );
  mDirectionEncodingComboBox->addItem( tr( "SAGA (0 = N, 1 = NE, 2 = E, 3 = SE, 4 = S, 5 = SW, 6 = W, 7 = NW)" ), QVariant::fromValue( Qgis::RasterDirectionEncoding::Saga ) );
  mDirectionEncodingComboBox->addItem( tr( "PCRaster LDD (1 = SW, 2 = S, 3 = SE, 4 = W, 6 = E, 7 = NW, 8 = N, 9 = NE)" ), QVariant::fromValue( Qgis::RasterDirectionEncoding::PcRaster ) );

  const int bandCount = mRasterLayer && mRasterLayer->dataProvider() ? mRasterLayer->dataProvider()->bandCount() : 0;

  // a renderer which was never set up starts on the defaults of a freshly created one
  const QgsRasterVectorFieldRenderer defaultRenderer( nullptr );
  QgsVectorFieldSettings settings = defaultRenderer.settings();
  Qgis::RasterVectorFieldSourceMode sourceMode = defaultRenderer.sourceMode();
  Qgis::RasterDirectionEncoding encoding = defaultRenderer.directionEncoding();
  int xBand = 1;
  int yBand = std::min( 2, std::max( 1, bandCount ) );
  int directionBand = 1;

  if ( const QgsRasterVectorFieldRenderer *renderer = dynamic_cast<const QgsRasterVectorFieldRenderer *>( mRasterLayer ? mRasterLayer->renderer() : nullptr ) )
  {
    sourceMode = renderer->sourceMode();
    xBand = renderer->xBand();
    yBand = renderer->yBand();
    directionBand = renderer->directionBand();
    encoding = renderer->directionEncoding();
    settings = renderer->settings();
  }

  whileBlocking( mXBandComboBox )->setBand( xBand );
  whileBlocking( mYBandComboBox )->setBand( yBand );
  whileBlocking( mDirectionBandComboBox )->setBand( directionBand );
  whileBlocking( mSourceModeComboBox )->setCurrentIndex( mSourceModeComboBox->findData( QVariant::fromValue( sourceMode ) ) );
  whileBlocking( mDirectionEncodingComboBox )->setCurrentIndex( mDirectionEncodingComboBox->findData( QVariant::fromValue( encoding ) ) );

  // the magnitude support and range must both be known before the settings are pushed in, so that
  // the widget hides what the data cannot carry and classifies the color ramp against the data
  updateSourceMode();
  mVectorFieldSettingsWidget->setSettings( settings );

  connect( mXBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsRasterVectorFieldRendererWidget::onBandChanged );
  connect( mYBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsRasterVectorFieldRendererWidget::onBandChanged );
  connect( mDirectionBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsRasterVectorFieldRendererWidget::onBandChanged );
  connect( mDirectionEncodingComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterVectorFieldRendererWidget::widgetChanged );
  connect( mSourceModeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterVectorFieldRendererWidget::onSourceModeChanged );
  connect( mVectorFieldSettingsWidget, &QgsVectorFieldSettingsWidget::widgetChanged, this, &QgsRasterVectorFieldRendererWidget::widgetChanged );
}

Qgis::RasterVectorFieldSourceMode QgsRasterVectorFieldRendererWidget::currentSourceMode() const
{
  return mSourceModeComboBox->currentData().value<Qgis::RasterVectorFieldSourceMode>();
}

void QgsRasterVectorFieldRendererWidget::onSourceModeChanged()
{
  // the symbology widget keeps its settings across the switch, but the ones which depend on a
  // magnitude have to be pushed back through it so they are hidden or forced
  const QgsVectorFieldSettings settings = mVectorFieldSettingsWidget->settings();
  updateSourceMode();
  mVectorFieldSettingsWidget->setSettings( settings );

  emit widgetChanged();
}

void QgsRasterVectorFieldRendererWidget::updateSourceMode()
{
  const bool encoded = currentSourceMode() == Qgis::RasterVectorFieldSourceMode::EncodedDirection;
  mBandsStackedWidget->setCurrentWidget( encoded ? mDirectionBandPage : mComponentBandsPage );
  mVectorFieldSettingsWidget->setHasMagnitude( !encoded );

  updateMagnitudeRange();
}

void QgsRasterVectorFieldRendererWidget::onBandChanged()
{
  updateMagnitudeRange();
  emit widgetChanged();
}

void QgsRasterVectorFieldRendererWidget::updateMagnitudeRange()
{
  mMinimumMagnitude = std::numeric_limits<double>::quiet_NaN();
  mMaximumMagnitude = std::numeric_limits<double>::quiet_NaN();

  if ( currentSourceMode() == Qgis::RasterVectorFieldSourceMode::EncodedDirection )
  {
    // the encodings carry a direction only, every vector has a magnitude of one. Reading the
    // statistics of the band would only measure the codes themselves
    mMinimumMagnitude = 0;
    mMaximumMagnitude = 1;
    mVectorFieldSettingsWidget->setMagnitudeRange( mMinimumMagnitude, mMaximumMagnitude );
    return;
  }

  QgsRasterDataProvider *provider = mRasterLayer ? mRasterLayer->dataProvider() : nullptr;
  const int xBand = mXBandComboBox->currentBand();
  const int yBand = mYBandComboBox->currentBand();
  if ( !provider || xBand <= 0 || yBand <= 0 )
    return;

  const Qgis::RasterBandStatistics wanted = Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max;
  const QgsRasterBandStats xStats = provider->bandStatistics( xBand, wanted, mExtent );
  const QgsRasterBandStats yStats = provider->bandStatistics( yBand, wanted, mExtent );

  // the components are unrelated, so the largest magnitude they can combine into is the one where
  // both are at their extreme, and the smallest one is a vector of zero length
  const double largestX = std::max( std::fabs( xStats.minimumValue ), std::fabs( xStats.maximumValue ) );
  const double largestY = std::max( std::fabs( yStats.minimumValue ), std::fabs( yStats.maximumValue ) );
  if ( std::isnan( largestX ) || std::isnan( largestY ) )
    return;

  mMinimumMagnitude = 0;
  mMaximumMagnitude = std::hypot( largestX, largestY );

  mVectorFieldSettingsWidget->setMagnitudeRange( mMinimumMagnitude, mMaximumMagnitude );
}

QgsRasterRenderer *QgsRasterVectorFieldRendererWidget::renderer()
{
  QgsRasterDataProvider *provider = mRasterLayer ? mRasterLayer->dataProvider() : nullptr;
  if ( !provider )
    return nullptr;

  auto renderer = std::make_unique<QgsRasterVectorFieldRenderer>( provider );
  renderer->setSourceMode( currentSourceMode() );
  renderer->setXBand( mXBandComboBox->currentBand() );
  renderer->setYBand( mYBandComboBox->currentBand() );
  renderer->setDirectionBand( mDirectionBandComboBox->currentBand() );
  renderer->setDirectionEncoding( mDirectionEncodingComboBox->currentData().value<Qgis::RasterDirectionEncoding>() );
  renderer->setSettings( mVectorFieldSettingsWidget->settings() );
  renderer->setMinimumMagnitude( mMinimumMagnitude );
  renderer->setMaximumMagnitude( mMaximumMagnitude );
  return renderer.release();
}
