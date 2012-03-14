/***************************************************************************
                         qgsmultibandcolorrendererwidget.cpp
                         -----------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmultibandcolorrendererwidget.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"

QgsMultiBandColorRendererWidget::QgsMultiBandColorRendererWidget( QgsRasterLayer* layer ): QgsRasterRendererWidget( layer )
{
  setupUi( this );
  createValidators();

  if ( mRasterLayer )
  {
    QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
    if ( !provider )
    {
      return;
    }

    //fill available bands into combo boxes
    mRedBandComboBox->addItem( tr( "Not set" ), -1 );
    mGreenBandComboBox->addItem( tr( "Not set" ), -1 );
    mBlueBandComboBox->addItem( tr( "Not set" ), -1 );

    //contrast enhancement algorithms
    mContrastEnhancementAlgorithmComboBox->addItem( tr( "No enhancement" ), 0 );
    mContrastEnhancementAlgorithmComboBox->addItem( tr( "Stretch to MinMax" ), 1 );
    mContrastEnhancementAlgorithmComboBox->addItem( tr( "Stretch and clip to MinMax" ), 2 );
    mContrastEnhancementAlgorithmComboBox->addItem( tr( "Clip to MinMax" ), 3 );

    int nBands = provider->bandCount();
    for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
    {
      mRedBandComboBox->addItem( provider->colorInterpretationName( i ), i );
      mGreenBandComboBox->addItem( provider->colorInterpretationName( i ), i );
      mBlueBandComboBox->addItem( provider->colorInterpretationName( i ), i );
    }

    QgsMultiBandColorRenderer* r = dynamic_cast<QgsMultiBandColorRenderer*>( mRasterLayer->renderer() );
    if ( r )
    {
      mRedBandComboBox->setCurrentIndex( mRedBandComboBox->findData( r->redBand() ) );
      mGreenBandComboBox->setCurrentIndex( mGreenBandComboBox->findData( r->greenBand() ) );
      mBlueBandComboBox->setCurrentIndex( mBlueBandComboBox->findData( r->blueBand() ) );
    }
    else
    {
      mRedBandComboBox->setCurrentIndex( mRedBandComboBox->findText( tr( "Red" ) ) );
      mGreenBandComboBox->setCurrentIndex( mGreenBandComboBox->findText( tr( "Green" ) ) );
      mBlueBandComboBox->setCurrentIndex( mBlueBandComboBox->findText( tr( "Blue" ) ) );
    }
  }
}

QgsMultiBandColorRendererWidget::~QgsMultiBandColorRendererWidget()
{
}

QgsRasterRenderer* QgsMultiBandColorRendererWidget::renderer()
{
  if ( !mRasterLayer )
  {
    return 0;
  }
  QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return 0;
  }

  int redBand = mRedBandComboBox->itemData( mRedBandComboBox->currentIndex() ).toInt();
  int greenBand = mGreenBandComboBox->itemData( mGreenBandComboBox->currentIndex() ).toInt();
  int blueBand = mBlueBandComboBox->itemData( mBlueBandComboBox->currentIndex() ).toInt();

  QgsMultiBandColorRenderer* r = new QgsMultiBandColorRenderer( provider, redBand, greenBand, blueBand );
  setCustomMinMaxValues( r, provider, redBand, greenBand, blueBand );
  return r;
}

void QgsMultiBandColorRendererWidget::createValidators()
{
  mRedMinLineEdit->setValidator( new QDoubleValidator( mRedMinLineEdit ) );
  mRedMaxLineEdit->setValidator( new QDoubleValidator( mRedMinLineEdit ) );
  mGreenMinLineEdit->setValidator( new QDoubleValidator( mGreenMinLineEdit ) );
  mGreenMaxLineEdit->setValidator( new QDoubleValidator( mGreenMinLineEdit ) );
  mBlueMinLineEdit->setValidator( new QDoubleValidator( mBlueMinLineEdit ) );
  mBlueMaxLineEdit->setValidator( new QDoubleValidator( mBlueMinLineEdit ) );
}

void QgsMultiBandColorRendererWidget::setCustomMinMaxValues( QgsMultiBandColorRenderer* r,
    const QgsRasterDataProvider* provider,
    int redBand, int greenBand, int blueBand )
{
  if ( !r || !provider )
  {
    return;
  }

  if ( mContrastEnhancementAlgorithmComboBox->itemData( mContrastEnhancementAlgorithmComboBox->currentIndex() ).toInt() ==
       QgsContrastEnhancement::NoEnhancement )
  {
    r->setRedContrastEnhancement( 0 );
    r->setGreenContrastEnhancement( 0 );
    r->setBlueContrastEnhancement( 0 );
    return;
  }

  QgsContrastEnhancement* redEnhancement = 0;
  QgsContrastEnhancement* greenEnhancement = 0;
  QgsContrastEnhancement* blueEnhancement = 0;

  bool redMinOk, redMaxOk;
  double redMin = mRedMinLineEdit->text().toDouble( &redMinOk );
  double redMax = mRedMaxLineEdit->text().toDouble( &redMaxOk );
  if ( redMinOk && redMaxOk )
  {
    redEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
          provider->dataType( redBand ) ) );
    redEnhancement->setMinimumValue( redMin );
    redEnhancement->setMaximumValue( redMax );
  }

  bool greenMinOk, greenMaxOk;
  double greenMin = mGreenMinLineEdit->text().toDouble( &greenMinOk );
  double greenMax = mGreenMaxLineEdit->text().toDouble( &greenMaxOk );
  if ( greenMinOk && greenMaxOk )
  {
    greenEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
          provider->dataType( greenBand ) ) );
    greenEnhancement->setMinimumValue( greenMin );
    greenEnhancement->setMaximumValue( greenMax );
  }

  bool blueMinOk, blueMaxOk;
  double blueMin = mBlueMinLineEdit->text().toDouble( &blueMinOk );
  double blueMax = mBlueMaxLineEdit->text().toDouble( &blueMaxOk );
  if ( blueMinOk && blueMaxOk )
  {
    blueEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
          provider->dataType( blueBand ) ) );
    blueEnhancement->setMinimumValue( blueMin );
    blueEnhancement->setMaximumValue( blueMax );
  }

  if ( redEnhancement )
  {
    redEnhancement->setContrastEnhancementAlgorithm(( QgsContrastEnhancement::ContrastEnhancementAlgorithm )
        ( mContrastEnhancementAlgorithmComboBox->itemData( mContrastEnhancementAlgorithmComboBox->currentIndex() ).toInt() ) );
  }
  if ( greenEnhancement )
  {
    greenEnhancement->setContrastEnhancementAlgorithm(( QgsContrastEnhancement::ContrastEnhancementAlgorithm )
        ( mContrastEnhancementAlgorithmComboBox->itemData( mContrastEnhancementAlgorithmComboBox->currentIndex() ).toInt() ) );
  }
  if ( blueEnhancement )
  {
    blueEnhancement->setContrastEnhancementAlgorithm(( QgsContrastEnhancement::ContrastEnhancementAlgorithm )
        ( mContrastEnhancementAlgorithmComboBox->itemData( mContrastEnhancementAlgorithmComboBox->currentIndex() ).toInt() ) );
  }
  r->setRedContrastEnhancement( redEnhancement );
  r->setGreenContrastEnhancement( greenEnhancement );
  r->setBlueContrastEnhancement( blueEnhancement );
}
