/***************************************************************************
  qgsattributetablerendererwidget.cpp - QgsAttributeTableRendererWidget

 ---------------------
 begin                : 3.12.2021
 copyright            : (C) 2021 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributetablerendererwidget.h"
#include "qgsattributetablerenderer.h"
#include "qgsrasterdataprovider.h"

QgsAttributeTableRendererWidget::QgsAttributeTableRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
  :   QgsRasterRendererWidget( layer, extent )
{

  setupUi( this );

  if ( !mRasterLayer )
  {
    return;
  }

  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return;
  }

  connect( mBandComboBox, &QgsRasterBandComboBox::bandChanged, [ = ]( int bandNumber )
  {
    if ( provider->attributeTable( bandNumber ).isValid() )
    {
      // TODO: show attr table
      mNoRatMessage->hide();
      mLoadRatButton->hide();
    }
    else
    {
      mNoRatMessage->show();
      mLoadRatButton->show();
    }
  } );

  mBandComboBox->setLayer( layer );

}

QgsRasterRenderer *QgsAttributeTableRendererWidget::renderer()
{
  const int bandNumber = mBandComboBox->currentBand();
  QgsAttributeTableRenderer *renderer = new QgsAttributeTableRenderer( mRasterLayer->dataProvider(), bandNumber );
  return renderer;
}
