/***************************************************************************
  qgsinterpolatedlinesymbollayerwidget.cpp - QgsInterpolatedLineSymbolLayerWidget

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
#include "qgsinterpolatedlinesymbollayerwidget.h"

#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsproject.h"
#include "qgstemporalcontroller.h"
#include "qgsmapcanvas.h"
#include "qgsdoublevalidator.h"


QgsInterpolatedLineSymbolLayerWidget::QgsInterpolatedLineSymbolLayerWidget( QgsVectorLayer *layer, QWidget *parent )
  : QgsSymbolLayerWidget( parent, layer )
{
  setupUi( this );

  mWidthMethodComboBox->addItem( tr( "Fixed Width" ), false );
  mWidthMethodComboBox->addItem( tr( "Varying Width" ), true );
  mColorMethodComboBox->addItem( tr( "Single Color" ), QgsInterpolatedLineColor::SingleColor );
  mColorMethodComboBox->addItem( tr( "Varying Color" ), QgsInterpolatedLineColor::ColorRamp );

  mWidthStartFieldExpression->setFilters( QgsFieldProxyModel::Numeric );
  mWidthEndFieldExpression->setFilters( QgsFieldProxyModel::Numeric );
  mColorStartFieldExpression->setFilters( QgsFieldProxyModel::Numeric );
  mColorEndFieldExpression->setFilters( QgsFieldProxyModel::Numeric );

  mWidthStartFieldExpression->setLayer( layer );
  mWidthEndFieldExpression->setLayer( layer );
  mColorStartFieldExpression->setLayer( layer );
  mColorEndFieldExpression->setLayer( layer );

  mWidthUnitSelectionFixed->setUnits( QgsUnitTypes::RenderUnitList()
                                      << QgsUnitTypes::RenderUnit::RenderInches
                                      << QgsUnitTypes::RenderUnit::RenderMapUnits
                                      << QgsUnitTypes::RenderUnit::RenderMetersInMapUnits
                                      << QgsUnitTypes::RenderUnit::RenderMillimeters
                                      << QgsUnitTypes::RenderUnit::RenderPixels
                                      << QgsUnitTypes::RenderUnit::RenderPoints );

  mWidthUnitSelectionVarying->setUnits( QgsUnitTypes::RenderUnitList()
                                        << QgsUnitTypes::RenderUnit::RenderInches
                                        << QgsUnitTypes::RenderUnit::RenderMapUnits
                                        << QgsUnitTypes::RenderUnit::RenderMetersInMapUnits
                                        << QgsUnitTypes::RenderUnit::RenderMillimeters
                                        << QgsUnitTypes::RenderUnit::RenderPixels
                                        << QgsUnitTypes::RenderUnit::RenderPoints );

  connect( mWidthMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ),
           this, &QgsInterpolatedLineSymbolLayerWidget::updateVisibleWidget );
  connect( mColorMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ),
           this, &QgsInterpolatedLineSymbolLayerWidget::updateVisibleWidget );

  // Width parameter
  connect( mWidthMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ),
           this, &QgsInterpolatedLineSymbolLayerWidget::apply );
  connect( mDoubleSpinBoxWidth, qOverload<double>( &QDoubleSpinBox::valueChanged ),
           this, &QgsInterpolatedLineSymbolLayerWidget::apply );
  connect( mWidthStartFieldExpression, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged )
           , this, &QgsInterpolatedLineSymbolLayerWidget::reloadMinMaxWidthFromLayer );
  connect( mWidthEndFieldExpression, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged )
           , this, &QgsInterpolatedLineSymbolLayerWidget::reloadMinMaxWidthFromLayer );
  connect( mButtonLoadMinMaxValueWidth, &QPushButton::clicked, this, &QgsInterpolatedLineSymbolLayerWidget::onReloadMinMaxValueWidth );
  connect( mLineEditWidthMinValue, &QLineEdit::textChanged, this, &QgsInterpolatedLineSymbolLayerWidget::apply );
  connect( mLineEditWidthMaxValue, &QLineEdit::textChanged, this, &QgsInterpolatedLineSymbolLayerWidget::apply );
  connect( mDoubleSpinBoxMinWidth, qOverload<double>( &QDoubleSpinBox::valueChanged ),
           this, &QgsInterpolatedLineSymbolLayerWidget::apply );
  connect( mDoubleSpinBoxMaxWidth, qOverload<double>( &QDoubleSpinBox::valueChanged ),
           this, &QgsInterpolatedLineSymbolLayerWidget::apply );
  connect( mWidthUnitSelectionFixed, &QgsUnitSelectionWidget::changed, this, &QgsInterpolatedLineSymbolLayerWidget::apply );
  connect( mWidthUnitSelectionVarying, &QgsUnitSelectionWidget::changed, this, &QgsInterpolatedLineSymbolLayerWidget::apply );

  connect( mWidthUnitSelectionVarying, &QgsUnitSelectionWidget::changed, this, [this]
  {
    whileBlocking( mWidthUnitSelectionFixed )->setUnit( mWidthUnitSelectionVarying->unit() );
  } );

  connect( mWidthUnitSelectionFixed, &QgsUnitSelectionWidget::changed, this, [this]
  {
    whileBlocking( mWidthUnitSelectionVarying )->setUnit( mWidthUnitSelectionFixed->unit() );
  } );

  connect( mCheckBoxAbsoluteValue, &QCheckBox::clicked, this, &QgsInterpolatedLineSymbolLayerWidget::apply );
  connect( mCheckBoxOutOfrange, &QCheckBox::clicked, this, &QgsInterpolatedLineSymbolLayerWidget::apply );

  // Color parameter
  connect( mColorMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ),
           this, &QgsInterpolatedLineSymbolLayerWidget::apply );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this,  &QgsInterpolatedLineSymbolLayerWidget::apply );
  connect( mColorButton, &QgsColorButton::colorChanged, this,  &QgsInterpolatedLineSymbolLayerWidget::apply );

  connect( mColorStartFieldExpression, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged )
           , this, &QgsInterpolatedLineSymbolLayerWidget::reloadMinMaxColorFromLayer );
  connect( mColorEndFieldExpression, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged )
           , this, &QgsInterpolatedLineSymbolLayerWidget::reloadMinMaxColorFromLayer );

  connect( mLineEditColorMinValue, &QLineEdit::textChanged, this, &QgsInterpolatedLineSymbolLayerWidget::onColorMinMaxLineTextChanged );
  connect( mLineEditColorMinValue, &QLineEdit::textEdited, this, &QgsInterpolatedLineSymbolLayerWidget::onColorMinMaxLineTextEdited );
  connect( mLineEditColorMaxValue, &QLineEdit::textChanged, this, &QgsInterpolatedLineSymbolLayerWidget::onColorMinMaxLineTextChanged );
  connect( mLineEditColorMaxValue, &QLineEdit::textEdited, this, &QgsInterpolatedLineSymbolLayerWidget::onColorMinMaxLineTextEdited );
  connect( mButtonLoadMinMaxValueColor, &QPushButton::clicked, this, &QgsInterpolatedLineSymbolLayerWidget::onReloadMinMaxValueColor );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsInterpolatedLineSymbolLayerWidget::apply );

}

void QgsInterpolatedLineSymbolLayerWidget::setSymbolLayer( QgsSymbolLayer *layer )
{
  if ( !layer || layer->layerType() != QLatin1String( "InterpolatedLine" ) )
    return;

  mLayer = static_cast<QgsInterpolatedLineSymbolLayer *>( layer );

  const QgsInterpolatedLineWidth interpolatedWidth = mLayer->interpolatedWidth();
  whileBlocking( mWidthMethodComboBox )->setCurrentIndex( mWidthMethodComboBox->findData( interpolatedWidth.isVariableWidth() ) );

  whileBlocking( mDoubleSpinBoxWidth )->setValue( interpolatedWidth.fixedStrokeWidth() );
  whileBlocking( mWidthStartFieldExpression )->setExpression( mLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyLineStartWidthValue ).asExpression() );
  whileBlocking( mWidthEndFieldExpression )->setExpression( mLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyLineEndWidthValue ).asExpression() );
  setLineEditValue( mLineEditWidthMinValue, interpolatedWidth.minimumValue() );
  setLineEditValue( mLineEditWidthMaxValue, interpolatedWidth.maximumValue() );
  whileBlocking( mDoubleSpinBoxMinWidth )->setValue( interpolatedWidth.minimumWidth() );
  whileBlocking( mDoubleSpinBoxMaxWidth )->setValue( interpolatedWidth.maximumWidth() );
  whileBlocking( mWidthUnitSelectionFixed )->setUnit( mLayer->widthUnit() );
  whileBlocking( mWidthUnitSelectionVarying )->setUnit( mLayer->widthUnit() );
  whileBlocking( mCheckBoxAbsoluteValue )->setChecked( interpolatedWidth.useAbsoluteValue() );
  whileBlocking( mCheckBoxOutOfrange )->setChecked( interpolatedWidth.ignoreOutOfRange() );

  const QgsInterpolatedLineColor interpolatedColor = mLayer->interpolatedColor();
  whileBlocking( mColorMethodComboBox )->setCurrentIndex( mColorMethodComboBox->findData( interpolatedColor.coloringMethod() ) );

  whileBlocking( mColorStartFieldExpression )->setExpression( mLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyLineStartColorValue ).asExpression() );
  whileBlocking( mColorEndFieldExpression )->setExpression( mLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyLineEndColorValue ).asExpression() );
  whileBlocking( mColorRampShaderWidget )->setFromShader( interpolatedColor.colorRampShader() );
  setLineEditValue( mLineEditColorMinValue, interpolatedColor.colorRampShader().minimumValue() );
  setLineEditValue( mLineEditColorMaxValue, interpolatedColor.colorRampShader().maximumValue() );
  whileBlocking( mColorButton )->setColor( interpolatedColor.singleColor() );


  updateVisibleWidget();
}

QgsSymbolLayer *QgsInterpolatedLineSymbolLayerWidget::symbolLayer()
{
  if ( !mLayer )
    return nullptr;

  return mLayer;
}

void QgsInterpolatedLineSymbolLayerWidget::apply()
{
  if ( !mLayer )
    return;

  bool isExpression = false;
  QString fieldOrExpression = mWidthStartFieldExpression->currentField( &isExpression );
  mLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyLineStartWidthValue, isExpression ? QgsProperty::fromExpression( fieldOrExpression ) : QgsProperty::fromField( fieldOrExpression ) );
  fieldOrExpression = mWidthEndFieldExpression->currentField( &isExpression );
  mLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyLineEndWidthValue, isExpression ? QgsProperty::fromExpression( fieldOrExpression ) : QgsProperty::fromField( fieldOrExpression ) );

  mLayer->setInterpolatedWidth( interpolatedLineWidth() );
  if ( mWidthMethodComboBox->currentData().toBool() )
    mLayer->setWidthUnit( mWidthUnitSelectionVarying->unit() );
  else
    mLayer->setWidthUnit( mWidthUnitSelectionFixed->unit() );

  fieldOrExpression = mColorStartFieldExpression->currentField( &isExpression );
  mLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyLineStartColorValue, isExpression ? QgsProperty::fromExpression( fieldOrExpression ) : QgsProperty::fromField( fieldOrExpression ) );
  fieldOrExpression = mColorEndFieldExpression->currentField( &isExpression );
  mLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyLineEndColorValue, isExpression ? QgsProperty::fromExpression( fieldOrExpression ) : QgsProperty::fromField( fieldOrExpression ) );

  mLayer->setInterpolatedColor( interpolatedLineColor() );

  emit changed();
}

void QgsInterpolatedLineSymbolLayerWidget::updateVisibleWidget()
{
  mFixedWidthWidget->setVisible( !mWidthMethodComboBox->currentData().toBool() );
  mVaryingWidthWidget->setVisible( mWidthMethodComboBox->currentData().toBool() );

  mFixedColorWidget->setVisible(
    static_cast<QgsInterpolatedLineColor::ColoringMethod>( mColorMethodComboBox->currentData().toInt() ) == QgsInterpolatedLineColor::SingleColor );
  mVaryingColorWidget->setVisible(
    static_cast<QgsInterpolatedLineColor::ColoringMethod>( mColorMethodComboBox->currentData().toInt() ) == QgsInterpolatedLineColor::ColorRamp );
}

void QgsInterpolatedLineSymbolLayerWidget::onReloadMinMaxValueWidth()
{
  reloadMinMaxWidthFromLayer();
  setLineEditValue( mLineEditWidthMinValue, mMinimumForWidthFromLayer );
  setLineEditValue( mLineEditWidthMaxValue, mMaximumForWidthFromLayer );
  apply();
}

void QgsInterpolatedLineSymbolLayerWidget::onReloadMinMaxValueColor()
{
  reloadMinMaxColorFromLayer();
  setLineEditValue( mLineEditColorMinValue, mMinimumForColorFromLayer );
  setLineEditValue( mLineEditColorMaxValue, mMaximumForColorFromLayer );
  onColorMinMaxLineTextEdited();
}

void QgsInterpolatedLineSymbolLayerWidget::reloadMinMaxWidthFromLayer()
{
  QgsExpressionContext expressionContext = createExpressionContext();

  QgsExpression expressionStart( mWidthStartFieldExpression->expression() );
  if ( !expressionStart.prepare( &expressionContext ) )
  {
    apply();
    return;
  }

  QgsExpression expressionEnd( mWidthEndFieldExpression->expression() );
  if ( !expressionEnd.prepare( &expressionContext ) )
  {
    apply();
    return;
  }

  if ( !mLayer || !vectorLayer() )
  {
    apply();
    return;
  }

  QgsFeatureIterator it = vectorLayer()->getFeatures();
  QgsFeature feat;
  mMinimumForWidthFromLayer = std::numeric_limits<double>::max();
  mMaximumForWidthFromLayer = -std::numeric_limits<double>::max();
  while ( it.nextFeature( feat ) )
  {
    expressionContext.setFeature( feat );
    double startValue = expressionStart.evaluate( &expressionContext ).toDouble();
    double endValue = expressionEnd.evaluate( &expressionContext ).toDouble();

    if ( mCheckBoxAbsoluteValue->isChecked() )
    {
      startValue = fabs( startValue );
      endValue = fabs( endValue );
    }

    if ( startValue < mMinimumForWidthFromLayer )
      mMinimumForWidthFromLayer = startValue;
    if ( startValue > mMaximumForWidthFromLayer )
      mMaximumForWidthFromLayer = startValue;

    if ( endValue < mMinimumForWidthFromLayer )
      mMinimumForWidthFromLayer = endValue;
    if ( endValue > mMaximumForWidthFromLayer )
      mMaximumForWidthFromLayer = endValue;
  }

  if ( mLineEditWidthMinValue->text().isEmpty() && !std::isnan( mMinimumForWidthFromLayer ) )
  {
    setLineEditValue( mLineEditWidthMinValue, mMinimumForWidthFromLayer );
  }

  if ( mLineEditWidthMaxValue->text().isEmpty() && !std::isnan( mMaximumForWidthFromLayer ) )
  {
    setLineEditValue( mLineEditWidthMaxValue, mMaximumForWidthFromLayer );
  }

  apply();
}

void QgsInterpolatedLineSymbolLayerWidget::reloadMinMaxColorFromLayer()
{
  QgsExpressionContext expressionContext = createExpressionContext();

  QgsExpression expressionStart( mColorStartFieldExpression->expression() );
  if ( !expressionStart.prepare( &expressionContext ) )
  {
    apply();
    return;
  }

  QgsExpression expressionEnd( mColorEndFieldExpression->expression() );
  if ( !expressionEnd.prepare( &expressionContext ) )
  {
    apply();
    return;
  }

  if ( !mLayer || !vectorLayer() )
  {
    apply();
    return;
  }

  QgsFeatureIterator it = vectorLayer()->getFeatures();
  QgsFeature feat;
  mMinimumForColorFromLayer = std::numeric_limits<double>::max();
  mMaximumForColorFromLayer = -std::numeric_limits<double>::max();
  while ( it.nextFeature( feat ) )
  {
    expressionContext.setFeature( feat );
    const double startValue = expressionStart.evaluate( &expressionContext ).toDouble();
    const double endValue = expressionEnd.evaluate( &expressionContext ).toDouble();

    if ( startValue < mMinimumForColorFromLayer )
      mMinimumForColorFromLayer = startValue;
    if ( startValue > mMaximumForColorFromLayer )
      mMaximumForColorFromLayer = startValue;

    if ( endValue < mMinimumForColorFromLayer )
      mMinimumForColorFromLayer = endValue;
    if ( endValue > mMaximumForColorFromLayer )
      mMaximumForColorFromLayer = endValue;
  }

  bool minMaxColorChanged = false;
  if ( mLineEditColorMinValue->text().isEmpty() && !std::isnan( mMinimumForColorFromLayer ) )
  {
    setLineEditValue( mLineEditColorMinValue, mMinimumForColorFromLayer );
    minMaxColorChanged = true;
  }

  if ( mLineEditColorMaxValue->text().isEmpty() && !std::isnan( mMaximumForColorFromLayer ) )
  {
    setLineEditValue( mLineEditColorMaxValue, mMaximumForColorFromLayer );
    minMaxColorChanged = true;
  }

  if ( minMaxColorChanged )
    onColorMinMaxLineTextEdited();

  apply();
}

void QgsInterpolatedLineSymbolLayerWidget::onColorMinMaxLineTextChanged()
{
  const double min = lineEditValue( mLineEditColorMinValue );
  const double max = lineEditValue( mLineEditColorMaxValue );
  whileBlocking( mColorRampShaderWidget )->setMinimumMaximum( min, max );
  apply();
}

void QgsInterpolatedLineSymbolLayerWidget::onColorMinMaxLineTextEdited()
{
  const double min = lineEditValue( mLineEditColorMinValue );
  const double max = lineEditValue( mLineEditColorMaxValue );
  whileBlocking( mColorRampShaderWidget )->setMinimumMaximumAndClassify( min, max );
  apply();
}

QgsInterpolatedLineWidth QgsInterpolatedLineSymbolLayerWidget::interpolatedLineWidth()
{
  QgsInterpolatedLineWidth interWidth;
  interWidth.setIsVariableWidth( mWidthMethodComboBox->currentData().toBool() );
  interWidth.setMinimumValue( lineEditValue( mLineEditWidthMinValue ) );
  interWidth.setMaximumValue( lineEditValue( mLineEditWidthMaxValue ) );
  interWidth.setMinimumWidth( mDoubleSpinBoxMinWidth->value() );
  interWidth.setMaximumWidth( mDoubleSpinBoxMaxWidth->value() );
  interWidth.setFixedStrokeWidth( mDoubleSpinBoxWidth->value() );
  interWidth.setIgnoreOutOfRange( mCheckBoxOutOfrange->isChecked() );
  interWidth.setUseAbsoluteValue( mCheckBoxAbsoluteValue->isChecked() );

  return interWidth;
}

QgsInterpolatedLineColor QgsInterpolatedLineSymbolLayerWidget::interpolatedLineColor()
{
  QgsInterpolatedLineColor interColor;
  interColor.setColor( mColorButton->color() );
  const QgsColorRampShader colorRampShader = mColorRampShaderWidget->shader();
  interColor.setColor( colorRampShader );
  interColor.setColoringMethod( static_cast<QgsInterpolatedLineColor::ColoringMethod>( mColorMethodComboBox->currentData().toInt() ) );

  return interColor;
}

double QgsInterpolatedLineSymbolLayerWidget::lineEditValue( QLineEdit *lineEdit )
{
  if ( lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return QgsDoubleValidator::toDouble( lineEdit->text() );
}

void QgsInterpolatedLineSymbolLayerWidget::setLineEditValue( QLineEdit *lineEdit, double value )
{
  QString strValue;
  if ( !std::isnan( value ) )
    strValue = QLocale().toString( value );
  whileBlocking( lineEdit )->setText( strValue );
}
