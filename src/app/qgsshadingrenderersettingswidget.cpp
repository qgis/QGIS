/***************************************************************************
  qgsshadingrenderersettingswidget.cpp - QgsShadingRendererSettingsWidget

 ---------------------
 begin                : 12.12.2022
 copyright            : (C) 2022 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsshadingrenderersettingswidget.h"
#include "ui_qgsshadingrenderersettingswidget.h"

#include "qgsproject.h"
#include "qgsshadingrenderer.h"

QgsShadingRendererSettingsWidget::QgsShadingRendererSettingsWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent ) :
  QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );

  syncToProject();

  connect( QgsProject::instance(), &QgsProject::mapShadingRendererChanged, this, &QgsShadingRendererSettingsWidget::syncToProject );

  connect( mShadingGroupBox, &QGroupBox::toggled, this, &QgsShadingRendererSettingsWidget::onChanged );

  connect( mEdlGroupBox, &QGroupBox::toggled, this, &QgsShadingRendererSettingsWidget::onChanged );
  connect( mEdlStrengthSpinBox,  qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsShadingRendererSettingsWidget::onChanged );
  connect( mEdlDistanceSpinBox,  qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsShadingRendererSettingsWidget::onChanged );

  connect( mHillshadingGroupBox, &QGroupBox::toggled, this, &QgsShadingRendererSettingsWidget::onChanged );
  connect( mHillshadingZFactorSpinBox,  qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsShadingRendererSettingsWidget::onChanged );
  connect( mHillshadingMultidirCheckBox, &QCheckBox::toggled, this, &QgsShadingRendererSettingsWidget::onChanged );

  connect( mDirectionalLightWidget, &QgsDirectionalLightWidget::directionChanged, this, &QgsShadingRendererSettingsWidget::onChanged );
}

void QgsShadingRendererSettingsWidget::apply()
{
  QgsShadingRenderer shadingRenderer;

  shadingRenderer.setActive( mShadingGroupBox->isChecked() );
  shadingRenderer.setActiveEyeDomeLighting( mEdlGroupBox->isChecked() );
  shadingRenderer.setEyeDomeLightingStrength( mEdlStrengthSpinBox->value() );
  shadingRenderer.setEyeDomeLightingDistance( mEdlDistanceSpinBox->value() );
  shadingRenderer.setEyeDomeLightingDistanceUnit( mEdlDistanceUnit->unit() );
  shadingRenderer.setActiveHillShading( mHillshadingGroupBox->isChecked() );
  shadingRenderer.setHillShadingMultidirectional( mHillshadingMultidirCheckBox->isChecked() );
  shadingRenderer.setHillShadingZFactor( mHillshadingZFactorSpinBox->value() );

  shadingRenderer.setLightAltitude( mDirectionalLightWidget->altitude() );
  shadingRenderer.setLightAzimuth( mDirectionalLightWidget->azimuth() );

  QgsProject::instance()->setMapShadingRenderer( shadingRenderer );
}

void QgsShadingRendererSettingsWidget::syncToProject()
{
  mBlockUpdates = true;
  QgsShadingRenderer shadingRenderer = QgsProject::instance()->mapShadingRenderer();
  mShadingGroupBox->setChecked( shadingRenderer.isActive() );
  mEdlGroupBox->setChecked( shadingRenderer.isActiveEyeDomeLighting() );
  mEdlStrengthSpinBox->setValue( shadingRenderer.eyeDomeLightingStrength() );
  mEdlDistanceSpinBox->setValue( shadingRenderer.eyeDomeLightingDistance() );
  mEdlDistanceUnit->setUnits( QgsUnitTypes::RenderUnitList() <<
                              QgsUnitTypes::RenderMillimeters <<
                              QgsUnitTypes::RenderMetersInMapUnits <<
                              QgsUnitTypes::RenderMapUnits <<
                              QgsUnitTypes::RenderPixels <<
                              QgsUnitTypes::RenderPoints <<
                              QgsUnitTypes::RenderInches );
  mEdlDistanceUnit->setUnit( shadingRenderer.eyeDomeLightingDistanceUnit() );
  mHillshadingGroupBox->setChecked( shadingRenderer.isActiveHillShading() );
  mHillshadingMultidirCheckBox->setChecked( shadingRenderer.isHillShadingMultidirectional() );
  mHillshadingZFactorSpinBox->setValue( shadingRenderer.hillShadingZFactor() );

  mDirectionalLightWidget->setAltitude( shadingRenderer.lightAltitude() );
  mDirectionalLightWidget->setAzimuth( shadingRenderer.lightAzimuth() );
  mBlockUpdates = false;
}

void QgsShadingRendererSettingsWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}

QgsShadingRendererSettingsWidgetFactory::QgsShadingRendererSettingsWidgetFactory( QObject *parent ): QObject( parent )
{

}

QgsMapLayerConfigWidget *QgsShadingRendererSettingsWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const
{
  return new QgsShadingRendererSettingsWidget( layer, canvas, parent );
}
