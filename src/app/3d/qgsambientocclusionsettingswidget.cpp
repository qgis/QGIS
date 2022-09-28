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

  mIntensitySpinBox->setToolTip( tr( "The strength of the shading applied, bigger values means more pronounced and darker colors." ) );
  mRadiusSpinBox->setToolTip( tr( "The radius of the neighborhood: bigger values mean objects further away will add to the occlusion." ) );
  mThresholdSpinBox->setToolTip( tr( "Only apply occlusion effect when at least the specified amount of neighboring points is occluded." ) );

  mIntensitySpinBox->setClearValue( 0.5 );
  mRadiusSpinBox->setClearValue( 25. );
  mThresholdSpinBox->setClearValue( 50 );
}

void QgsAmbientOcclusionSettingsWidget::setAmbientOcclusionSettings( const QgsAmbientOcclusionSettings &settings )
{
  mAmbientOcclusionGroupBox->setChecked( settings.isEnabled() );
  mIntensitySpinBox->setValue( settings.intensity() );
  mRadiusSpinBox->setValue( settings.radius() );
  mThresholdSpinBox->setValue( settings.threshold() * 100 );
}

QgsAmbientOcclusionSettings QgsAmbientOcclusionSettingsWidget::toAmbientOcclusionSettings()
{
  QgsAmbientOcclusionSettings settings;
  settings.setEnabled( mAmbientOcclusionGroupBox->isChecked() );
  settings.setIntensity( mIntensitySpinBox->value() );
  settings.setRadius( mRadiusSpinBox->value() );
  settings.setThreshold( mThresholdSpinBox->value() / 100. );
  return settings;
}
