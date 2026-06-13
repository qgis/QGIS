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
  mDoubleSpinBoxX = new QgsDoubleSpinBox();
  mHorizontalSliderX = new QSlider( Qt::Horizontal );
  mDoubleSpinBoxY = new QgsDoubleSpinBox();
  mHorizontalSliderY = new QSlider( Qt::Horizontal );

  QLabel *titleLabel = new QLabel( tr( "Topocentric origin:" ) );
  titleLabel->setAlignment( Qt::AlignCenter );

  QGridLayout *grid = new QGridLayout();
  grid->addWidget( new QLabel( tr( "Latitude" ) ), 0, 0 );
  grid->addWidget( mHorizontalSliderX, 0, 1 );
  grid->addWidget( mDoubleSpinBoxX, 0, 2 );
  grid->addWidget( new QLabel( tr( "Longitude" ) ), 1, 0 );
  grid->addWidget( mHorizontalSliderY, 1, 1 );
  grid->addWidget( mDoubleSpinBoxY, 1, 2 );
  grid->setColumnStretch( 1, 1 );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 6, 6, 6, 6 );
  layout->addWidget( titleLabel );
  layout->addLayout( grid );

  mDoubleSpinBoxX->setRange( -90.0, 90.0 );
  mDoubleSpinBoxX->setDecimals( 1 );
  mHorizontalSliderX->setRange( -90, 90 );

  mDoubleSpinBoxY->setRange( -180.0, 180.0 );
  mDoubleSpinBoxY->setDecimals( 1 );
  mHorizontalSliderY->setRange( -180, 180 );

  mEditTimer = new QTimer( this );
  mEditTimer->setSingleShot( true );
  mEditTimer->setInterval( 250 );
  connect( mEditTimer, &QTimer::timeout, this, [this]() { emit originChanged( latitude(), longitude() ); } );

  connect( mHorizontalSliderX, &QSlider::valueChanged, this, [this]( int v ) {
    whileBlocking( mDoubleSpinBoxX )->setValue( static_cast<double>( v ) );
    mEditTimer->start();
  } );

  connect( mDoubleSpinBoxX, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [this]( double v ) {
    whileBlocking( mHorizontalSliderX )->setValue( static_cast<int>( v ) );
    emit originChanged( latitude(), longitude() );
  } );

  connect( mHorizontalSliderY, &QSlider::valueChanged, this, [this]( int v ) {
    whileBlocking( mDoubleSpinBoxY )->setValue( static_cast<double>( v ) );
    mEditTimer->start();
  } );

  connect( mDoubleSpinBoxY, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [this]( double v ) {
    whileBlocking( mHorizontalSliderY )->setValue( static_cast<int>( v ) );
    emit originChanged( latitude(), longitude() );
  } );
}

double QgsTopocentricWidget::latitude() const
{
  return mDoubleSpinBoxX->value();
}

double QgsTopocentricWidget::longitude() const
{
  return mDoubleSpinBoxY->value();
}

void QgsTopocentricWidget::setLatitude( double latitude )
{
  mDoubleSpinBoxX->setValue( latitude );
}

void QgsTopocentricWidget::setLongitude( double longitude )
{
  mDoubleSpinBoxY->setValue( longitude );
}
