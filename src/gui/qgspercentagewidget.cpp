/***************************************************************************
    qgspercentagewidget.h
     -----------------
    Date                 : January 2024
    Copyright            : (C) 2024 Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspercentagewidget.h"
#include "moc_qgspercentagewidget.cpp"
#include "qgsdoublespinbox.h"
#include "qgis.h"
#include <QHBoxLayout>
#include <QSlider>

QgsPercentageWidget::QgsPercentageWidget( QWidget *parent )
  : QWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 3 );
  setLayout( layout );

  mSlider = new QSlider();
  mSlider->setMinimum( 0 );
  mSlider->setMaximum( 1000 );
  mSlider->setSingleStep( 10 );
  mSlider->setPageStep( 100 );
  mSlider->setValue( 1000 );
  mSlider->setOrientation( Qt::Horizontal );
  layout->addWidget( mSlider, 1 );

  mSpinBox = new QgsDoubleSpinBox();
  mSpinBox->setMinimum( 0.0 );
  mSpinBox->setMaximum( 100.0 );
  mSpinBox->setValue( 100.0 );
  mSpinBox->setClearValue( 100.0 );
  mSpinBox->setMinimumSize( QSize( 100, 0 ) );
  mSpinBox->setDecimals( 1 );
  mSpinBox->setSuffix( tr( " %" ) );
  layout->addWidget( mSpinBox, 0 );

  setFocusProxy( mSpinBox );

  connect( mSlider, &QSlider::valueChanged, this, [=]( int value ) { mSpinBox->setValue( value / 10.0 ); } );
  connect( mSpinBox, static_cast<void ( QgsDoubleSpinBox::* )( double )>( &QgsDoubleSpinBox::valueChanged ), this, [=]( double value ) { whileBlocking( mSlider )->setValue( static_cast<int>( std::lround( value * 10 ) ) ); } );
  connect( mSpinBox, static_cast<void ( QgsDoubleSpinBox::* )( double )>( &QgsDoubleSpinBox::valueChanged ), this, &QgsPercentageWidget::spinChanged );
}

double QgsPercentageWidget::value() const
{
  return mSpinBox->value() / 100.0;
}

void QgsPercentageWidget::setValue( double value )
{
  mSpinBox->setValue( value * 100.0 );
}

void QgsPercentageWidget::spinChanged( double value )
{
  emit valueChanged( value / 100.0 );
}
