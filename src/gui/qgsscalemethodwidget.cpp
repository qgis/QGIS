/***************************************************************************
                              qgsscalemethodwidget.cpp
                              ------------------------
  begin                : March 2025
  copyright            : (C) 2025 by Nyall Dawson
  email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscalemethodwidget.h"
#include "moc_qgsscalemethodwidget.cpp"
#include <QHBoxLayout>

QgsScaleMethodWidget::QgsScaleMethodWidget( QWidget *parent )
  : QWidget( parent )
{
  mCombo = new QComboBox();
  mCombo->setSizeAdjustPolicy( QComboBox::AdjustToMinimumContentsLengthWithIcon );

  mCombo->addItem( tr( "Average Top, Middle and Bottom Scales" ), QVariant::fromValue( Qgis::ScaleCalculationMethod::HorizontalAverage ) );
  mCombo->addItem( tr( "Calculate along Top of Map" ), QVariant::fromValue( Qgis::ScaleCalculationMethod::HorizontalTop ) );
  mCombo->addItem( tr( "Calculate along Middle of Map" ), QVariant::fromValue( Qgis::ScaleCalculationMethod::HorizontalMiddle ) );
  mCombo->addItem( tr( "Calculate along Bottom of Map" ), QVariant::fromValue( Qgis::ScaleCalculationMethod::HorizontalBottom ) );

  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->setContentsMargins( 0, 0, 0, 0 );
  hLayout->addWidget( mCombo );
  setLayout( hLayout );

  setFocusPolicy( Qt::FocusPolicy::StrongFocus );
  setFocusProxy( mCombo );

  connect( mCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsScaleMethodWidget::methodChanged );
}

Qgis::ScaleCalculationMethod QgsScaleMethodWidget::scaleMethod() const
{
  return mCombo->currentData().value< Qgis::ScaleCalculationMethod >();
}

void QgsScaleMethodWidget::setScaleMethod( Qgis::ScaleCalculationMethod method )
{
  mCombo->setCurrentIndex( mCombo->findData( QVariant::fromValue( method ) ) );
}
