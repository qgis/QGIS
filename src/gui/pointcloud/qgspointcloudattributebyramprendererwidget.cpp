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

///@cond PRIVATE

QgsPointCloudAttributeByRampRendererWidget::QgsPointCloudAttributeByRampRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style )
  : QgsPointCloudRendererWidget( layer, style )
{
  setupUi( this );

  mAttributeComboBox->setAllowEmptyAttributeName( true );
  mAttributeComboBox->setFilters( QgsPointCloudAttributeProxyModel::Numeric );

  if ( layer )
  {
    mAttributeComboBox->setLayer( layer );

    setFromRenderer( layer->renderer() );
  }

  connect( mAttributeComboBox, &QgsPointCloudAttributeComboBox::attributeChanged,
           this, &QgsPointCloudAttributeByRampRendererWidget::emitWidgetChanged );
  connect( mBtnColorRamp, &QgsColorRampButton::colorRampChanged,
           this, &QgsPointCloudAttributeByRampRendererWidget::emitWidgetChanged );
  connect( mMinSpin, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloudAttributeByRampRendererWidget::emitWidgetChanged );
  connect( mMaxSpin, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloudAttributeByRampRendererWidget::emitWidgetChanged );
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

  std::unique_ptr< QgsPointCloudAttributeByRampRenderer > renderer = qgis::make_unique< QgsPointCloudAttributeByRampRenderer >();
  renderer->setAttribute( mAttributeComboBox->currentAttribute() );

  renderer->setColorRamp( mBtnColorRamp->colorRamp() );
  renderer->setMin( mMinSpin->value() );
  renderer->setMax( mMaxSpin->value() );
  return renderer.release();
}

void QgsPointCloudAttributeByRampRendererWidget::emitWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit widgetChanged();
}

void QgsPointCloudAttributeByRampRendererWidget::setFromRenderer( const QgsPointCloudRenderer *r )
{
  mBlockChangedSignal = true;
  const QgsPointCloudAttributeByRampRenderer *mbcr = dynamic_cast<const QgsPointCloudAttributeByRampRenderer *>( r );
  if ( mbcr )
  {
    mAttributeComboBox->setAttribute( mbcr->attribute() );

    mBtnColorRamp->setColorRamp( mbcr->colorRamp() );

    mMinSpin->setValue( mbcr->min() );
    mMaxSpin->setValue( mbcr->max() );
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

    mBtnColorRamp->setColorRamp( QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
  }
  mBlockChangedSignal = false;
}

///@endcond
