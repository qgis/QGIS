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
#include "qgsrendererregistry.h"

#include "qgssymbol.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgscolorramp.h"
#include "qgsstyle.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include <QGridLayout>
#include <QLabel>

QgsRendererWidget* QgsHeatmapRendererWidget::create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer )
{
  return new QgsHeatmapRendererWidget( layer, style, renderer );
}

QgsExpressionContext QgsHeatmapRendererWidget::createExpressionContext() const
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::atlasScope( nullptr );

  if ( mContext.mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( mContext.mapCanvas()->mapSettings() )
    << new QgsExpressionContextScope( mContext.mapCanvas()->expressionContextScope() );
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  if ( vectorLayer() )
    expContext << QgsExpressionContextUtils::layerScope( vectorLayer() );

  // additional scopes
  Q_FOREACH ( const QgsExpressionContextScope& scope, mContext.additionalExpressionContextScopes() )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  return expContext;
}

QgsHeatmapRendererWidget::QgsHeatmapRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer )
    : QgsRendererWidget( layer, style )
    , mRenderer( nullptr )
{
  if ( !layer )
  {
    return;
  }
  // the renderer only applies to point vector layers
  if ( layer->geometryType() != QgsWkbTypes::PointGeometry )
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

  mRadiusUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderMapUnits );
  mWeightExpressionWidget->registerExpressionContextGenerator( this );

  if ( renderer )
  {
    mRenderer = QgsHeatmapRenderer::convertFromRenderer( renderer );
  }
  if ( !mRenderer )
  {
    mRenderer = new QgsHeatmapRenderer();
  }

  mRampComboBox->setShowGradientOnly( true );
  mRampComboBox->populate( QgsStyle::defaultStyle() );
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

QgsFeatureRenderer* QgsHeatmapRendererWidget::renderer()
{
  return mRenderer;
}

void QgsHeatmapRendererWidget::setContext( const QgsSymbolWidgetContext& context )
{
  QgsRendererWidget::setContext( context );
  if ( context.mapCanvas() )
    mRadiusUnitWidget->setMapCanvas( context.mapCanvas() );
}

void QgsHeatmapRendererWidget::applyColorRamp()
{
  if ( !mRenderer )
  {
    return;
  }

  QgsColorRamp* ramp = mRampComboBox->currentColorRamp();
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
