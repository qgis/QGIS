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
#include "qgselevationshadingrenderersettingswidget.h"
#include "moc_qgselevationshadingrenderersettingswidget.cpp"
#include "ui_qgselevationshadingrenderersettingswidget.h"

#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgselevationshadingrenderer.h"

QgsElevationShadingRendererSettingsWidget::QgsElevationShadingRendererSettingsWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );

  mCombineMethodCombo->addItem( tr( "Highest Elevation" ), QVariant::fromValue( Qgis::ElevationMapCombineMethod::HighestElevation ) );
  mCombineMethodCombo->addItem( tr( "Based on Layer's Order" ), QVariant::fromValue( Qgis::ElevationMapCombineMethod::NewerElevation ) );

  syncToProject();

  connect( QgsProject::instance(), &QgsProject::elevationShadingRendererChanged, this, &QgsElevationShadingRendererSettingsWidget::syncToProject );

  connect( mShadingGroupBox, &QGroupBox::toggled, this, &QgsElevationShadingRendererSettingsWidget::onChanged );

  connect( mCombineMethodCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsElevationShadingRendererSettingsWidget::onChanged );

  mEdlDistanceSpinBox->setClearValue( 0.5 );
  mEdlStrengthSpinBox->setClearValue( 1000 );
  connect( mEdlGroupBox, &QGroupBox::toggled, this, &QgsElevationShadingRendererSettingsWidget::onChanged );
  connect( mEdlStrengthSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsElevationShadingRendererSettingsWidget::onChanged );
  connect( mEdlDistanceSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsElevationShadingRendererSettingsWidget::onChanged );
  connect( mEdlDistanceUnit, &QgsUnitSelectionWidget::changed, this, &QgsElevationShadingRendererSettingsWidget::onChanged );

  connect( mHillshadingGroupBox, &QGroupBox::toggled, this, &QgsElevationShadingRendererSettingsWidget::onChanged );
  mHillshadingZFactorSpinBox->setClearValue( 1.0 );
  connect( mHillshadingZFactorSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsElevationShadingRendererSettingsWidget::onChanged );
  connect( mHillshadingMultidirCheckBox, &QCheckBox::toggled, this, &QgsElevationShadingRendererSettingsWidget::onChanged );

  connect( mDirectionalLightWidget, &QgsDirectionalLightWidget::directionChanged, this, &QgsElevationShadingRendererSettingsWidget::onChanged );
}

void QgsElevationShadingRendererSettingsWidget::apply()
{
  QgsElevationShadingRenderer shadingRenderer;

  shadingRenderer.setActive( mShadingGroupBox->isChecked() );
  shadingRenderer.setCombinedElevationMethod( mCombineMethodCombo->currentData().value<Qgis::ElevationMapCombineMethod>() );
  shadingRenderer.setActiveEyeDomeLighting( mEdlGroupBox->isChecked() );
  shadingRenderer.setEyeDomeLightingStrength( mEdlStrengthSpinBox->value() );
  shadingRenderer.setEyeDomeLightingDistance( mEdlDistanceSpinBox->value() );
  shadingRenderer.setEyeDomeLightingDistanceUnit( mEdlDistanceUnit->unit() );
  shadingRenderer.setActiveHillshading( mHillshadingGroupBox->isChecked() );
  shadingRenderer.setHillshadingMultidirectional( mHillshadingMultidirCheckBox->isChecked() );
  shadingRenderer.setHillshadingZFactor( mHillshadingZFactorSpinBox->value() );

  shadingRenderer.setLightAltitude( mDirectionalLightWidget->altitude() );
  shadingRenderer.setLightAzimuth( mDirectionalLightWidget->azimuth() );

  QgsProject::instance()->setElevationShadingRenderer( shadingRenderer );
}

void QgsElevationShadingRendererSettingsWidget::syncToProject()
{
  mBlockUpdates = true;
  QgsElevationShadingRenderer shadingRenderer = QgsProject::instance()->elevationShadingRenderer();
  mShadingGroupBox->setChecked( shadingRenderer.isActive() );
  mCombineMethodCombo->setCurrentIndex(
    mCombineMethodCombo->findData( QVariant::fromValue( shadingRenderer.combinedElevationMethod() ) )
  );
  mEdlGroupBox->setChecked( shadingRenderer.isActiveEyeDomeLighting() );
  mEdlStrengthSpinBox->setValue( shadingRenderer.eyeDomeLightingStrength() );
  mEdlDistanceSpinBox->setValue( shadingRenderer.eyeDomeLightingDistance() );
  mEdlDistanceUnit->setUnits(
    { Qgis::RenderUnit::Millimeters,
      Qgis::RenderUnit::MetersInMapUnits,
      Qgis::RenderUnit::MapUnits,
      Qgis::RenderUnit::Pixels,
      Qgis::RenderUnit::Points,
      Qgis::RenderUnit::Inches
    }
  );
  mEdlDistanceUnit->setUnit( shadingRenderer.eyeDomeLightingDistanceUnit() );
  mHillshadingGroupBox->setChecked( shadingRenderer.isActiveHillshading() );
  mHillshadingMultidirCheckBox->setChecked( shadingRenderer.isHillshadingMultidirectional() );
  mHillshadingZFactorSpinBox->setValue( shadingRenderer.hillshadingZFactor() );

  mDirectionalLightWidget->setAltitude( shadingRenderer.lightAltitude() );
  mDirectionalLightWidget->setAzimuth( shadingRenderer.lightAzimuth() );
  mDirectionalLightWidget->setEnableAzimuth( !mHillshadingMultidirCheckBox->isChecked() );
  mBlockUpdates = false;
}

void QgsElevationShadingRendererSettingsWidget::onChanged()
{
  mDirectionalLightWidget->setEnableAzimuth( !mHillshadingMultidirCheckBox->isChecked() );

  if ( !mBlockUpdates )
    emit widgetChanged();
}

QgsElevationShadingRendererSettingsWidgetFactory::QgsElevationShadingRendererSettingsWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mShadingRenderer.svg" ) ) );
  setTitle( tr( "Shading Renderer" ) );
}

QgsMapLayerConfigWidget *QgsElevationShadingRendererSettingsWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsElevationShadingRendererSettingsWidget( layer, canvas, parent );
}
