/***************************************************************************
    qgstiledsceneelevationpropertieswidget.cpp
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledsceneelevationpropertieswidget.h"
#include "moc_qgstiledsceneelevationpropertieswidget.cpp"
#include "qgstiledscenerendererpropertieswidget.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenelayerelevationproperties.h"

QgsTiledSceneElevationPropertiesWidget::QgsTiledSceneElevationPropertiesWidget( QgsTiledSceneLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );
  setObjectName( QStringLiteral( "mOptsPage_Elevation" ) );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );

  syncToLayer( layer );

  connect( mOffsetZSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsTiledSceneElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsTiledSceneElevationPropertiesWidget::onChanged );
  connect( mShiftZAxisButton, &QPushButton::clicked, this, &QgsTiledSceneElevationPropertiesWidget::shiftSceneZAxis );

  // setProperty( "helpPage", QStringLiteral( "working_with_point_clouds/point_clouds.html#elevation-properties" ) );
}

void QgsTiledSceneElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast<QgsTiledSceneLayer *>( layer );
  if ( !mLayer )
    return;

  const QgsTiledSceneLayerElevationProperties *properties = qgis::down_cast<const QgsTiledSceneLayerElevationProperties *>( mLayer->elevationProperties() );

  mBlockUpdates = true;
  mOffsetZSpinBox->setValue( properties->zOffset() );
  mScaleZSpinBox->setValue( properties->zScale() );

  mBlockUpdates = false;
}

void QgsTiledSceneElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  QgsTiledSceneLayerElevationProperties *properties = qgis::down_cast<QgsTiledSceneLayerElevationProperties *>( mLayer->elevationProperties() );

  const bool changed3DrelatedProperties = !qgsDoubleNear( mOffsetZSpinBox->value(), properties->zOffset() )
                                          || !qgsDoubleNear( mScaleZSpinBox->value(), properties->zScale() );

  properties->setZOffset( mOffsetZSpinBox->value() );
  properties->setZScale( mScaleZSpinBox->value() );

  if ( changed3DrelatedProperties )
    mLayer->trigger3DUpdate();
}

void QgsTiledSceneElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}

void QgsTiledSceneElevationPropertiesWidget::shiftSceneZAxis()
{
  const QgsDoubleRange range = mLayer->elevationProperties()->calculateZRange( mLayer );
  if ( !range.isEmpty() )
  {
    mOffsetZSpinBox->setValue( -range.lower() + mOffsetZSpinBox->value() );
  }
}

//
// QgsTiledSceneElevationPropertiesWidgetFactory
//

QgsTiledSceneElevationPropertiesWidgetFactory::QgsTiledSceneElevationPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ) );
  setTitle( tr( "Elevation" ) );
}

QgsMapLayerConfigWidget *QgsTiledSceneElevationPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsTiledSceneElevationPropertiesWidget( qobject_cast<QgsTiledSceneLayer *>( layer ), canvas, parent );
}

bool QgsTiledSceneElevationPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsTiledSceneElevationPropertiesWidgetFactory::supportsStyleDock() const
{
  return true;
}

bool QgsTiledSceneElevationPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::TiledScene;
}

QString QgsTiledSceneElevationPropertiesWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}
