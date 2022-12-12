/***************************************************************************
  qgsdirectionallightwidget.cpp - QgsDirectionalLightWidget

 ---------------------
 begin                : 11.12.2022
 copyright            : (C) 2022 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdirectionallightwidget.h"
#include "ui_qgsdirectionallightwidget.h"

#include "qgis.h"

QgsDirectionalLightWidget::QgsDirectionalLightWidget( QWidget *parent ) :
  QWidget( parent )
{
  setupUi( this );

  connect( mAzimuthSpinBox,  qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value )
  {
    whileBlocking( mDialAzimuth )->setValue( static_cast<int>( value * 10 ) );
  } );

  connect( mDialAzimuth, &QDial::valueChanged, this, [this]( int value )
  {
    whileBlocking( mAzimuthSpinBox )->setValue( value / 10.0 );
  } );

  connect( mAltitudeSpinBox,  qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value )
  {
    whileBlocking( mAltitudeSlider )->setValue( static_cast<int>( value * 10 ) );
  } );

  connect( mAltitudeSlider, &QSlider::valueChanged, this, [this]( int value )
  {
    whileBlocking( mAltitudeSpinBox )->setValue( value / 10.0 );
  } );
}

QgsDirectionalLightWidget::~QgsDirectionalLightWidget()
{
}

void QgsDirectionalLightWidget::setAzimuth( double azimuth )
{
  mAzimuthSpinBox->setValue( azimuth );
}

double QgsDirectionalLightWidget::azimuth() const
{
  return mAzimuthSpinBox->value();
}

void QgsDirectionalLightWidget::setAltitude( double altitude )
{
  mAltitudeSpinBox->setValue( altitude );
}

double QgsDirectionalLightWidget::altitude() const
{
  return mAltitudeSpinBox->value();
}
