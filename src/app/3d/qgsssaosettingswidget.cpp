/***************************************************************************
  qgsssaosettingswidget.cpp
  --------------------------------------
  Date                 : Juin 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsssaosettingswidget.h"

QgsSsaoSettingsWidget::QgsSsaoSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
}

void QgsSsaoSettingsWidget::setSsaoSettings( const QgsSsaoSettings &settings )
{
  mSsaoGroupBox->setChecked( settings.ssaoEnabled() );
  mSssaBlurCheckbox->setChecked( settings.blurringEnabled() );
  mShadingFactorSpinBox->setValue( settings.shadingFactor() );
  mDistanceAttenuationFactorSpinBox->setValue( settings.distanceAttenuationFactor() );
  mRadiusParameterSpinBox->setValue( settings.radiusParameter() );
}

QgsSsaoSettings QgsSsaoSettingsWidget::toSsaoSettings()
{
  QgsSsaoSettings settings;
  settings.setSsaoEnabled( mSsaoGroupBox->isChecked() );
  settings.setBlurringEnabled( mSssaBlurCheckbox->isChecked() );
  settings.setShadingFactor( mShadingFactorSpinBox->value() );
  settings.setDistanceAttenuationFactor( mDistanceAttenuationFactorSpinBox->value() );
  settings.setRadiusParameter( mRadiusParameterSpinBox->value() );
  return settings;
}
