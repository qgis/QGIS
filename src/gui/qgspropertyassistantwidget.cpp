/***************************************************************************
    qgspropertyassistantwidget.cpp
    ------------------------------
    begin                : February, 2017
    copyright            : (C) 2017 Nyall Dawson
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

#include "qgspropertyassistantwidget.h"
#include "qgsproject.h"
#include "qgsmapsettings.h"
#include "qgsvectorlayer.h"

QgsPropertyAssistantWidget::QgsPropertyAssistantWidget( QWidget* parent ,
    const QgsPropertyDefinition& definition, const QgsProperty& initialState,
    const QgsVectorLayer* layer )
    : QgsPanelWidget( parent )
    , mDefinition( definition )
    , mLayer( layer )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );
  layout()->setMargin( 0 );

  setPanelTitle( mDefinition.description() );

  mLegendPreview->hide();

  minValueSpinBox->setShowClearButton( false );
  maxValueSpinBox->setShowClearButton( false );

  // TODO expression widget shouldn't require a non-const layer
  mExpressionWidget->setLayer( const_cast< QgsVectorLayer* >( mLayer ) );
  mExpressionWidget->setFilters( QgsFieldProxyModel::Numeric );
  mExpressionWidget->setField( initialState.propertyType() == QgsProperty::ExpressionBasedProperty ? initialState.expressionString() : initialState.field() );

  if ( initialState.transformer() )
  {
    minValueSpinBox->setValue( initialState.transformer()->minValue() );
    maxValueSpinBox->setValue( initialState.transformer()->maxValue() );
  }

  connect( computeValuesButton, &QPushButton::clicked, this, &QgsPropertyAssistantWidget::computeValuesFromLayer );

  switch ( definition.standardTemplate() )
  {
    case QgsPropertyDefinition::Size:
    {
      mTransformerWidget = new QgsPropertySizeAssistantWidget( this, mDefinition, initialState );
    }
    default:
      break;
  }

  if ( mTransformerWidget )
  {
    mOutputWidget->layout()->addWidget( mTransformerWidget );
    connect( mTransformerWidget, &QgsPropertyAbstractTransformerWidget::widgetChanged, this, &QgsPropertyAssistantWidget::widgetChanged );
  }

  connect( minValueSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertyAssistantWidget::widgetChanged );
  connect( maxValueSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertyAssistantWidget::widgetChanged );
  connect( mExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString& ) > ( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsPropertyAssistantWidget::widgetChanged );
}

void QgsPropertyAssistantWidget::registerExpressionContextGenerator( QgsExpressionContextGenerator* generator )
{
  mExpressionContextGenerator = generator;
  mExpressionWidget->registerExpressionContextGenerator( generator );
}

void QgsPropertyAssistantWidget::updateProperty( QgsProperty& property )
{
  property.setActive( !mExpressionWidget->currentText().isEmpty() );
  if ( mExpressionWidget->isExpression() )
    property.setExpressionString( mExpressionWidget->currentField() );
  else
    property.setField( mExpressionWidget->currentField() );

  if ( mTransformerWidget )
    property.setTransformer( mTransformerWidget->createTransformer( minValueSpinBox->value(), maxValueSpinBox->value() ) );
}

void QgsPropertyAssistantWidget::computeValuesFromLayer()
{
  if ( !mLayer )
    return;

  double minValue = 0.0;
  double maxValue = 0.0;

  if ( mExpressionWidget->isExpression() )
  {
    if ( !computeValuesFromExpression( mExpressionWidget->currentField(), minValue, maxValue ) )
      return;
  }
  else
  {
    if ( !computeValuesFromField( mExpressionWidget->currentField(), minValue, maxValue ) )
      return;
  }

  whileBlocking( minValueSpinBox )->setValue( minValue );
  whileBlocking( maxValueSpinBox )->setValue( maxValue );
  emit widgetChanged();
}

bool QgsPropertyAssistantWidget::computeValuesFromExpression( const QString& expression, double& minValue, double& maxValue ) const
{
  QgsExpression e( expression );

  QgsExpressionContext context;
  if ( mExpressionContextGenerator )
  {
    context = mExpressionContextGenerator->createExpressionContext();
  }
  else
  {
    context << QgsExpressionContextUtils::globalScope()
    << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
    << QgsExpressionContextUtils::layerScope( mLayer );
  }

  if ( !e.prepare( &context ) )
    return false;

  QSet<QString> referencedCols( e.referencedColumns() );

  QgsFeatureIterator fit = mLayer->getFeatures(
                             QgsFeatureRequest().setFlags( e.needsGeometry()
                                                           ? QgsFeatureRequest::NoFlags
                                                           : QgsFeatureRequest::NoGeometry )
                             .setSubsetOfAttributes( referencedCols, mLayer->fields() ) );

  // create list of non-null attribute values
  double min = DBL_MAX;
  double max = -DBL_MAX;
  QgsFeature f;
  bool found = false;
  while ( fit.nextFeature( f ) )
  {
    bool ok;
    context.setFeature( f );
    const double value = e.evaluate( &context ).toDouble( &ok );
    if ( ok )
    {
      max = qMax( max, value );
      min = qMin( min, value );
      found = true;
    }
  }
  if ( found )
  {
    minValue = min;
    maxValue = max;
  }
  return found;
}

bool QgsPropertyAssistantWidget::computeValuesFromField( const QString& fieldName, double& minValue, double& maxValue ) const
{
  int fieldIndex = mLayer->fields().lookupField( fieldName );
  if ( fieldIndex < 0 )
  {
    return false;
  }

  bool ok = false;
  double minDouble = mLayer->minimumValue( fieldIndex ).toDouble( &ok );
  if ( !ok )
    return false;

  double maxDouble = mLayer->maximumValue( fieldIndex ).toDouble( &ok );
  if ( !ok )
    return false;

  minValue = minDouble;
  maxValue = maxDouble;
  return true;
}


//
// QgsPropertySizeAssistantWidget
//

QgsPropertySizeAssistantWidget::QgsPropertySizeAssistantWidget( QWidget* parent, const QgsPropertyDefinition& definition, const QgsProperty& initialState )
    : QgsPropertyAbstractTransformerWidget( parent, definition )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );
  layout()->setMargin( 0 );

  scaleMethodComboBox->addItem( tr( "Flannery" ), QgsSizeScaleTransformer::Flannery );
  scaleMethodComboBox->addItem( tr( "Surface" ), QgsSizeScaleTransformer::Area );
  scaleMethodComboBox->addItem( tr( "Radius" ), QgsSizeScaleTransformer::Linear );
  scaleMethodComboBox->addItem( tr( "Exponential" ), QgsSizeScaleTransformer::Exponential );

  minSizeSpinBox->setShowClearButton( false );
  maxSizeSpinBox->setShowClearButton( false );
  nullSizeSpinBox->setShowClearButton( false );

  if ( const QgsSizeScaleTransformer* sizeTransform = dynamic_cast< const QgsSizeScaleTransformer* >( initialState.transformer() ) )
  {
    minSizeSpinBox->setValue( sizeTransform->minSize() );
    maxSizeSpinBox->setValue( sizeTransform->maxSize() );
    nullSizeSpinBox->setValue( sizeTransform->nullSize() );
    exponentSpinBox->setValue( sizeTransform->exponent() );
    scaleMethodComboBox->setCurrentIndex( scaleMethodComboBox->findData( sizeTransform->type() ) );
  }

  exponentSpinBox->setEnabled( scaleMethodComboBox->currentData().toInt() == QgsSizeScaleTransformer::Exponential );

  connect( minSizeSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertySizeAssistantWidget::widgetChanged );
  connect( maxSizeSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertySizeAssistantWidget::widgetChanged );
  connect( nullSizeSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertySizeAssistantWidget::widgetChanged );
  connect( exponentSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertySizeAssistantWidget::widgetChanged );
  connect( scaleMethodComboBox, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::currentIndexChanged ), this, &QgsPropertySizeAssistantWidget::widgetChanged );
  connect( scaleMethodComboBox, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::currentIndexChanged ), this,
           [=]
  {
    exponentSpinBox->setEnabled( scaleMethodComboBox->currentData().toInt() == QgsSizeScaleTransformer::Exponential );
  }
         );
}

QgsPropertyTransformer* QgsPropertySizeAssistantWidget::createTransformer( double minValue, double maxValue ) const
{
  QgsSizeScaleTransformer* transformer = new QgsSizeScaleTransformer(
    static_cast< QgsSizeScaleTransformer::ScaleType >( scaleMethodComboBox->currentData().toInt() ),
    minValue,
    maxValue,
    minSizeSpinBox->value(),
    maxSizeSpinBox->value(),
    nullSizeSpinBox->value(),
    exponentSpinBox->value() );
  return transformer;
}
