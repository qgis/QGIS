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
  }
}

QgsSingleBandGrayRendererWidget::~QgsSingleBandGrayRendererWidget()
{
}

QgsRasterRenderer* QgsSingleBandGrayRendererWidget::renderer()
{
  return 0; //soon...
}
