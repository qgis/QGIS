/***************************************************************************
                qgsprocessingrastersourceoptionswidget.cpp
                       --------------------------
    begin                : August 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingrastersourceoptionswidget.h"
#include "moc_qgsprocessingrastersourceoptionswidget.cpp"

///@cond NOT_STABLE

QgsProcessingRasterSourceOptionsWidget::QgsProcessingRasterSourceOptionsWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mDpiSpinBox->setClearValue( 0, tr( "Not set" ) );
  mDpiSpinBox->setValue( 96 );

  connect( mReferenceScale, qOverload<double>( &QgsScaleWidget::scaleChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mDpiSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
}

void QgsProcessingRasterSourceOptionsWidget::setReferenceScale( long scale )
{
  mReferenceScale->setScale( static_cast<double>( scale ) );
}

void QgsProcessingRasterSourceOptionsWidget::setDpi( int dpi )
{
  mDpiSpinBox->setValue( dpi );
}

long QgsProcessingRasterSourceOptionsWidget::referenceScale() const
{
  return static_cast< long >( mReferenceScale->scale() );
}

int QgsProcessingRasterSourceOptionsWidget::dpi() const
{
  return mDpiSpinBox->value();
}


///@endcond
