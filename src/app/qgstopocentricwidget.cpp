/***************************************************************************
    qgstopocentricwidget.cpp
    ---------------------
    begin                : March 2026
    copyright            : (C) 2026 by Dominik Cindrić
    email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ui_qgstopocentricwidget.h"
#include "qgstopocentricwidget.h"

#include <QDoubleSpinBox>
#include <QSlider>
#include <QTimer>

#include "moc_qgstopocentricwidget.cpp"

QgsTopocentricWidget::QgsTopocentricWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  doubleSpinBoxX->setRange( -90.0, 90.0 );
  doubleSpinBoxX->setDecimals( 1 );
  horizontalSliderX->setRange( -90, 90 );

  doubleSpinBoxY->setRange( -180.0, 180.0 );
  doubleSpinBoxY->setDecimals( 1 );
  horizontalSliderY->setRange( -180, 180 );

  mEditTimer = new QTimer( this );
  mEditTimer->setSingleShot( true );
  mEditTimer->setInterval( 250 );
  connect( mEditTimer, &QTimer::timeout, this, [this]() {
    emit originChanged( latitude(), longitude() );
  } );

  connect( horizontalSliderX, &QSlider::valueChanged, this, [this]( int v ) {
    QSignalBlocker blocker( doubleSpinBoxX );
    doubleSpinBoxX->setValue( static_cast<double>( v ) );
    mEditTimer->start();
  } );

  connect( doubleSpinBoxX, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double v ) {
    QSignalBlocker blocker( horizontalSliderX );
    horizontalSliderX->setValue( static_cast<int>( v ) );
    emit originChanged( latitude(), longitude() );
  } );

  connect( horizontalSliderY, &QSlider::valueChanged, this, [this]( int v ) {
    QSignalBlocker blocker( doubleSpinBoxY );
    doubleSpinBoxY->setValue( static_cast<double>( v ) );
    mEditTimer->start();
  } );

  connect( doubleSpinBoxY, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double v ) {
    QSignalBlocker blocker( horizontalSliderY );
    horizontalSliderY->setValue( static_cast<int>( v ) );
    emit originChanged( latitude(), longitude() );
  } );
}

double QgsTopocentricWidget::latitude() const
{
  return doubleSpinBoxX->value();
}

double QgsTopocentricWidget::longitude() const
{
  return doubleSpinBoxY->value();
}

void QgsTopocentricWidget::setLatitude( double lat )
{
  doubleSpinBoxX->setValue( lat );
}

void QgsTopocentricWidget::setLongitude( double lon )
{
  doubleSpinBoxY->setValue( lon );
}
