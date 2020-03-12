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

///@cond PRIVATE
QgsTemporalMapSettingsWidget::QgsTemporalMapSettingsWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  setPanelTitle( tr( "Temporal Settings" ) );

  mFrameSpinBox->setClearValue( 1 );

  connect( mFrameSpinBox,  qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsTemporalMapSettingsWidget::frameRateChanged );
}

double QgsTemporalMapSettingsWidget::frameRateValue()
{
  return mFrameSpinBox->value();
}

void QgsTemporalMapSettingsWidget::setFrameRateValue( double value )
{
  mFrameSpinBox->setValue( value );
}

///@endcond
