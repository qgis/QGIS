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
  setObjectName( QStringLiteral( "mOptsPage_Elevation" ) );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );

  mPointStyleComboBox->addItem( tr( "Square" ), static_cast< int >( Qgis::PointCloudSymbol::Square ) );
  mPointStyleComboBox->addItem( tr( "Circle" ), static_cast< int >( Qgis::PointCloudSymbol::Circle ) );
  mPointSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                  << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  mMaxErrorUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                 << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mMaxErrorSpinBox->setClearValue( 0.3 );

  mPointSizeSpinBox->setClearValue( 1.0 );

  mPointColorButton->setAllowOpacity( true );
  mPointColorButton->setColorDialogTitle( tr( "Point Color" ) );

  syncToLayer( layer );

  connect( mOffsetZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mShifPointCloudZAxisButton, &QPushButton::clicked, this, &QgsPointCloudElevationPropertiesWidget::shiftPointCloudZAxis );
  connect( mPointSizeSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mPointSizeUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mMaxErrorSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mMaxErrorUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mPointStyleComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mPointColorButton, &QgsColorButton::colorChanged, this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mCheckBoxRespectLayerColors, &QCheckBox::toggled, this, &QgsPointCloudElevationPropertiesWidget::onChanged );
  connect( mOpacityByDistanceCheckBox, &QCheckBox::toggled, this, &QgsPointCloudElevationPropertiesWidget::onChanged );
}

void QgsPointCloudElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast< QgsPointCloudLayer * >( layer );
  if ( !mLayer )
    return;

  const QgsPointCloudLayerElevationProperties *properties = qgis::down_cast< const QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() );

  mBlockUpdates = true;
  mOffsetZSpinBox->setValue( properties->zOffset() );
  mScaleZSpinBox->setValue( properties->zScale() );
  mPointSizeSpinBox->setValue( properties->pointSize() );
  mPointSizeUnitWidget->setUnit( properties->pointSizeUnit() );
  mPointStyleComboBox->setCurrentIndex( mPointStyleComboBox->findData( static_cast< int >( properties->pointSymbol() ) ) );
  mMaxErrorSpinBox->setValue( properties->maximumScreenError() );
  mMaxErrorUnitWidget->setUnit( properties->maximumScreenErrorUnit() );
  mPointColorButton->setColor( properties->pointColor() );
  mCheckBoxRespectLayerColors->setChecked( properties->respectLayerColors() );
  mOpacityByDistanceCheckBox->setChecked( properties->applyOpacityByDistanceEffect() );

  mBlockUpdates = false;
}

void QgsPointCloudElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  QgsPointCloudLayerElevationProperties *properties = qgis::down_cast< QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() );

  const bool changed3DrelatedProperties = !qgsDoubleNear( mOffsetZSpinBox->value(), properties->zOffset() )
                                          || !qgsDoubleNear( mScaleZSpinBox->value(), properties->zScale() );

  properties->setZOffset( mOffsetZSpinBox->value() );
  properties->setZScale( mScaleZSpinBox->value() );
  properties->setPointSize( mPointSizeSpinBox->value() );
  properties->setPointSizeUnit( mPointSizeUnitWidget->unit() );
  properties->setPointSymbol( static_cast< Qgis::PointCloudSymbol >( mPointStyleComboBox->currentData().toInt() ) );
  properties->setMaximumScreenError( mMaxErrorSpinBox->value() );
  properties->setMaximumScreenErrorUnit( mMaxErrorUnitWidget->unit() );
  properties->setPointColor( mPointColorButton->color() );
  properties->setRespectLayerColors( mCheckBoxRespectLayerColors->isChecked() );
  properties->setApplyOpacityByDistanceEffect( mOpacityByDistanceCheckBox->isChecked() );

  if ( changed3DrelatedProperties )
    mLayer->trigger3DUpdate();
}

void QgsPointCloudElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}

void QgsPointCloudElevationPropertiesWidget::shiftPointCloudZAxis()
{
  const QgsDoubleRange range = mLayer->elevationProperties()->calculateZRange( mLayer );
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

