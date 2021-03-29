/***************************************************************************
  qgsinterpolatedlinerendererwidget.cpp - QgsInterpolatedLineRendererWidget

 ---------------------
 begin                : 23.3.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsinterpolatedlinerendererwidget.h"

#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsproject.h"
#include "qgstemporalcontroller.h"
#include "qgsmapcanvas.h"
#include "qgsdoublevalidator.h"

QgsRendererWidget *QgsInterpolatedLineRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsInterpolatedLineRendererWidget( layer, style, renderer );
}

QgsInterpolatedLineRendererWidget::QgsInterpolatedLineRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) : QgsRendererWidget( layer, style )
{
  if ( !layer )
  {
    return;
  }
  // the renderer only applies to point vector layers
  if ( layer->geometryType() != QgsWkbTypes::LineGeometry )
  {
    //setup blank dialog
    mRenderer = nullptr;
    QLabel *label = new QLabel( tr( "The interpolated lines renderer only applies to line and multiline layers. \n"
                                    "'%1' is not a line layer and cannot be rendered as interpolated lines." )
                                .arg( layer->name() ), this );
    if ( !layout() )
      setLayout( new QGridLayout() );
    layout()->addWidget( label );
    return;
  }

  setupUi( this );

  if ( renderer )
  {
    mRenderer.reset( QgsInterpolatedLineFeatureRenderer::convertFromRenderer( renderer ) );
  }
  else
    mRenderer = std::make_unique<QgsInterpolatedLineFeatureRenderer>();

  syncToRenderer();

  mFirstValueFieldExpressionWidget->setFilters( QgsFieldProxyModel::Numeric );
  mSecondValueFieldExpressionWidget->setFilters( QgsFieldProxyModel::Numeric );
  mFirstValueFieldExpressionWidget->setLayer( layer );
  mSecondValueFieldExpressionWidget->setLayer( layer );

  mLineEditColorMin->setValidator( new QgsDoubleValidator( mLineEditColorMin ) );
  mLineEditColorMax->setValidator( new QgsDoubleValidator( mLineEditColorMax ) );
  connect( mLineEditColorMin, &QLineEdit::textChanged, this, &QgsInterpolatedLineRendererWidget::onColorMinMaxLineTextChanged );
  connect( mLineEditColorMin, &QLineEdit::textEdited, this, &QgsInterpolatedLineRendererWidget::onColorMinMaxLineTextEdited );
  connect( mLineEditColorMax, &QLineEdit::textChanged, this, &QgsInterpolatedLineRendererWidget::onColorMinMaxLineTextChanged );
  connect( mLineEditColorMax, &QLineEdit::textEdited, this, &QgsInterpolatedLineRendererWidget::onColorMinMaxLineTextEdited );

  connect( mFirstValueFieldExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged )
           , this, &QgsInterpolatedLineRendererWidget::reloadMinMaxFromLayer );

  connect( mSecondValueFieldExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged )
           , this, &QgsInterpolatedLineRendererWidget::reloadMinMaxFromLayer );

  connect( mButtonLoadColorMinMax, &QPushButton::clicked, this, &QgsInterpolatedLineRendererWidget::onLoadColorMinmax );

  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsInterpolatedLineRendererWidget::widgetChanged );
  connect( mPushButtonVariableWidth, &QgsMeshVariableStrokeWidthButton::widgetChanged, this, &QgsInterpolatedLineRendererWidget::widgetChanged );
  connect( mRadioButtonFixWidth, &QRadioButton::toggled, this, &QgsInterpolatedLineRendererWidget::onFixedWidthChanged );

  mWidthUnitSelection->setUnits( QgsUnitTypes::RenderUnitList()
                                 << QgsUnitTypes::RenderUnit::RenderInches
                                 << QgsUnitTypes::RenderUnit::RenderMapUnits
                                 << QgsUnitTypes::RenderUnit::RenderMetersInMapUnits
                                 << QgsUnitTypes::RenderUnit::RenderMillimeters
                                 << QgsUnitTypes::RenderUnit::RenderPixels
                                 << QgsUnitTypes::RenderUnit::RenderPoints );

  connect( mWidthUnitSelection, &QgsUnitSelectionWidget::changed, this, &QgsInterpolatedLineRendererWidget::widgetChanged );

  onFixedWidthChanged();
}

QgsFeatureRenderer *QgsInterpolatedLineRendererWidget::renderer()
{
  mRenderer->setInterpolatedColor( QgsInterpolatedLineColor( mColorRampShaderWidget->shader() ) );
  QgsInterpolatedLineWidth interpolatedWidth = mPushButtonVariableWidth->variableStrokeWidth();
  interpolatedWidth.setFixedStrokeWidth( mSpinBoxWidth->value() );
  interpolatedWidth.setIsVariableWidth( mRadioButtonVariableWidth->isChecked() );

  QgsInterpolatedLineWidth interpolatedWitdh( mPushButtonVariableWidth->variableStrokeWidth() );
  interpolatedWitdh.setIsVariableWidth( mRadioButtonVariableWidth->isChecked() );
  mRenderer->setWidthUnit( mWidthUnitSelection->unit() );
  mRenderer->setInterpolatedWidth( interpolatedWitdh );
  mRenderer->setExpressionsString( mFirstValueFieldExpressionWidget->expression(), mSecondValueFieldExpressionWidget->expression() );
  return mRenderer.get();
}

void QgsInterpolatedLineRendererWidget::reloadMinMaxFromLayer()
{
  QgsExpressionContext expressionContext = createExpressionContext();

  QgsExpression expressionFirst( mFirstValueFieldExpressionWidget->expression() );
  if ( !expressionFirst.prepare( &expressionContext ) )
  {
    emit widgetChanged();
    return;
  }

  QgsExpression expressionSecond( mSecondValueFieldExpressionWidget->expression() );
  if ( !expressionSecond.prepare( &expressionContext ) )
  {
    emit widgetChanged();
    return;
  }

  QgsFeatureIterator it = vectorLayer()->getFeatures();
  QgsFeature feat;
  mMinimumFromLayer = std::numeric_limits<double>::max();
  mMaximumFromLayer = -std::numeric_limits<double>::max();
  while ( it.nextFeature( feat ) )
  {
    expressionContext.setFeature( feat );
    double firstvalue = expressionFirst.evaluate( &expressionContext ).toDouble();
    double secondvalue = expressionSecond.evaluate( &expressionContext ).toDouble();

    if ( firstvalue < mMinimumFromLayer )
      mMinimumFromLayer = firstvalue;
    if ( firstvalue > mMaximumFromLayer )
      mMaximumFromLayer = firstvalue;

    if ( secondvalue < mMinimumFromLayer )
      mMinimumFromLayer = secondvalue;
    if ( secondvalue > mMaximumFromLayer )
      mMaximumFromLayer = secondvalue;
  }

  mPushButtonVariableWidth->setDefaultMinMaxValue( mMinimumFromLayer, mMaximumFromLayer );

  bool minMaxColorChanged = false;
  if ( mLineEditColorMin->text().isEmpty() && !std::isnan( mMinimumFromLayer ) )
  {
    setLineEditValue( mLineEditColorMin, mMinimumFromLayer );
    minMaxColorChanged = true;
  }

  if ( mLineEditColorMax->text().isEmpty() && !std::isnan( mMaximumFromLayer ) )
  {
    setLineEditValue( mLineEditColorMax, mMaximumFromLayer );
    minMaxColorChanged = true;
  }

  if ( minMaxColorChanged )
    onColorMinMaxLineTextEdited();

  emit widgetChanged();
}

void QgsInterpolatedLineRendererWidget::onLoadColorMinmax()
{
  setLineEditValue( mLineEditColorMin, mMinimumFromLayer );
  setLineEditValue( mLineEditColorMax, mMaximumFromLayer );
  onColorMinMaxLineTextEdited();
}

void QgsInterpolatedLineRendererWidget::onColorMinMaxLineTextChanged()
{
  double min = lineEditValue( mLineEditColorMin );
  double max = lineEditValue( mLineEditColorMax );
  whileBlocking( mColorRampShaderWidget )->setMinimumMaximum( min, max );
  emit widgetChanged();
}

void QgsInterpolatedLineRendererWidget::onColorMinMaxLineTextEdited()
{
  double min = lineEditValue( mLineEditColorMin );
  double max = lineEditValue( mLineEditColorMax );
  whileBlocking( mColorRampShaderWidget )->setMinimumMaximumAndClassify( min, max );
  emit widgetChanged();
}

void QgsInterpolatedLineRendererWidget::onFixedWidthChanged()
{
  mSpinBoxWidth->setVisible( mRadioButtonFixWidth->isChecked() );
  mPushButtonVariableWidth->setVisible( mRadioButtonVariableWidth->isChecked() );
  emit widgetChanged();
}

QgsExpressionContext QgsInterpolatedLineRendererWidget::createExpressionContext() const
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
             << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
             << QgsExpressionContextUtils::atlasScope( nullptr );

  if ( auto *lMapCanvas = mContext.mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( lMapCanvas->mapSettings() )
               << new QgsExpressionContextScope( lMapCanvas->expressionContextScope() );
    if ( const QgsExpressionContextScopeGenerator *generator = dynamic_cast< const QgsExpressionContextScopeGenerator * >( lMapCanvas->temporalController() ) )
    {
      expContext << generator->createExpressionContextScope();
    }
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
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

double QgsInterpolatedLineRendererWidget::lineEditValue( QLineEdit *lineEdit )
{
  if ( lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return QgsDoubleValidator::toDouble( lineEdit->text() );
}

void QgsInterpolatedLineRendererWidget::setLineEditValue( QLineEdit *lineEdit, double value )
{
  QString strValue;
  if ( !std::isnan( value ) )
    strValue = QString::number( value );
  whileBlocking( lineEdit )->setText( strValue );
}

void QgsInterpolatedLineRendererWidget::syncToRenderer()
{
  if ( !mRenderer )
    return;

  whileBlocking( mFirstValueFieldExpressionWidget )->setExpression( mRenderer->firstExpression() );
  whileBlocking( mSecondValueFieldExpressionWidget )->setExpression( mRenderer->secondExpression() );

  const QgsInterpolatedLineColor &interpolatedColor = mRenderer->interpolatedColor();
  double minColor = interpolatedColor.colorRampShader().minimumValue();
  double maxColor = interpolatedColor.colorRampShader().maximumValue();
  setLineEditValue( mLineEditColorMin, minColor );
  setLineEditValue( mLineEditColorMax, maxColor );
  whileBlocking( mColorRampShaderWidget )->setFromShader( interpolatedColor.colorRampShader() );
  whileBlocking( mColorRampShaderWidget )->setMinimumMaximum( minColor, maxColor );

  const QgsInterpolatedLineWidth &interpolatedWidth = mRenderer->interpolatedLineWidth();
  whileBlocking( mPushButtonVariableWidth )->setVariableStrokeWidth( interpolatedWidth );
  whileBlocking( mSpinBoxWidth )->setValue( interpolatedWidth.fixedStrokeWidth() );
  whileBlocking( mWidthUnitSelection )->setUnit( mRenderer->widthUnit() );
  whileBlocking( mRadioButtonVariableWidth )->setChecked( interpolatedWidth.isVariableWidth() );
}
