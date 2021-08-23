/***************************************************************************
  qgsshadowrenderingsettingswidget.cpp
  --------------------------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb uderscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsshadowrenderingsettingswidget.h"

#include <QCheckBox>
#include <QLineEdit>
#include "qgs3dmapsettings.h"
#include "qgis.h"

QgsShadowRenderingSettingsWidget::QgsShadowRenderingSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  shadowRenderinMaximumDistanceSpinBox->setClearValue( 1500.00 );
  shadowBiasSpinBox->setClearValue( 0.000010 );
  shadowMapResolutionSpinBox->setClearValue( 2048 );
}

void QgsShadowRenderingSettingsWidget::setShadowSettings( const QgsShadowSettings &shadowSettings )
{
  usedDirectionalLightComboBox->setCurrentIndex( shadowSettings.selectedDirectionalLight() );
  shadowRenderinMaximumDistanceSpinBox->setValue( shadowSettings.maximumShadowRenderingDistance() );
  shadowBiasSpinBox->setValue( shadowSettings.shadowBias() );
  shadowMapResolutionSpinBox->setValue( shadowSettings.shadowMapResolution() );
}

QgsShadowSettings QgsShadowRenderingSettingsWidget::toShadowSettings()
{
  QgsShadowSettings settings;
  settings.setSelectedDirectionalLight( usedDirectionalLightComboBox->currentIndex() );
  settings.setMaximumShadowRenderingDistance( shadowRenderinMaximumDistanceSpinBox->value() );
  settings.setShadowBias( shadowBiasSpinBox->value() );
  settings.setShadowMapResolution( shadowMapResolutionSpinBox->value() );
  return settings;
}

void QgsShadowRenderingSettingsWidget::onDirectionalLightsCountChanged( int newCount )
{
  const int previousItemIndex = usedDirectionalLightComboBox->currentIndex();
  while ( usedDirectionalLightComboBox->count() < newCount )
    usedDirectionalLightComboBox->addItem( tr( "Directional light %1" ).arg( usedDirectionalLightComboBox->count() + 1 ) );
  while ( usedDirectionalLightComboBox->count() > newCount )
    usedDirectionalLightComboBox->removeItem( usedDirectionalLightComboBox->count() - 1 );
  if ( previousItemIndex < 0 || previousItemIndex >= usedDirectionalLightComboBox->count() )
    usedDirectionalLightComboBox->setCurrentIndex( 0 );
  else
    usedDirectionalLightComboBox->setCurrentIndex( previousItemIndex );
}
