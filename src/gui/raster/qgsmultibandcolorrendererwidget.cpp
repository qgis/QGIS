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

    setFromRenderer( mRasterLayer->renderer() );
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

void QgsMultiBandColorRendererWidget::on_mLoadPushButton_clicked()
{
  int redBand = mRedBandComboBox->itemData( mRedBandComboBox->currentIndex() ).toInt();
  int greenBand = mGreenBandComboBox->itemData( mGreenBandComboBox->currentIndex() ).toInt();
  int blueBand = mBlueBandComboBox->itemData( mBlueBandComboBox->currentIndex() ).toInt();

  loadMinMaxValueForBand( redBand, mRedMinLineEdit, mRedMaxLineEdit );
  loadMinMaxValueForBand( greenBand, mGreenMinLineEdit, mGreenMaxLineEdit );
  loadMinMaxValueForBand( blueBand, mBlueMinLineEdit, mBlueMaxLineEdit );
}

void QgsMultiBandColorRendererWidget::setMinMaxValue( const QgsContrastEnhancement* ce, QLineEdit* minEdit, QLineEdit* maxEdit )
{
  if ( !minEdit || !maxEdit )
  {
    return;
  }

  if ( !ce )
  {
    minEdit->clear();
    maxEdit->clear();
    return;
  }

  minEdit->setText( QString::number( ce->minimumValue() ) );
  maxEdit->setText( QString::number( ce->maximumValue() ) );
  mContrastEnhancementAlgorithmComboBox->setCurrentIndex( mContrastEnhancementAlgorithmComboBox->findData(
        ( int )( ce->contrastEnhancementAlgorithm() ) ) );
}

void QgsMultiBandColorRendererWidget::loadMinMaxValueForBand( int band, QLineEdit* minEdit, QLineEdit* maxEdit )
{
  if ( !minEdit || !maxEdit || !mRasterLayer )
  {
    return;
  }

  QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return;
  }

  if ( band < 0 )
  {
    minEdit->clear();
    maxEdit->clear();
    return;
  }

  double minVal = 0;
  double maxVal = 0;
  if ( mEstimateRadioButton->isChecked() )
  {
    minVal = provider->minimumValue( band );
    maxVal = provider->maximumValue( band );
  }
  else if ( mActualRadioButton->isChecked() )
  {
    QgsRasterBandStats rasterBandStats = mRasterLayer->bandStatistics( band );
    minVal = rasterBandStats.minimumValue;
    maxVal = rasterBandStats.maximumValue;
  }
  else if ( mCurrentExtentRadioButton->isChecked() )
  {
    double minMax[2];
    mRasterLayer->computeMinimumMaximumFromLastExtent( band, minMax );
    minVal = minMax[0];
    maxVal = minMax[1];
  }

  minEdit->setText( QString::number( minVal ) );
  maxEdit->setText( QString::number( maxVal ) );
}

void QgsMultiBandColorRendererWidget::setFromRenderer( const QgsRasterRenderer* r )
{
  const QgsMultiBandColorRenderer* mbcr = dynamic_cast<const QgsMultiBandColorRenderer*>( r );
  if ( mbcr )
  {
    mRedBandComboBox->setCurrentIndex( mRedBandComboBox->findData( mbcr->redBand() ) );
    mGreenBandComboBox->setCurrentIndex( mGreenBandComboBox->findData( mbcr->greenBand() ) );
    mBlueBandComboBox->setCurrentIndex( mBlueBandComboBox->findData( mbcr->blueBand() ) );

    setMinMaxValue( mbcr->redContrastEnhancement(), mRedMinLineEdit, mRedMaxLineEdit );
    setMinMaxValue( mbcr->greenContrastEnhancement(), mGreenMinLineEdit, mGreenMaxLineEdit );
    setMinMaxValue( mbcr->blueContrastEnhancement(), mBlueMinLineEdit, mBlueMaxLineEdit );
  }
  else
  {
    mRedBandComboBox->setCurrentIndex( mRedBandComboBox->findText( tr( "Red" ) ) );
    mGreenBandComboBox->setCurrentIndex( mGreenBandComboBox->findText( tr( "Green" ) ) );
    mBlueBandComboBox->setCurrentIndex( mBlueBandComboBox->findText( tr( "Blue" ) ) );
  }
}
