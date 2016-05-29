/***************************************************************************
                         qgsstatusbarmagnifierwidget.cpp
    begin                : April 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>

#include <qgsapplication.h>
#include "qgsstatusbarmagnifierwidget.h"
#include "qgsdoublespinbox.h"

QgsStatusBarMagnifierWidget::QgsStatusBarMagnifierWidget( QWidget* parent )
    : QWidget( parent )
{
  QSettings settings;
  int minimumFactor = ( int ) 100 * settings.value( "/qgis/magnifier_factor_min", 0.1 ).toDouble();
  int maximumFactor = ( int ) 100 * settings.value( "/qgis/magnifier_factor_max", 10 ).toDouble();
  int defaultFactor = ( int ) 100 * settings.value( "/qgis/magnifier_factor_default", 1.0 ).toDouble();

  // label
  mLabel = new QLabel();
  mLabel->setMinimumWidth( 10 );
  mLabel->setMargin( 3 );
  mLabel->setAlignment( Qt::AlignCenter );
  mLabel->setFrameStyle( QFrame::NoFrame );
  mLabel->setText( tr( "Magnifier" ) );
  mLabel->setToolTip( tr( "Magnifier" ) );

  mSpinBox = new QgsDoubleSpinBox();
  mSpinBox->setSuffix( "%" );
  mSpinBox->setKeyboardTracking( false );
  mSpinBox->setMaximumWidth( 120 );
  mSpinBox->setDecimals( 0 );
  mSpinBox->setRange( minimumFactor, maximumFactor );
  mSpinBox->setWrapping( false );
  mSpinBox->setSingleStep( 50 );
  mSpinBox->setToolTip( tr( "Magnifier level" ) );
  mSpinBox->setClearValueMode( QgsDoubleSpinBox::CustomValue );
  mSpinBox->setClearValue( defaultFactor );

  connect( mSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( setMagnification( double ) ) );

  // layout
  mLayout = new QHBoxLayout( this );
  mLayout->addWidget( mLabel );
  mLayout->addWidget( mSpinBox );
  mLayout->setContentsMargins( 0, 0, 0, 0 );
  mLayout->setAlignment( Qt::AlignRight );
  mLayout->setSpacing( 0 );

  setLayout( mLayout );
}

QgsStatusBarMagnifierWidget::~QgsStatusBarMagnifierWidget()
{
}

void QgsStatusBarMagnifierWidget::setDefaultFactor( double factor )
{
  mSpinBox->setClearValue(( int )100*factor );
}

void QgsStatusBarMagnifierWidget::setFont( const QFont& myFont )
{
  mLabel->setFont( myFont );
  mSpinBox->setFont( myFont );
}

void QgsStatusBarMagnifierWidget::updateMagnification( double factor )
{
  mSpinBox->setValue( factor * 100 );
}

void QgsStatusBarMagnifierWidget::setMagnification( double value )
{
  emit magnificationChanged( value / 100 );
}
