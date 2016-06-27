/***************************************************************************
                         qgshillshaderendererwidget.cpp
                         ---------------------------------
    begin                : May 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
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
  mLightAngle->setClearValue( 45.0 );
  mLightAzimuth->setValue( 315.00 );
  mLightAzimuth->setClearValue( 315.00 );

  // Update the dial correctly
  on_mLightAzimuth_updated( 315.00 );
  mZFactor->setValue( 1 );
  mZFactor->setClearValue( 1 );

  mMultiDirection->setChecked( false );

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
  connect( mLightAzimuth, SIGNAL( valueChanged( double ) ), this, SLOT( on_mLightAzimuth_updated( double ) ) );
  connect( mLightAzimuthDial, SIGNAL( valueChanged( int ) ), this, SLOT( on_mLightAzimuthDail_updated( int ) ) );
  connect( mZFactor, SIGNAL( valueChanged( double ) ), this, SIGNAL( widgetChanged() ) );
  connect( mMultiDirection, SIGNAL( toggled( bool ) ), this, SIGNAL( widgetChanged() ) );
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
  renderer->setMultiDirectional( mMultiDirection->checkState() );
  return renderer;
}

void QgsHillshadeRendererWidget::setFromRenderer( const QgsRasterRenderer *renderer )
{
  const QgsHillshadeRenderer* r = dynamic_cast<const QgsHillshadeRenderer*>( renderer );
  if ( r )
  {
    mBandsCombo->setCurrentIndex( mBandsCombo->findData( r->band() ) );
    mLightAngle->setValue( r->altitude() );
    mLightAzimuth->setValue( r->azimuth() );
    mZFactor->setValue( r->zFactor() );
    mMultiDirection->setChecked( r->multiDirectional() );
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

void QgsHillshadeRendererWidget::setMultiDirectional( bool isMultiDirectional )
{
  mMultiDirection->setChecked( isMultiDirectional );
}

void QgsHillshadeRendererWidget::on_mLightAzimuth_updated( double value )
{
  int newvalue = ( int )value - 180;
  if ( newvalue < 0 )
    newvalue += 360;
  whileBlocking( mLightAzimuthDial )->setValue( newvalue );
  emit widgetChanged();
}

void QgsHillshadeRendererWidget::on_mLightAzimuthDail_updated( int value )
{
  int newvalue = ( int )value + 180;
  if ( newvalue > 360 )
    newvalue -= 360 ;
  whileBlocking( mLightAzimuth )->setValue( newvalue );
  emit widgetChanged();
}

double QgsHillshadeRendererWidget::azimuth() const
{
  return mLightAzimuth->value();
}

double QgsHillshadeRendererWidget::altitude() const
{
  return mLightAngle->value();
}

double QgsHillshadeRendererWidget::zFactor() const
{
  return mZFactor->value();
}

bool QgsHillshadeRendererWidget::multiDirectional() const
{
  return mMultiDirection->isChecked();
}
