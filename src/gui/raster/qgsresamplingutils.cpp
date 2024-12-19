/***************************************************************************
  qgsresamplingutils.cpp
  --------------------
      begin                : 12/06/2020
      copyright            : (C) 2020 Even Rouault
      email                : even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterresampler.h"
#include "qgsrasterresamplefilter.h"
#include "qgsresamplingutils.h"
#include "moc_qgsresamplingutils.cpp"
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"

#include <QObject>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

/// @cond PRIVATE

QgsResamplingUtils::QgsResamplingUtils() = default;

void QgsResamplingUtils::initWidgets( QgsRasterLayer *rasterLayer, QComboBox *zoomedInResamplingComboBox, QComboBox *zoomedOutResamplingComboBox, QDoubleSpinBox *maximumOversamplingSpinBox, QCheckBox *cbEarlyResampling )
{
  mRasterLayer = rasterLayer;
  mZoomedInResamplingComboBox = zoomedInResamplingComboBox;
  mZoomedOutResamplingComboBox = zoomedOutResamplingComboBox;
  mMaximumOversamplingSpinBox = maximumOversamplingSpinBox;
  mCbEarlyResampling = cbEarlyResampling;

  for ( QComboBox *combo : { mZoomedInResamplingComboBox, mZoomedOutResamplingComboBox } )
  {
    combo->addItem( QObject::tr( "Nearest Neighbour" ), static_cast<int>( Qgis::RasterResamplingMethod::Nearest ) );
    combo->addItem( QObject::tr( "Bilinear (2x2 Kernel)" ), static_cast<int>( Qgis::RasterResamplingMethod::Bilinear ) );
    combo->addItem( QObject::tr( "Cubic (4x4 Kernel)" ), static_cast<int>( Qgis::RasterResamplingMethod::Cubic ) );
  }

  if ( mCbEarlyResampling->isChecked() )
  {
    addExtraEarlyResamplingMethodsToCombos();
  }

  QObject::connect( mCbEarlyResampling, &QCheckBox::toggled, this, [=]( bool state ) {
    if ( state )
      addExtraEarlyResamplingMethodsToCombos();
    else
      removeExtraEarlyResamplingMethodsFromCombos();
  } );
}

void QgsResamplingUtils::refreshWidgetsFromLayer()
{
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  mCbEarlyResampling->setVisible(
    provider && ( provider->providerCapabilities() & Qgis::RasterProviderCapability::ProviderHintCanPerformProviderResampling )
  );
  mCbEarlyResampling->setChecked( mRasterLayer->resamplingStage() == Qgis::RasterResamplingStage::Provider );

  switch ( mRasterLayer->resamplingStage() )
  {
    case Qgis::RasterResamplingStage::ResampleFilter:
      removeExtraEarlyResamplingMethodsFromCombos();
      break;
    case Qgis::RasterResamplingStage::Provider:
      addExtraEarlyResamplingMethodsToCombos();
      break;
  }

  if ( provider && mRasterLayer->resamplingStage() == Qgis::RasterResamplingStage::Provider )
  {
    mZoomedInResamplingComboBox->setCurrentIndex( mZoomedInResamplingComboBox->findData( static_cast<int>( provider->zoomedInResamplingMethod() ) ) );
    mZoomedOutResamplingComboBox->setCurrentIndex( mZoomedOutResamplingComboBox->findData( static_cast<int>( provider->zoomedOutResamplingMethod() ) ) );

    mMaximumOversamplingSpinBox->setValue( provider->maxOversampling() );
  }
  else
  {
    const QgsRasterResampleFilter *resampleFilter = mRasterLayer->resampleFilter();
    //set combo boxes to current resampling types
    if ( resampleFilter )
    {
      const QgsRasterResampler *zoomedInResampler = resampleFilter->zoomedInResampler();
      if ( zoomedInResampler )
      {
        if ( zoomedInResampler->type() == QLatin1String( "bilinear" ) )
        {
          mZoomedInResamplingComboBox->setCurrentIndex( mZoomedInResamplingComboBox->findData( static_cast<int>( Qgis::RasterResamplingMethod::Bilinear ) ) );
        }
        else if ( zoomedInResampler->type() == QLatin1String( "cubic" ) )
        {
          mZoomedInResamplingComboBox->setCurrentIndex( mZoomedInResamplingComboBox->findData( static_cast<int>( Qgis::RasterResamplingMethod::Cubic ) ) );
        }
      }
      else
      {
        mZoomedInResamplingComboBox->setCurrentIndex( mZoomedInResamplingComboBox->findData( static_cast<int>( Qgis::RasterResamplingMethod::Nearest ) ) );
      }

      const QgsRasterResampler *zoomedOutResampler = resampleFilter->zoomedOutResampler();
      if ( zoomedOutResampler )
      {
        if ( zoomedOutResampler->type() == QLatin1String( "bilinear" ) )
        {
          mZoomedOutResamplingComboBox->setCurrentIndex( mZoomedOutResamplingComboBox->findData( static_cast<int>( Qgis::RasterResamplingMethod::Bilinear ) ) );
        }
        else if ( zoomedOutResampler->type() == QLatin1String( "cubic" ) )
        {
          mZoomedOutResamplingComboBox->setCurrentIndex( mZoomedOutResamplingComboBox->findData( static_cast<int>( Qgis::RasterResamplingMethod::Cubic ) ) );
        }
      }
      else
      {
        mZoomedOutResamplingComboBox->setCurrentIndex( mZoomedOutResamplingComboBox->findData( static_cast<int>( Qgis::RasterResamplingMethod::Nearest ) ) );
      }
      mMaximumOversamplingSpinBox->setValue( resampleFilter->maxOversampling() );
    }
  }
}


void QgsResamplingUtils::refreshLayerFromWidgets()
{
  const Qgis::RasterResamplingMethod zoomedInMethod = static_cast<Qgis::RasterResamplingMethod>(
    mZoomedInResamplingComboBox->itemData( mZoomedInResamplingComboBox->currentIndex() ).toInt()
  );
  const Qgis::RasterResamplingMethod zoomedOutMethod = static_cast<Qgis::RasterResamplingMethod>(
    mZoomedOutResamplingComboBox->itemData( mZoomedOutResamplingComboBox->currentIndex() ).toInt()
  );

  mRasterLayer->setResamplingStage( mCbEarlyResampling->isChecked() ? Qgis::RasterResamplingStage::Provider : Qgis::RasterResamplingStage::ResampleFilter );
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( provider )
  {
    provider->setZoomedInResamplingMethod( zoomedInMethod );
    provider->setZoomedOutResamplingMethod( zoomedOutMethod );

    provider->setMaxOversampling( mMaximumOversamplingSpinBox->value() );
  }

  QgsRasterResampleFilter *resampleFilter = mRasterLayer->resampleFilter();
  if ( resampleFilter )
  {
    std::unique_ptr<QgsRasterResampler> zoomedInResampler;

    // NOLINTBEGIN(bugprone-branch-clone)
    switch ( zoomedInMethod )
    {
      case Qgis::RasterResamplingMethod::Nearest:
        break;

      case Qgis::RasterResamplingMethod::Bilinear:
        zoomedInResampler = std::make_unique<QgsBilinearRasterResampler>();
        break;

      case Qgis::RasterResamplingMethod::Cubic:
        zoomedInResampler = std::make_unique<QgsCubicRasterResampler>();
        break;

      case Qgis::RasterResamplingMethod::CubicSpline:
      case Qgis::RasterResamplingMethod::Lanczos:
      case Qgis::RasterResamplingMethod::Average:
      case Qgis::RasterResamplingMethod::Mode:
      case Qgis::RasterResamplingMethod::Gauss:

        // not supported as late resampler methods
        break;
    }
    // NOLINTEND(bugprone-branch-clone)

    resampleFilter->setZoomedInResampler( zoomedInResampler.release() );

    //raster resampling
    std::unique_ptr<QgsRasterResampler> zoomedOutResampler;

    // NOLINTBEGIN(bugprone-branch-clone)
    switch ( zoomedOutMethod )
    {
      case Qgis::RasterResamplingMethod::Nearest:
        break;

      case Qgis::RasterResamplingMethod::Bilinear:
        zoomedOutResampler = std::make_unique<QgsBilinearRasterResampler>();
        break;

      case Qgis::RasterResamplingMethod::Cubic:
        zoomedOutResampler = std::make_unique<QgsCubicRasterResampler>();
        break;


      case Qgis::RasterResamplingMethod::CubicSpline:
      case Qgis::RasterResamplingMethod::Lanczos:
      case Qgis::RasterResamplingMethod::Average:
      case Qgis::RasterResamplingMethod::Mode:
      case Qgis::RasterResamplingMethod::Gauss:
        // not supported as late resampler methods
        break;
    }
    // NOLINTEND(bugprone-branch-clone)

    resampleFilter->setZoomedOutResampler( zoomedOutResampler.release() );

    resampleFilter->setMaxOversampling( mMaximumOversamplingSpinBox->value() );
  }
}

void QgsResamplingUtils::addExtraEarlyResamplingMethodsToCombos()
{
  if ( mZoomedInResamplingComboBox->findData( static_cast<int>( Qgis::RasterResamplingMethod::CubicSpline ) ) != -1 )
    return; // already present

  for ( QComboBox *combo : { mZoomedInResamplingComboBox, mZoomedOutResamplingComboBox } )
  {
    combo->addItem( QObject::tr( "Cubic B-Spline (4x4 Kernel)" ), static_cast<int>( Qgis::RasterResamplingMethod::CubicSpline ) );
    combo->addItem( QObject::tr( "Lanczos (6x6 Kernel)" ), static_cast<int>( Qgis::RasterResamplingMethod::Lanczos ) );
    combo->addItem( QObject::tr( "Average" ), static_cast<int>( Qgis::RasterResamplingMethod::Average ) );
    combo->addItem( QObject::tr( "Mode" ), static_cast<int>( Qgis::RasterResamplingMethod::Mode ) );
    combo->addItem( QObject::tr( "Gauss" ), static_cast<int>( Qgis::RasterResamplingMethod::Gauss ) );
  }
}

void QgsResamplingUtils::removeExtraEarlyResamplingMethodsFromCombos()
{
  if ( mZoomedInResamplingComboBox->findData( static_cast<int>( Qgis::RasterResamplingMethod::CubicSpline ) ) == -1 )
    return; // already removed

  for ( QComboBox *combo : { mZoomedInResamplingComboBox, mZoomedOutResamplingComboBox } )
  {
    for ( const Qgis::RasterResamplingMethod method :
          {
            Qgis::RasterResamplingMethod::CubicSpline,
            Qgis::RasterResamplingMethod::Lanczos,
            Qgis::RasterResamplingMethod::Average,
            Qgis::RasterResamplingMethod::Mode,
            Qgis::RasterResamplingMethod::Gauss
          } )
    {
      combo->removeItem( combo->findData( static_cast<int>( method ) ) );
    }
  }
}

/// @endcond PRIVATE
