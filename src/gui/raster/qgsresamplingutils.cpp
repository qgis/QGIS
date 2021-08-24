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
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"

#include <QObject>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

/// @cond PRIVATE

QgsResamplingUtils::QgsResamplingUtils() = default;

void QgsResamplingUtils::initWidgets( QgsRasterLayer *rasterLayer,
                                      QComboBox  *zoomedInResamplingComboBox,
                                      QComboBox *zoomedOutResamplingComboBox,
                                      QDoubleSpinBox *maximumOversamplingSpinBox,
                                      QCheckBox *cbEarlyResampling )
{
  mRasterLayer = rasterLayer;
  mZoomedInResamplingComboBox = zoomedInResamplingComboBox;
  mZoomedOutResamplingComboBox = zoomedOutResamplingComboBox;
  mMaximumOversamplingSpinBox = maximumOversamplingSpinBox;
  mCbEarlyResampling = cbEarlyResampling;

  for ( QComboBox *combo :  {mZoomedInResamplingComboBox, mZoomedOutResamplingComboBox } )
  {
    combo->addItem( QObject::tr( "Nearest Neighbour" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Nearest ) );
    combo->addItem( QObject::tr( "Bilinear" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Bilinear ) );
    combo->addItem( QObject::tr( "Cubic" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Cubic ) );
  }

  if ( mCbEarlyResampling->isChecked() )
  {
    addExtraEarlyResamplingMethodsToCombos();
  }

  QObject::connect( mCbEarlyResampling, &QCheckBox::toggled, this, [ = ]( bool state )
  {
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
    provider && ( provider->providerCapabilities() & QgsRasterDataProvider::ProviderHintCanPerformProviderResampling ) );
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
          mZoomedInResamplingComboBox->setCurrentIndex( mZoomedInResamplingComboBox->findData( static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Bilinear ) ) );
        }
        else if ( zoomedInResampler->type() == QLatin1String( "cubic" ) )
        {
          mZoomedInResamplingComboBox->setCurrentIndex( mZoomedInResamplingComboBox->findData( static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Cubic ) ) );
        }
      }
      else
      {
        mZoomedInResamplingComboBox->setCurrentIndex( mZoomedInResamplingComboBox->findData( static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Nearest ) ) );
      }

      const QgsRasterResampler *zoomedOutResampler = resampleFilter->zoomedOutResampler();
      if ( zoomedOutResampler )
      {
        if ( zoomedOutResampler->type() == QLatin1String( "bilinear" ) )
        {
          mZoomedOutResamplingComboBox->setCurrentIndex( mZoomedOutResamplingComboBox->findData( static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Bilinear ) ) );
        }
        else if ( zoomedOutResampler->type() == QLatin1String( "cubic" ) )
        {
          mZoomedOutResamplingComboBox->setCurrentIndex( mZoomedOutResamplingComboBox->findData( static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Cubic ) ) );
        }
      }
      else
      {
        mZoomedOutResamplingComboBox->setCurrentIndex( mZoomedOutResamplingComboBox->findData( static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Nearest ) ) );
      }
      mMaximumOversamplingSpinBox->setValue( resampleFilter->maxOversampling() );
    }
  }
}


