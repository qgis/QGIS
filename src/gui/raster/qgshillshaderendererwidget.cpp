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
#include "qgsrasterdataprovider.h"

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
  mLightAzimuth_updated( 315.00 );
  mZFactor->setValue( 1 );
  mZFactor->setClearValue( 1 );

  mMultiDirection->setChecked( false );
  mBandsCombo->setLayer( mRasterLayer );

  setFromRenderer( layer->renderer() );

  connect( mLightAngle, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsRasterRendererWidget::widgetChanged );
  connect( mLightAzimuth, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsHillshadeRendererWidget::mLightAzimuth_updated );
  connect( mLightAzimuthDial, &QAbstractSlider::valueChanged, this, &QgsHillshadeRendererWidget::mLightAzimuthDial_updated );
  connect( mZFactor, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsRasterRendererWidget::widgetChanged );
  connect( mMultiDirection, &QAbstractButton::toggled, this, &QgsRasterRendererWidget::widgetChanged );
  connect( mBandsCombo, &QgsRasterBandComboBox::bandChanged, this, &QgsHillshadeRendererWidget::widgetChanged );
}

QgsRasterRenderer *QgsHillshadeRendererWidget::renderer()
{
  if ( !mRasterLayer )
  {
    return nullptr;
  }

  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return nullptr;
  }

  const int band = mBandsCombo->currentBand();
  QgsHillshadeRenderer *renderer = new QgsHillshadeRenderer( provider, band, mLightAzimuth->value(), mLightAngle->value() );
  const double value = mZFactor->value();
  renderer->setZFactor( value );
  renderer->setMultiDirectional( mMultiDirection->checkState() );
  return renderer;
}

void QgsHillshadeRendererWidget::setFromRenderer( const QgsRasterRenderer *renderer )
{
  const QgsHillshadeRenderer *r = dynamic_cast<const QgsHillshadeRenderer *>( renderer );
  if ( r )
  {
    mBandsCombo->setBand( r->band() );
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

void QgsHillshadeRendererWidget::mLightAzimuth_updated( double value )
{
  int newvalue = static_cast<int>( value ) - 180;
  if ( newvalue < 0 )
    newvalue += 360;
  whileBlocking( mLightAzimuthDial )->setValue( newvalue );
  emit widgetChanged();
}

void QgsHillshadeRendererWidget::mLightAzimuthDial_updated( int value )
{
  int newvalue = static_cast<int>( value ) + 180;
  if ( newvalue > 360 )
    newvalue -= 360;
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
