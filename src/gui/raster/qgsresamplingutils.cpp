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

  mZoomedInResamplingComboBox->addItem( QObject::tr( "Nearest neighbour" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Nearest ) );
  mZoomedInResamplingComboBox->addItem( QObject::tr( "Bilinear" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Bilinear ) );
  mZoomedInResamplingComboBox->addItem( QObject::tr( "Cubic" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Cubic ) );

  mZoomedOutResamplingComboBox->addItem( QObject::tr( "Nearest neighbour" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Nearest ) );
  mZoomedOutResamplingComboBox->addItem( QObject::tr( "Bilinear" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Bilinear ) );
  mZoomedOutResamplingComboBox->addItem( QObject::tr( "Cubic" ), static_cast<int>( QgsRasterDataProvider::ResamplingMethod::Cubic ) );
}

void QgsResamplingUtils::refreshWidgetsFromLayer()
{
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  mCbEarlyResampling->setVisible(
    provider && ( provider->providerCapabilities() & QgsRasterDataProvider::ProviderHintCanPerformProviderResampling ) );
  mCbEarlyResampling->setChecked( mRasterLayer->resamplingStage() == QgsRasterPipe::ResamplingStage::Provider );

  if ( provider && mRasterLayer->resamplingStage() == QgsRasterPipe::ResamplingStage::Provider )
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
  QgsRasterDataProvider::ResamplingMethod zoomedInMethod =
    static_cast< QgsRasterDataProvider::ResamplingMethod >(
      mZoomedInResamplingComboBox->itemData( mZoomedInResamplingComboBox->currentIndex() ).toInt() );
  QgsRasterDataProvider::ResamplingMethod zoomedOutMethod =
    static_cast< QgsRasterDataProvider::ResamplingMethod >(
      mZoomedOutResamplingComboBox->itemData( mZoomedOutResamplingComboBox->currentIndex() ).toInt() );

  mRasterLayer->setResamplingStage( mCbEarlyResampling->isChecked() ? QgsRasterPipe::ResamplingStage::Provider : QgsRasterPipe::ResamplingStage::ResampleFilter );
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
    QgsRasterResampler *zoomedInResampler = nullptr;

    switch ( zoomedInMethod )
    {
      case QgsRasterDataProvider::ResamplingMethod::Nearest:
        break;

      case QgsRasterDataProvider::ResamplingMethod::Bilinear:
        zoomedInResampler = new QgsBilinearRasterResampler();
        break;

      case QgsRasterDataProvider::ResamplingMethod::Cubic:
        zoomedInResampler = new QgsCubicRasterResampler();
        break;
    }

    resampleFilter->setZoomedInResampler( zoomedInResampler );

    //raster resampling
    QgsRasterResampler *zoomedOutResampler = nullptr;

    switch ( zoomedOutMethod )
    {
      case QgsRasterDataProvider::ResamplingMethod::Nearest:
        break;

      case QgsRasterDataProvider::ResamplingMethod::Bilinear:
        zoomedOutResampler = new QgsBilinearRasterResampler();
        break;

      case QgsRasterDataProvider::ResamplingMethod::Cubic:
        zoomedOutResampler = new QgsCubicRasterResampler();
        break;
    }

    resampleFilter->setZoomedOutResampler( zoomedOutResampler );

    resampleFilter->setMaxOversampling( mMaximumOversamplingSpinBox->value() );
  }
}
