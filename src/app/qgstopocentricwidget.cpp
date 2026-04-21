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

#include "qgstopocentricwidget.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

#include "moc_qgstopocentricwidget.cpp"

QgsTopocentricWidget::QgsTopocentricWidget( QWidget *parent )
  : QWidget( parent )
{
  doubleSpinBoxX = new QDoubleSpinBox();
  horizontalSliderX = new QSlider( Qt::Horizontal );
  doubleSpinBoxY = new QDoubleSpinBox();
  horizontalSliderY = new QSlider( Qt::Horizontal );

  QHBoxLayout *latLayout = new QHBoxLayout();
  latLayout->addWidget( new QLabel( tr( "Latitude:" ) ) );
  latLayout->addWidget( horizontalSliderX );
  latLayout->addWidget( doubleSpinBoxX );

  QHBoxLayout *lonLayout = new QHBoxLayout();
  lonLayout->addWidget( new QLabel( tr( "Longitude:" ) ) );
  lonLayout->addWidget( horizontalSliderY );
  lonLayout->addWidget( doubleSpinBoxY );

  QLabel *titleLabel = new QLabel( tr( "Topocentric origin:" ) );
  titleLabel->setAlignment( Qt::AlignCenter );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 6, 6, 6, 6 );
  layout->addWidget( titleLabel );
  layout->addLayout( latLayout );
  layout->addLayout( lonLayout );

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

void QgsTopocentricWidget::setLatitude( double latitude )
{
  doubleSpinBoxX->setValue( latitude );
}

void QgsTopocentricWidget::setLongitude( double longitude )
{
  doubleSpinBoxY->setValue( longitude );
}
