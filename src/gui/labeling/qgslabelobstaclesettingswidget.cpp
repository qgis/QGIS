/***************************************************************************
    qgslabelobstaclesettingswidget.h
    ----------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgslabelobstaclesettingswidget.h"
#include "moc_qgslabelobstaclesettingswidget.cpp"
#include "qgsexpressioncontextutils.h"

QgsLabelObstacleSettingsWidget::QgsLabelObstacleSettingsWidget( QWidget *parent, QgsVectorLayer *vl )
  : QgsLabelSettingsWidgetBase( parent, vl )
{
  setupUi( this );

  setPanelTitle( tr( "Obstacle Settings" ) );

  mObstacleTypeComboBox->addItem( tr( "Over the Feature's Interior" ), static_cast<int>( QgsLabelObstacleSettings::ObstacleType::PolygonInterior ) );
  mObstacleTypeComboBox->addItem( tr( "Over the Feature's Boundary" ), static_cast<int>( QgsLabelObstacleSettings::ObstacleType::PolygonBoundary ) );

  connect( mObstacleTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=]( int ) {
    if ( !mBlockSignals )
      emit changed();
  } );
  connect( mObstacleFactorSlider, &QSlider::valueChanged, this, [=] {
    if ( !mBlockSignals )
      emit changed();
  } );

  registerDataDefinedButton( mObstacleFactorDDBtn, QgsPalLayerSettings::Property::ObstacleFactor );
}

void QgsLabelObstacleSettingsWidget::setSettings( const QgsLabelObstacleSettings &settings )
{
  mBlockSignals = true;
  mObstacleFactorSlider->setValue( static_cast<int>( std::round( settings.factor() * 5 ) ) );
  mObstacleTypeComboBox->setCurrentIndex( mObstacleTypeComboBox->findData( static_cast<int>( settings.type() ) ) );
  mBlockSignals = false;
}

QgsLabelObstacleSettings QgsLabelObstacleSettingsWidget::settings() const
{
  QgsLabelObstacleSettings settings;
  settings.setFactor( mObstacleFactorSlider->value() / 5.0 );
  settings.setType( static_cast<QgsLabelObstacleSettings::ObstacleType>( mObstacleTypeComboBox->currentData().toInt() ) );
  return settings;
}

void QgsLabelObstacleSettingsWidget::setGeometryType( Qgis::GeometryType type )
{
  mObstacleTypeComboBox->setVisible( type == Qgis::GeometryType::Polygon || type == Qgis::GeometryType::Unknown );
  mObstacleTypeLabel->setVisible( type == Qgis::GeometryType::Polygon || type == Qgis::GeometryType::Unknown );
}

void QgsLabelObstacleSettingsWidget::updateDataDefinedProperties( QgsPropertyCollection &properties )
{
  properties.setProperty( QgsPalLayerSettings::Property::ObstacleFactor, mDataDefinedProperties.property( QgsPalLayerSettings::Property::ObstacleFactor ) );
}
