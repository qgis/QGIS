/***************************************************************************
    qgsrasterelevationpropertieswidget.cpp
    ---------------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterelevationpropertieswidget.h"
#include "qgsstyle.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerelevationproperties.h"

QgsRasterElevationPropertiesWidget::QgsRasterElevationPropertiesWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );
  mElevationGroupBox->setChecked( false );

  syncToLayer( layer );

  connect( mOffsetZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mElevationGroupBox, &QGroupBox::toggled, this, &QgsRasterElevationPropertiesWidget::onChanged );
}

void QgsRasterElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast< QgsRasterLayer * >( layer );
  if ( !mLayer )
    return;

  mBlockUpdates = true;
  const QgsRasterLayerElevationProperties *props = qgis::down_cast< const QgsRasterLayerElevationProperties * >( mLayer->elevationProperties() );
  mElevationGroupBox->setChecked( props->isEnabled() );
  mOffsetZSpinBox->setValue( props->zOffset() );
  mScaleZSpinBox->setValue( props->zScale() );
  mBlockUpdates = false;
}

void QgsRasterElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  QgsRasterLayerElevationProperties *props = qgis::down_cast< QgsRasterLayerElevationProperties * >( mLayer->elevationProperties() );
  props->setEnabled( mElevationGroupBox->isChecked() );
  props->setZOffset( mOffsetZSpinBox->value() );
  props->setZScale( mScaleZSpinBox->value() );
  mLayer->trigger3DUpdate();
}

void QgsRasterElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}


//
// QgsRasterElevationPropertiesWidgetFactory
//

QgsRasterElevationPropertiesWidgetFactory::QgsRasterElevationPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ) );
  setTitle( tr( "Elevation" ) );
}

QgsMapLayerConfigWidget *QgsRasterElevationPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsRasterElevationPropertiesWidget( qobject_cast< QgsRasterLayer * >( layer ), canvas, parent );
}

bool QgsRasterElevationPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsRasterElevationPropertiesWidgetFactory::supportsStyleDock() const
{
  return false;
}

bool QgsRasterElevationPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == QgsMapLayerType::RasterLayer;
}

QString QgsRasterElevationPropertiesWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}

