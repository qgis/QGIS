/***************************************************************************
    qgsopacitywidget.cpp
     -------------------
    Date                 : May 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsopacitywidget.h"
#include "qgsdoublespinbox.h"
#include "qgis.h"
#include <QHBoxLayout>
#include <QSlider>

QgsOpacityWidget::QgsOpacityWidget( QWidget *parent )
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

  connect( mSlider, &QSlider::valueChanged, this, [ = ]( int value ) { mSpinBox->setValue( value / 10.0 ); } );
  connect( mSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double value ) { whileBlocking( mSlider )->setValue( value * 10 ); } );
  connect( mSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this,  &QgsOpacityWidget::spinChanged );
}

double QgsOpacityWidget::opacity() const
{
  return mSpinBox->value() / 100.0;
}

void QgsOpacityWidget::setOpacity( double opacity )
{
  mSpinBox->setValue( opacity * 100.0 );
}

void QgsOpacityWidget::spinChanged( double value )
{
  emit opacityChanged( value / 100.0 );
}

