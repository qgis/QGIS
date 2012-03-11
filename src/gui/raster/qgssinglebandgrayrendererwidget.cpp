/***************************************************************************
                         qgssinglebandgrayrendererwidget.h
                         ---------------------------------
    begin                : March 2012
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

#include "qgssinglebandgrayrendererwidget.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgsrasterlayer.h"

QgsSingleBandGrayRendererWidget::QgsSingleBandGrayRendererWidget( QgsRasterLayer* layer ): QgsRasterRendererWidget( layer )
{
  setupUi( this );

  if ( mRasterLayer )
  {
    QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
    if ( !provider )
    {
      return;
    }

    //fill available bands into combo box
    int nBands = provider->bandCount();
    for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
    {
      mGrayBandComboBox->addItem( provider->colorInterpretationName( i ), i );
    }

    //contrast enhancement algorithms
    mContrastEnhancementComboBox->addItem( tr( "No enhancement" ), 0 );
    mContrastEnhancementComboBox->addItem( tr( "Stretch to MinMax" ), 1 );
    mContrastEnhancementComboBox->addItem( tr( "Stretch and clip to MinMax" ), 2 );
    mContrastEnhancementComboBox->addItem( tr( "Clip to MinMax" ), 3 );

    QgsSingleBandGrayRenderer* r = dynamic_cast<QgsSingleBandGrayRenderer*>( layer->renderer() );
    if ( r )
    {
      //band
      mGrayBandComboBox->setCurrentIndex( mGrayBandComboBox->findData( r->grayBand() ) );
      const QgsContrastEnhancement* ce = r->contrastEnhancement();
      //minmax
      mMinLineEdit->setText( QString::number( ce->minimumValue() ) );
      mMaxLineEdit->setText( QString::number( ce->maximumValue() ) );
      //contrast enhancement algorithm
      mContrastEnhancementComboBox->setCurrentIndex(
        mContrastEnhancementComboBox->findData(( int )( ce->contrastEnhancementAlgorithm() ) ) );
    }
  }
}

QgsSingleBandGrayRendererWidget::~QgsSingleBandGrayRendererWidget()
{
}

QgsRasterRenderer* QgsSingleBandGrayRendererWidget::renderer()
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
  int band = mGrayBandComboBox->itemData( mGrayBandComboBox->currentIndex() ).toInt();

  QgsContrastEnhancement* e = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
        provider->dataType( band ) ) );
  e->setMinimumValue( mMinLineEdit->text().toInt() );
  e->setMaximumValue( mMaxLineEdit->text().toInt() );
  e->setContrastEnhancementAlgorithm(( QgsContrastEnhancement::ContrastEnhancementAlgorithm )( mContrastEnhancementComboBox->itemData(
                                       mContrastEnhancementComboBox->currentIndex() ).toInt() ) );


  QgsSingleBandGrayRenderer* renderer = new QgsSingleBandGrayRenderer( provider, band );
  renderer->setContrastEnhancement( e );
  return renderer;
}
