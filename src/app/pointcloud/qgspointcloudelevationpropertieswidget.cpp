/***************************************************************************
    qgspointcloudelevationpropertieswidget.cpp
    ---------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudelevationpropertieswidget.h"
#include "qgspointcloudrendererpropertieswidget.h"
#include "qgsstyle.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerelevationproperties.h"

QgsPointCloudElevationPropertiesWidget::QgsPointCloudElevationPropertiesWidget( QgsPointCloudLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );

  syncToLayer( layer );

  connect( mOffsetZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mShifPointCloudZAxisButton, &QPushButton::clicked, this, &QgsPointCloudElevationPropertiesWidget::shiftPointCloudZAxis );
}

void QgsPointCloudElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast< QgsPointCloudLayer * >( layer );
  if ( !mLayer )
    return;

  mBlockUpdates = true;
  mOffsetZSpinBox->setValue( static_cast< const QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() )->zOffset() );
  mScaleZSpinBox->setValue( static_cast< const QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() )->zScale() );
  mBlockUpdates = false;
}

void QgsPointCloudElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  static_cast< QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() )->setZOffset( mOffsetZSpinBox->value() );
  static_cast< QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() )->setZScale( mScaleZSpinBox->value() );
  mLayer->trigger3DUpdate();
}

void QgsPointCloudElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}

void QgsPointCloudElevationPropertiesWidget::shiftPointCloudZAxis()
{
  QgsDoubleRange range = mLayer->elevationProperties()->calculateZRange( mLayer );
  if ( !range.isEmpty() )
  {
    mOffsetZSpinBox->setValue( -range.lower() + mOffsetZSpinBox->value() );
  }
}

//
// QgsPointCloudElevationPropertiesWidgetFactory
//

QgsPointCloudElevationPropertiesWidgetFactory::QgsPointCloudElevationPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ) );
  setTitle( tr( "Elevation" ) );
}

QgsMapLayerConfigWidget *QgsPointCloudElevationPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsPointCloudElevationPropertiesWidget( qobject_cast< QgsPointCloudLayer * >( layer ), canvas, parent );
}

bool QgsPointCloudElevationPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsPointCloudElevationPropertiesWidgetFactory::supportsStyleDock() const
{
  return true;
}

bool QgsPointCloudElevationPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == QgsMapLayerType::PointCloudLayer;
}

QString QgsPointCloudElevationPropertiesWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}

