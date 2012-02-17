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
  return new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(),
                                        mRedBandComboBox->itemData( mRedBandComboBox->currentIndex() ).toInt(),
                                        mGreenBandComboBox->itemData( mGreenBandComboBox->currentIndex() ).toInt(),
                                        mBlueBandComboBox->itemData( mBlueBandComboBox->currentIndex() ).toInt()
                                      );
}
