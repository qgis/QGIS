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

#include "qgshillshaderendererwidget.h"
#include "qgsrasterlayer.h"
#include "qgsbilinearrasterresampler.h"
#include "qgshillshaderenderer.h"


QgsHillshadeRendererWidget::QgsHillshadeRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
    : QgsRasterRendererWidget( layer, extent )
{
  setupUi( this );

  mLightAngle->setMaximum( 90 );
  mLightAzimuth->setMaximum( 360.00 );

  mLightAngle->setValue( 45.00 );
  mLightAzimuth->setValue( 315.00 );
  mZFactor->setValue( 1 );

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
      mBandsCombo->addItem( displayBandName( i ), i );
    }

  }

  setFromRenderer( layer->renderer() );

  connect( mLightAngle, SIGNAL( valueChanged( double ) ), this, SIGNAL( widgetChanged() ) );
  connect( mLightAzimuth, SIGNAL( valueChanged( double ) ), this, SIGNAL( widgetChanged() ) );
  connect( mZFactor, SIGNAL( valueChanged( double ) ), this, SIGNAL( widgetChanged() ) );

  QgsBilinearRasterResampler* zoomedInResampler = new QgsBilinearRasterResampler();
  layer->resampleFilter()->setZoomedInResampler( zoomedInResampler );

}

QgsHillshadeRendererWidget::~QgsHillshadeRendererWidget()
{

}

QgsRasterRenderer *QgsHillshadeRendererWidget::renderer()
{
  if ( !mRasterLayer )
  {
    return nullptr;
  }

  QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return nullptr;
  }

  int band = mBandsCombo->itemData( mBandsCombo->currentIndex() ).toInt();
  QgsHillshadeRenderer* renderer = new QgsHillshadeRenderer( provider, band, mLightAzimuth->value(), mLightAngle->value() );
  double value = mZFactor->value();
  renderer->setZFactor( value );
  return renderer;
}

void QgsHillshadeRendererWidget::setFromRenderer( const QgsRasterRenderer *renderer )
{
  const QgsHillshadeRenderer* r = dynamic_cast<const QgsHillshadeRenderer*>( renderer );
  if ( r )
  {
    mBandsCombo->setCurrentIndex( mBandsCombo->findData( r->band() ) );
    mLightAngle->setValue( r->Angle() );
    mLightAzimuth->setValue( r->Azimuth() );
    mZFactor->setValue( r->zFactor() );
  }
}

void QgsHillshadeRendererWidget::setAltitude( double altitude )
{
  mLightAngle->setValue( altitude );
}

void QgsHillshadeRendererWidget::setAzimuth( double azimuth )
{
  mLightAzimuth->setValue( azimuth );
}

void QgsHillshadeRendererWidget::setZFactor( double zfactor )
{
  mZFactor->setValue( zfactor );
}
