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
#include "moc_qgsheatmaprendererwidget.cpp"
#include "qgsheatmaprenderer.h"
#include "qgsexpressioncontextutils.h"
#include "qgstemporalcontroller.h"
#include "qgsvectorlayer.h"
#include "qgscolorramp.h"
#include "qgscolorrampbutton.h"
#include "qgsstyle.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgscolorramplegendnodewidget.h"
#include <QGridLayout>
#include <QLabel>

QgsRendererWidget *QgsHeatmapRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsHeatmapRendererWidget( layer, style, renderer );
}

QgsExpressionContext QgsHeatmapRendererWidget::createExpressionContext() const
{
  QgsExpressionContext expContext;

  if ( auto *lMapCanvas = mContext.mapCanvas() )
  {
    expContext = lMapCanvas->createExpressionContext();
  }
  else
  {
    expContext << QgsExpressionContextUtils::globalScope()
               << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
               << QgsExpressionContextUtils::atlasScope( nullptr )
               << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  if ( auto *lVectorLayer = vectorLayer() )
    expContext << QgsExpressionContextUtils::layerScope( lVectorLayer );

  // additional scopes
  const auto constAdditionalExpressionContextScopes = mContext.additionalExpressionContextScopes();
  for ( const QgsExpressionContextScope &scope : constAdditionalExpressionContextScopes )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  return expContext;
}

QgsHeatmapRendererWidget::QgsHeatmapRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )

{
  if ( !layer )
  {
    return;
  }
  // the renderer only applies to point vector layers
  if ( layer->geometryType() != Qgis::GeometryType::Point )
  {
    //setup blank dialog
    mRenderer = nullptr;
    QLabel *label = new QLabel( tr( "The heatmap renderer only applies to point and multipoint layers. \n"
                                    "'%1' is not a point layer and cannot be rendered as a heatmap." )
                                  .arg( layer->name() ),
                                this );
    if ( !layout() )
      setLayout( new QGridLayout() );
    layout()->addWidget( label );
    return;
  }

  setupUi( this );
  connect( mRadiusUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsHeatmapRendererWidget::mRadiusUnitWidget_changed );
  connect( mRadiusSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsHeatmapRendererWidget::mRadiusSpinBox_valueChanged );
  connect( mMaxSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsHeatmapRendererWidget::mMaxSpinBox_valueChanged );
  connect( mQualitySlider, &QSlider::valueChanged, this, &QgsHeatmapRendererWidget::mQualitySlider_valueChanged );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );

  mRadiusUnitWidget->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::MetersInMapUnits, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::MapUnits, Qgis::RenderUnit::Points, Qgis::RenderUnit::Inches } );
  mWeightExpressionWidget->registerExpressionContextGenerator( this );
  mWeightExpressionWidget->setAllowEmptyFieldName( true );

  if ( renderer )
  {
    mRenderer.reset( QgsHeatmapRenderer::convertFromRenderer( renderer ) );
  }
  if ( !mRenderer )
  {
    mRenderer = std::make_unique<QgsHeatmapRenderer>();
    if ( renderer )
      renderer->copyRendererData( mRenderer.get() );
  }

  btnColorRamp->setShowGradientOnly( true );

  connect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsHeatmapRendererWidget::applyColorRamp );
  connect( mLegendSettingsButton, &QPushButton::clicked, this, &QgsHeatmapRendererWidget::showLegendSettings );

  if ( mRenderer->colorRamp() )
  {
    btnColorRamp->blockSignals( true );
    btnColorRamp->setColorRamp( mRenderer->colorRamp() );
    btnColorRamp->blockSignals( false );
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

  mWeightExpressionWidget->setLayer( layer );
  mWeightExpressionWidget->setField( mRenderer->weightExpression() );
  connect( mWeightExpressionWidget, static_cast<void ( QgsFieldExpressionWidget::* )( const QString & )>( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsHeatmapRendererWidget::weightExpressionChanged );

  registerDataDefinedButton( mRadiusDDBtn, QgsFeatureRenderer::Property::HeatmapRadius );
  registerDataDefinedButton( mMaximumValueDDBtn, QgsFeatureRenderer::Property::HeatmapMaximum );
}

QgsHeatmapRendererWidget::~QgsHeatmapRendererWidget() = default;

QgsFeatureRenderer *QgsHeatmapRendererWidget::renderer()
{
  return mRenderer.get();
}

void QgsHeatmapRendererWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsRendererWidget::setContext( context );
  if ( auto *lMapCanvas = context.mapCanvas() )
    mRadiusUnitWidget->setMapCanvas( lMapCanvas );
}

void QgsHeatmapRendererWidget::applyColorRamp()
{
  if ( !mRenderer )
  {
    return;
  }

  QgsColorRamp *ramp = btnColorRamp->colorRamp();
  if ( !ramp )
    return;

  mRenderer->setColorRamp( ramp );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::showLegendSettings()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( qobject_cast<QWidget *>( parent() ) );
  if ( panel && panel->dockMode() )
  {
    QgsColorRampLegendNodeWidget *legendPanel = new QgsColorRampLegendNodeWidget( nullptr, QgsColorRampLegendNodeWidget::Capabilities() );
    legendPanel->setUseContinuousRampCheckBoxVisibility( false );
    legendPanel->setPanelTitle( tr( "Legend Settings" ) );
    legendPanel->setSettings( mRenderer->legendSettings() );
    connect( legendPanel, &QgsColorRampLegendNodeWidget::widgetChanged, this, [=] {
      mRenderer->setLegendSettings( legendPanel->settings() );
      emit widgetChanged();
    } );
    panel->openPanel( legendPanel );
  }
  else
  {
    QgsColorRampLegendNodeDialog dialog( mRenderer->legendSettings(), this, QgsColorRampLegendNodeWidget::Capabilities() );
    dialog.setUseContinuousRampCheckBoxVisibility( false );
    dialog.setWindowTitle( tr( "Legend Settings" ) );
    if ( dialog.exec() )
    {
      mRenderer->setLegendSettings( dialog.settings() );
      emit widgetChanged();
    }
  }
}

void QgsHeatmapRendererWidget::mRadiusUnitWidget_changed()
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setRadiusUnit( mRadiusUnitWidget->unit() );
  mRenderer->setRadiusMapUnitScale( mRadiusUnitWidget->getMapUnitScale() );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::mRadiusSpinBox_valueChanged( double d )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setRadius( d );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::mMaxSpinBox_valueChanged( double d )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setMaximumValue( d );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::mQualitySlider_valueChanged( int v )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setRenderQuality( v );
  emit widgetChanged();
}

void QgsHeatmapRendererWidget::weightExpressionChanged( const QString &expression )
{
  mRenderer->setWeightExpression( expression );
  emit widgetChanged();
}
