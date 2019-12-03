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
#include "qgsexpressioncontextutils.h"

QgsLabelObstacleSettingsWidget::QgsLabelObstacleSettingsWidget( QWidget *parent, QgsVectorLayer *vl )
  : QgsLabelSettingsWidgetBase( parent, vl )
{
  setupUi( this );

  mObstacleTypeComboBox->addItem( tr( "Over the Feature's Interior" ), QgsLabelObstacleSettings::PolygonInterior );
  mObstacleTypeComboBox->addItem( tr( "Over the Feature's Boundary" ), QgsLabelObstacleSettings::PolygonBoundary );

  connect( mObstacleTypeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    if ( !mBlockSignals )
      emit changed();
  } );
  connect( mObstacleFactorSlider, &QSlider::valueChanged, this, [ = ]
  {
    if ( !mBlockSignals )
      emit changed();
  } );
}

void QgsLabelObstacleSettingsWidget::setObstacleSettings( const QgsLabelObstacleSettings &settings )
{
  mBlockSignals = true;
  mObstacleFactorSlider->setValue( static_cast< int >( std::round( settings.factor() * 5 ) ) );
  mObstacleTypeComboBox->setCurrentIndex( mObstacleTypeComboBox->findData( settings.type() ) );
  mBlockSignals = false;
}

QgsLabelObstacleSettings QgsLabelObstacleSettingsWidget::settings() const
{
  QgsLabelObstacleSettings settings;
  settings.setFactor( mObstacleFactorSlider->value() / 5.0 );
  settings.setType( static_cast< QgsLabelObstacleSettings::ObstacleType >( mObstacleTypeComboBox->currentData().toInt() ) );
  return settings;
}

void QgsLabelObstacleSettingsWidget::setGeometryType( QgsWkbTypes::GeometryType type )
{
  mObstacleTypeComboBox->setVisible( type == QgsWkbTypes::PolygonGeometry );
  mObstacleTypeComboBox->setVisible( type == QgsWkbTypes::PolygonGeometry );
}
