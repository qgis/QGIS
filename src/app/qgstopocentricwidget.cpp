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

#include "qgis.h"
#include "qgsdoublespinbox.h"

#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

#include "moc_qgstopocentricwidget.cpp"

QgsTopocentricWidget::QgsTopocentricWidget( QWidget *parent )
  : QWidget( parent )
{
  mDoubleSpinBoxLat = new QgsDoubleSpinBox();
  mHorizontalSliderLat = new QSlider( Qt::Horizontal );
  mDoubleSpinBoxLon = new QgsDoubleSpinBox();
  mHorizontalSliderLon = new QSlider( Qt::Horizontal );

  QLabel *titleLabel = new QLabel( tr( "Topocentric origin:" ) );
  titleLabel->setAlignment( Qt::AlignCenter );

  QGridLayout *grid = new QGridLayout();
  grid->addWidget( new QLabel( tr( "Latitude" ) ), 0, 0 );
  grid->addWidget( mHorizontalSliderLat, 0, 1 );
  grid->addWidget( mDoubleSpinBoxLat, 0, 2 );
  grid->addWidget( new QLabel( tr( "Longitude" ) ), 1, 0 );
  grid->addWidget( mHorizontalSliderLon, 1, 1 );
  grid->addWidget( mDoubleSpinBoxLon, 1, 2 );
  grid->setColumnStretch( 1, 1 );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 6, 6, 6, 6 );
  layout->addWidget( titleLabel );
  layout->addLayout( grid );

  mDoubleSpinBoxLat->setRange( -90.0, 90.0 );
  mDoubleSpinBoxLat->setDecimals( 1 );
  mHorizontalSliderLat->setRange( -90, 90 );

  mDoubleSpinBoxLon->setRange( -180.0, 180.0 );
  mDoubleSpinBoxLon->setDecimals( 1 );
  mHorizontalSliderLon->setRange( -180, 180 );

  mEditTimer = new QTimer( this );
  mEditTimer->setSingleShot( true );
  mEditTimer->setInterval( 250 );
  connect( mEditTimer, &QTimer::timeout, this, [this]() { emit originChanged( latitude(), longitude() ); } );

  connect( mHorizontalSliderLat, &QSlider::valueChanged, this, [this]( int v ) {
    whileBlocking( mDoubleSpinBoxLat )->setValue( static_cast<double>( v ) );
    mEditTimer->start();
  } );

  connect( mDoubleSpinBoxLat, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [this]( double v ) {
    whileBlocking( mHorizontalSliderLat )->setValue( static_cast<int>( v ) );
    emit originChanged( latitude(), longitude() );
  } );

  connect( mHorizontalSliderLon, &QSlider::valueChanged, this, [this]( int v ) {
    whileBlocking( mDoubleSpinBoxLon )->setValue( static_cast<double>( v ) );
    mEditTimer->start();
  } );

  connect( mDoubleSpinBoxLon, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [this]( double v ) {
    whileBlocking( mHorizontalSliderLon )->setValue( static_cast<int>( v ) );
    emit originChanged( latitude(), longitude() );
  } );
}

double QgsTopocentricWidget::latitude() const
{
  return mDoubleSpinBoxLat->value();
}

double QgsTopocentricWidget::longitude() const
{
  return mDoubleSpinBoxLon->value();
}

void QgsTopocentricWidget::setLatitude( double latitude )
{
  mDoubleSpinBoxLat->setValue( latitude );
}

void QgsTopocentricWidget::setLongitude( double longitude )
{
  mDoubleSpinBoxLon->setValue( longitude );
}

void QgsTopocentricWidget::setDefaultOrigin( double lat, double lon )
{
  mDoubleSpinBoxLat->setClearValue( lat );
  mDoubleSpinBoxLon->setClearValue( lon );
}
