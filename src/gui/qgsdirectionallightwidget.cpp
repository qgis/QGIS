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
#include "moc_qgsdirectionallightwidget.cpp"
#include "ui_qgsdirectionallightwidget.h"

#include "qgis.h"
#include <QDebug>

QgsDirectionalLightWidget::QgsDirectionalLightWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mAzimuthSpinBox->setClearValue( 315.0 );
  mAltitudeSpinBox->setClearValue( 45.0 );

  connect( mAzimuthSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    whileBlocking( mDialAzimuth )->setValue( static_cast<int>( value * 10 + 1800 ) % 3600 );
    emit directionChanged();
  } );

  connect( mDialAzimuth, &QDial::valueChanged, this, [this]( int value ) {
    whileBlocking( mAzimuthSpinBox )->setValue( std::fmod( value / 10.0 + 180, 360.0 ) );
    emit directionChanged();
  } );

  connect( mAltitudeSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    whileBlocking( mAltitudeSlider )->setValue( static_cast<int>( value * 10 ) );
    emit directionChanged();
  } );

  connect( mAltitudeSlider, &QSlider::valueChanged, this, [this]( int value ) {
    whileBlocking( mAltitudeSpinBox )->setValue( value / 10.0 );
    emit directionChanged();
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

void QgsDirectionalLightWidget::setEnableAzimuth( bool enable )
{
  mAzimuthSpinBox->setEnabled( enable );
  mDialAzimuth->setEnabled( enable );
}
