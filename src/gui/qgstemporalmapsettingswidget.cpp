/***************************************************************************
                         qgstemporalmapsettingswidget.cpp
                         ---------------
    begin                : March 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstemporalmapsettingswidget.h"
#include "qgsgui.h"
#include "qgis.h"

QgsTemporalMapSettingsWidget::QgsTemporalMapSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( mFrameSpinBox,  qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsTemporalMapSettingsWidget::frameRateChange );
}

double QgsTemporalMapSettingsWidget::frameRateValue()
{
  return mFrameSpinBox->value();
}

void QgsTemporalMapSettingsWidget::setFrameRateValue( double value )
{
  mFrameSpinBox->setValue( value );
}

void QgsTemporalMapSettingsWidget::frameRateChange()
{
  emit frameRateChanged();
}
