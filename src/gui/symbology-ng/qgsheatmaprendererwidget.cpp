/***************************************************************************
    qgsheatmaprendererwidget.cpp
    ----------------------------
    begin                : November 2014
    copyright            : (C) 2014 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsheatmaprendererwidget.h"
#include "qgsheatmaprenderer.h"
#include "qgsrendererv2registry.h"

#include "qgssymbolv2.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsstylev2.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include <QGridLayout>
#include <QLabel>

QgsRendererV2Widget* QgsHeatmapRendererWidget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsHeatmapRendererWidget( layer, style, renderer );
}

static QgsExpressionContext _getExpressionContext( const void* context )
{
  const QgsHeatmapRendererWidget* widget = reinterpret_cast< const QgsHeatmapRendererWidget* >( context );

  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::atlasScope( nullptr );

  if ( widget->mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( widget->mapCanvas()->mapSettings() )
    << new QgsExpressionContextScope( widget->mapCanvas()->expressionContextScope() );
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  if ( widget->vectorLayer() )
    expContext << QgsExpressionContextUtils::layerScope( widget->vectorLayer() );

  return expContext;
}

QgsHeatmapRendererWidget::QgsHeatmapRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
    , mRenderer( nullptr )
{
  if ( !layer )
  {
    return;
  }
  // the renderer only applies to point vector layers
  if ( layer->geometryType() != QGis::Point )
  {
    //setup blank dialog
    mRenderer = nullptr;
    QLabel* label = new QLabel( tr( "The heatmap renderer only applies to point and multipoint layers. \n"
                                    "'%1' is not a point layer and cannot be rendered as a heatmap." )
                                .arg( layer->name() ), this );
    layout()->addWidget( label );
    return;
  }

  setupUi( this );

  mRadiusUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::Pixel << QgsSymbolV2::MapUnit );
  mWeightExpressionWidget->registerGetExpressionContextCallback( &_getExpressionContext, this );

  if ( renderer )
  {
    mRenderer = QgsHeatmapRenderer::convertFromRenderer( renderer );
  }
  if ( !mRenderer )
  {
    mRenderer = new QgsHeatmapRenderer();
  }

  mRampComboBox->setShowGradientOnly( true );
  mRampComboBox->populate( QgsStyleV2::defaultStyle() );
  connect( mRampComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( applyColorRamp() ) );
  connect( mRampComboBox, SIGNAL( sourceRampEdited() ), this, SLOT( applyColorRamp() ) );
  connect( mButtonEditRamp, SIGNAL( clicked() ), mRampComboBox, SLOT( editSourceRamp() ) );

  if ( mRenderer->colorRamp() )
  {
    mRampComboBox->blockSignals( true );
    mRampComboBox->setSourceColorRamp( mRenderer->colorRamp() );
    mRampComboBox->blockSignals( false );
  }
  mRadiusSpinBox->blockSignals( true );
  mRadiusSpinBox->setValue( mRenderer->radius() );
  mRadiusSpinBox->blockSignals( false );
  mRadiusUnitWidget->blockSignals( true );
  mRadiusUnitWidget->setUnit( mRenderer->radiusUnit() );
  mRadiusUnitWidget->setMapUnitScale( mRenderer->radiusMapUnitScale() );
  mRadiusUnitWidget->blockSignals( false );
  mMaxSpinBox->blockSignals( true );
  mMaxSpinBox->setValue( mRenderer->maximumValue() );
  mMaxSpinBox->blockSignals( false );
  mQualitySlider->blockSignals( true );
  mQualitySlider->setValue( mRenderer->renderQuality() );
  mQualitySlider->blockSignals( false );
  mInvertCheckBox->blockSignals( true );
  mInvertCheckBox->setChecked( mRenderer->invertRamp() );
  mInvertCheckBox->blockSignals( false );

  mWeightExpressionWidget->setLayer( layer );
  mWeightExpressionWidget->setField( mRenderer->weightExpression() );
  connect( mWeightExpressionWidget, SIGNAL( fieldChanged( QString ) ), this, SLOT( weightExpressionChanged( QString ) ) );
}

QgsFeatureRendererV2* QgsHeatmapRendererWidget::renderer()
{
  return mRenderer;
}

void QgsHeatmapRendererWidget::setMapCanvas( QgsMapCanvas* canvas )
{
  QgsRendererV2Widget::setMapCanvas( canvas );
  if ( mRadiusUnitWidget )
    mRadiusUnitWidget->setMapCanvas( canvas );
}

void QgsHeatmapRendererWidget::applyColorRamp()
{
  if ( !mRenderer )
  {
    return;
  }

  QgsVectorColorRampV2* ramp = mRampComboBox->currentColorRamp();
  if ( !ramp )
    return;

  mRenderer->setColorRamp( ramp );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::on_mRadiusUnitWidget_changed()
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setRadiusUnit( mRadiusUnitWidget->unit() );
  mRenderer->setRadiusMapUnitScale( mRadiusUnitWidget->getMapUnitScale() );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::on_mRadiusSpinBox_valueChanged( double d )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setRadius( d );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::on_mMaxSpinBox_valueChanged( double d )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setMaximumValue( d );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::on_mQualitySlider_valueChanged( int v )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setRenderQuality( v );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::on_mInvertCheckBox_toggled( bool v )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setInvertRamp( v );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::weightExpressionChanged( const QString& expression )
{
  mRenderer->setWeightExpression( expression );
  emit widgetChanged();
}