void QgsResamplingUtils::refreshLayerFromWidgets()
{
  const QgsRasterDataProvider::ResamplingMethod zoomedInMethod =
    static_cast< QgsRasterDataProvider::ResamplingMethod >(
      mZoomedInResamplingComboBox->itemData( mZoomedInResamplingComboBox->currentIndex() ).toInt() );
  const QgsRasterDataProvider::ResamplingMethod zoomedOutMethod =
    static_cast< QgsRasterDataProvider::ResamplingMethod >(
      mZoomedOutResamplingComboBox->itemData( mZoomedOutResamplingComboBox->currentIndex() ).toInt() );

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
    std::unique_ptr< QgsRasterResampler > zoomedInResampler;

    switch ( zoomedInMethod )
    {
      case QgsRasterDataProvider::ResamplingMethod::Nearest:
        break;

      case QgsRasterDataProvider::ResamplingMethod::Bilinear:
        zoomedInResampler = std::make_unique< QgsBilinearRasterResampler >();
        break;

      case QgsRasterDataProvider::ResamplingMethod::Cubic:
        zoomedInResampler = std::make_unique< QgsCubicRasterResampler >();
        break;

      case QgsRasterDataProvider::ResamplingMethod::CubicSpline:
      case QgsRasterDataProvider::ResamplingMethod::Lanczos:
      case QgsRasterDataProvider::ResamplingMethod::Average:
      case QgsRasterDataProvider::ResamplingMethod::Mode:
      case QgsRasterDataProvider::ResamplingMethod::Gauss:

        // not supported as late resampler methods
        break;
    }

    resampleFilter->setZoomedInResampler( zoomedInResampler.release() );

    //raster resampling
    std::unique_ptr< QgsRasterResampler > zoomedOutResampler;

    switch ( zoomedOutMethod )
    {
      case QgsRasterDataProvider::ResamplingMethod::Nearest:
        break;

      case QgsRasterDataProvider::ResamplingMethod::Bilinear:
        zoomedOutResampler = std::make_unique< QgsBilinearRasterResampler >();
        break;

      case QgsRasterDataProvider::ResamplingMethod::Cubic:
        zoomedOutResampler = std::make_unique< QgsCubicRasterResampler >();
        break;


      case QgsRasterDataProvider::ResamplingMethod::CubicSpline:
      case QgsRasterDataProvider::ResamplingMethod::Lanczos:
      case QgsRasterDataProvider::ResamplingMethod::Average:
      case QgsRasterDataProvider::ResamplingMethod::Mode:
      case QgsRasterDataProvider::ResamplingMethod::Gauss:
        // not supported as late resampler methods
        break;
    }

    resampleFilter->setZoomedOutResampler( zoomedOutResampler.release() );

    resampleFilter->setMaxOversampling( mMaximumOversamplingSpinBox->value() );
  }
}

void QgsResamplingUtils::addExtraEarlyResamplingMethodsToCombos()
{
  if ( mZoomedInResamplingComboBox->findData( static_cast<int>( QgsRasterDataProvider::ResamplingMethod::CubicSpline ) ) != -1 )
    return; // already present

  for ( QComboBox *combo : {mZoomedInResamplingComboBox, mZoomedOutResamplingComboBox } )
  {
    combo->addItem( QObject::tr( "Cubic Spline" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::CubicSpline ) );
    combo->addItem( QObject::tr( "Lanczos" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Lanczos ) );
    combo->addItem( QObject::tr( "Average" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Average ) );
    combo->addItem( QObject::tr( "Mode" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Mode ) );
    combo->addItem( QObject::tr( "Gauss" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Gauss ) );
  }
}

void QgsResamplingUtils::removeExtraEarlyResamplingMethodsFromCombos()
{
  if ( mZoomedInResamplingComboBox->findData( static_cast<int>( QgsRasterDataProvider::ResamplingMethod::CubicSpline ) ) == -1 )
    return; // already removed

  for ( QComboBox *combo : {mZoomedInResamplingComboBox, mZoomedOutResamplingComboBox } )
  {
    for ( const QgsRasterDataProvider::ResamplingMethod method :
          {
            QgsRasterDataProvider::ResamplingMethod::CubicSpline,
            QgsRasterDataProvider::ResamplingMethod::Lanczos,
            QgsRasterDataProvider::ResamplingMethod::Average,
            QgsRasterDataProvider::ResamplingMethod::Mode,
            QgsRasterDataProvider::ResamplingMethod::Gauss
          } )
    {
      combo->removeItem( combo->findData( static_cast< int >( method ) ) );
    }
  }
}

/// @endcond PRIVATE
