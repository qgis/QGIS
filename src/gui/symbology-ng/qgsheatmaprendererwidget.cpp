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
#include "qgsvectorgradientcolorrampv2dialog.h"
#include "qgsstylev2.h"
#include "qgsproject.h"
#include <QGridLayout>
#include <QLabel>

QgsRendererV2Widget* QgsHeatmapRendererWidget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsHeatmapRendererWidget( layer, style, renderer );
}

QgsHeatmapRendererWidget::QgsHeatmapRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
{
  if ( !layer )
  {
    return;
  }
  // the renderer only applies to point vector layers
  if ( layer->geometryType() != QGis::Point )
  {
    //setup blank dialog
    mRenderer = NULL;
    QGridLayout* layout = new QGridLayout( this );
    QLabel* label = new QLabel( tr( "The heatmap renderer only applies to point and multipoint layers. \n"
                                    "'%1' is not a point layer and cannot be rendered as a heatmap." )
                                .arg( layer->name() ), this );
    layout->addWidget( label );
    return;
  }

  setupUi( this );
  mRadiusUnitWidget->setUnits( QStringList() << tr( "Pixels" ) << tr( "Millimeter" ) << tr( "Map unit" ), 2 );

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
  switch ( mRenderer->radiusUnit() )
  {
    case QgsSymbolV2::MM:
      mRadiusUnitWidget->setUnit( 1 );
      break;
    case QgsSymbolV2::MapUnit:
      mRadiusUnitWidget->setUnit( 2 );
      break;
    case QgsSymbolV2::Pixel:
    default:
      mRadiusUnitWidget->setUnit( 0 );
      break;
  }
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

void QgsHeatmapRendererWidget::applyColorRamp()
{
  if ( !mRenderer )
  {
    return;
  }

  QgsVectorColorRampV2* ramp = mRampComboBox->currentColorRamp();
  if ( ramp == NULL )
    return;

  mRenderer->setColorRamp( ramp );
}

void QgsHeatmapRendererWidget::on_mButtonEditRamp_clicked()
{
  if ( mRenderer && mRenderer->colorRamp()->type() == "gradient" )
  {
    QgsVectorColorRampV2* ramp = mRenderer->colorRamp()->clone();
    QgsVectorGradientColorRampV2* gradRamp = static_cast<QgsVectorGradientColorRampV2*>( ramp );
    QgsVectorGradientColorRampV2Dialog dlg( gradRamp, this );

    if ( dlg.exec() && gradRamp )
    {
      mRenderer->setColorRamp( gradRamp );
      mRampComboBox->blockSignals( true );
      mRampComboBox->setSourceColorRamp( mRenderer->colorRamp() );
      mRampComboBox->blockSignals( false );
    }
    else
    {
      delete ramp;
    }
  }
}

void QgsHeatmapRendererWidget::on_mRadiusUnitWidget_changed()
{
  if ( !mRenderer )
  {
    return;
  }
  QgsSymbolV2::OutputUnit unit;
  switch ( mRadiusUnitWidget->getUnit() )
  {
    case 0:
      unit = QgsSymbolV2::Pixel;
      break;
    case 2:
      unit = QgsSymbolV2::MapUnit;
      break;
    case 1:
    default:
      unit = QgsSymbolV2::MM;
      break;
  }

  mRenderer->setRadiusUnit( unit );
  mRenderer->setRadiusMapUnitScale( mRadiusUnitWidget->getMapUnitScale() );
}

void QgsHeatmapRendererWidget::on_mRadiusSpinBox_valueChanged( double d )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setRadius( d );
}

void QgsHeatmapRendererWidget::on_mMaxSpinBox_valueChanged( double d )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setMaximumValue( d );
}

void QgsHeatmapRendererWidget::on_mQualitySlider_valueChanged( int v )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setRenderQuality( v );
}

void QgsHeatmapRendererWidget::on_mInvertCheckBox_toggled( bool v )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setInvertRamp( v );
}

void QgsHeatmapRendererWidget::weightExpressionChanged( QString expression )
{
  mRenderer->setWeightExpression( expression );
}
