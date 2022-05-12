/***************************************************************************
    qgspointcloudattributebyramprendererwidget.cpp
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudattributebyramprendererwidget.h"
#include "qgscontrastenhancement.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudattributebyramprenderer.h"
#include "qgsdoublevalidator.h"
#include "qgsstyle.h"
#include "qgspointcloudlayerelevationproperties.h"

///@cond PRIVATE

QgsPointCloudAttributeByRampRendererWidget::QgsPointCloudAttributeByRampRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style )
  : QgsPointCloudRendererWidget( layer, style )
{
  setupUi( this );

  mAttributeComboBox->setAllowEmptyAttributeName( false );
  mAttributeComboBox->setFilters( QgsPointCloudAttributeProxyModel::AllTypes );

  mMinSpin->setShowClearButton( false );
  mMaxSpin->setShowClearButton( false );

  if ( layer )
  {
    mAttributeComboBox->setLayer( layer );

    setFromRenderer( layer->renderer() );
  }

  connect( mAttributeComboBox, &QgsPointCloudAttributeComboBox::attributeChanged,
           this, &QgsPointCloudAttributeByRampRendererWidget::attributeChanged );
  connect( mMinSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloudAttributeByRampRendererWidget::minMaxChanged );
  connect( mMaxSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloudAttributeByRampRendererWidget::minMaxChanged );

  connect( mScalarColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsPointCloudAttributeByRampRendererWidget::emitWidgetChanged );
  connect( mScalarRecalculateMinMaxButton, &QPushButton::clicked, this, &QgsPointCloudAttributeByRampRendererWidget::setMinMaxFromLayer );
}

QgsPointCloudRendererWidget *QgsPointCloudAttributeByRampRendererWidget::create( QgsPointCloudLayer *layer, QgsStyle *style, QgsPointCloudRenderer * )
{
  return new QgsPointCloudAttributeByRampRendererWidget( layer, style );
}

QgsPointCloudRenderer *QgsPointCloudAttributeByRampRendererWidget::renderer()
{
  if ( !mLayer )
  {
    return nullptr;
  }

  std::unique_ptr< QgsPointCloudAttributeByRampRenderer > renderer = std::make_unique< QgsPointCloudAttributeByRampRenderer >();
  renderer->setAttribute( mAttributeComboBox->currentAttribute() );

  renderer->setMinimum( mMinSpin->value() );
  renderer->setMaximum( mMaxSpin->value() );

  renderer->setColorRampShader( mScalarColorRampShaderWidget->shader() );

  return renderer.release();
}

void QgsPointCloudAttributeByRampRendererWidget::emitWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit widgetChanged();
}

void QgsPointCloudAttributeByRampRendererWidget::minMaxChanged()
{
  if ( mBlockMinMaxChanged )
    return;

  mScalarColorRampShaderWidget->setMinimumMaximumAndClassify( mMinSpin->value(), mMaxSpin->value() );
}

void QgsPointCloudAttributeByRampRendererWidget::attributeChanged()
{
  if ( mLayer && mLayer->dataProvider() )
  {
    const QgsPointCloudStatistics stats = mLayer->statistics();
    const double min = stats.minimum( mAttributeComboBox->currentAttribute() );
    const double max = stats.maximum( mAttributeComboBox->currentAttribute() );
    if ( !std::isnan( min ) && !std::isnan( max ) )
    {
      mProviderMin = min;
      mProviderMax = max;
    }
    else
    {
      mProviderMin = std::numeric_limits< double >::quiet_NaN();
      mProviderMax = std::numeric_limits< double >::quiet_NaN();
    }

    if ( mAttributeComboBox->currentAttribute().compare( QLatin1String( "z" ), Qt::CaseInsensitive ) == 0 )
    {
      const double zScale = static_cast< const QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() )->zScale();
      const double zOffset = static_cast< const QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() )->zOffset();
      mProviderMin = mProviderMin * zScale + zOffset;
      mProviderMax = mProviderMax * zScale + zOffset;
    }

  }
  if ( !mBlockSetMinMaxFromLayer )
    setMinMaxFromLayer();

  mScalarRecalculateMinMaxButton->setEnabled( !std::isnan( mProviderMin ) && !std::isnan( mProviderMax ) );
  emitWidgetChanged();
}

void QgsPointCloudAttributeByRampRendererWidget::setMinMaxFromLayer()
{
  if ( std::isnan( mProviderMin ) || std::isnan( mProviderMax ) )
    return;

  mBlockMinMaxChanged = true;
  mMinSpin->setValue( mProviderMin );
  mMaxSpin->setValue( mProviderMax );
  mBlockMinMaxChanged = false;

  minMaxChanged();
}

void QgsPointCloudAttributeByRampRendererWidget::setFromRenderer( const QgsPointCloudRenderer *r )
{
  mBlockChangedSignal = true;
  const QgsPointCloudAttributeByRampRenderer *mbcr = dynamic_cast<const QgsPointCloudAttributeByRampRenderer *>( r );
  if ( mbcr )
  {
    // we will be restoring the existing ramp classes -- we don't want to regenerate any automatically!
    mBlockSetMinMaxFromLayer = true;

    mAttributeComboBox->setAttribute( mbcr->attribute() );

    mMinSpin->setValue( mbcr->minimum() );
    mMaxSpin->setValue( mbcr->maximum() );

    whileBlocking( mScalarColorRampShaderWidget )->setFromShader( mbcr->colorRampShader() );
    whileBlocking( mScalarColorRampShaderWidget )->setMinimumMaximum( mbcr->minimum(), mbcr->maximum() );
  }
  else
  {
    if ( mAttributeComboBox->findText( QStringLiteral( "Intensity" ) ) > -1 )
    {
      mAttributeComboBox->setAttribute( QStringLiteral( "Intensity" ) );
    }
    else
    {
      mAttributeComboBox->setCurrentIndex( mAttributeComboBox->count() > 1 ? 1 : 0 );
    }
  }
  attributeChanged();
  mBlockChangedSignal = false;
  mBlockSetMinMaxFromLayer = false;
}

///@endcond
