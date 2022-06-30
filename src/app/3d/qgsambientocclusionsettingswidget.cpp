/***************************************************************************
  qgsambientocclusionsettingswidget.cpp
  --------------------------------------
  Date                 : June 2022
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

#include "qgsambientocclusionsettingswidget.h"

QgsAmbientOcclusionSettingsWidget::QgsAmbientOcclusionSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mShadingFactorSpinBox->setToolTip( tr( "The strength of the shading applied, bigger values means more pronounced and daraker colors." ) );
  mDistanceAttenuationFactorSpinBox->setToolTip( tr( "The contributions of farther points into the sampling: bigger values mean lines will be graduated more." ) );
  mRadiusParameterSpinBox->setToolTip( tr( "The radius of the sampled sphere: bigger values mean thicker edges" ) );
}

void QgsAmbientOcclusionSettingsWidget::setAmbientOcclusionSettings( const QgsAmbientOcclusionSettings &settings )
{
  mAmbientOcclusionGroupBox->setChecked( settings.ambientOcclusionEnabled() );
  mAmbientOcclusionBlurCheckbox->setChecked( settings.blurringEnabled() );
  mShadingFactorSpinBox->setValue( settings.shadingFactor() );
  mDistanceAttenuationFactorSpinBox->setValue( settings.distanceAttenuationFactor() );
  mRadiusParameterSpinBox->setValue( settings.radiusParameter() );
}

QgsAmbientOcclusionSettings QgsAmbientOcclusionSettingsWidget::toAmbientOcclusionSettings()
{
  QgsAmbientOcclusionSettings settings;
  settings.setAmbientOcclusionEnabled( mAmbientOcclusionGroupBox->isChecked() );
  settings.setBlurringEnabled( mAmbientOcclusionBlurCheckbox->isChecked() );
  settings.setShadingFactor( mShadingFactorSpinBox->value() );
  settings.setDistanceAttenuationFactor( mDistanceAttenuationFactorSpinBox->value() );
  settings.setRadiusParameter( mRadiusParameterSpinBox->value() );
  return settings;
}
