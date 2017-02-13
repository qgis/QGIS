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
#include "qgslayertreelayer.h"
#include "qgssymbollayerutils.h"

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

  if ( mLayer )
  {
    mLayerTreeLayer = new QgsLayerTreeLayer( const_cast< QgsVectorLayer* >( mLayer ) );
    mRoot.addChildNode( mLayerTreeLayer ); // takes ownership
  }
  mLegendPreview->setModel( &mPreviewList );
  mLegendPreview->setItemDelegate( new ItemDelegate( &mPreviewList ) );
  mLegendPreview->setHeaderHidden( true );
  mLegendPreview->expandAll();

  switch ( definition.standardTemplate() )
  {
    case QgsPropertyDefinition::Size:
    case QgsPropertyDefinition::StrokeWidth:
    {
      mTransformerWidget = new QgsPropertySizeAssistantWidget( this, mDefinition, initialState );
      break;
    }

    case QgsPropertyDefinition::ColorNoAlpha:
    case QgsPropertyDefinition::ColorWithAlpha:
    {
      mTransformerWidget = new QgsPropertyColorAssistantWidget( this, mDefinition, initialState );
      break;
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
  connect( this, &QgsPropertyAssistantWidget::widgetChanged, this, &QgsPropertyAssistantWidget::updatePreview );
  updatePreview();
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

void QgsPropertyAssistantWidget::updatePreview()
{
  if ( !mTransformerWidget || !mLayer ) // TODO - make this work OK without a layer
    return;

  mLegendPreview->setIconSize( QSize( 512, 512 ) );
  mPreviewList.clear();

  QList<double> breaks = QgsSymbolLayerUtils::prettyBreaks( minValueSpinBox->value(),
                         maxValueSpinBox->value(), 4 );

  QList< QgsSymbolLegendNode* > nodes = mTransformerWidget->generatePreviews( breaks, mLayerTreeLayer, mSymbol.get(), minValueSpinBox->value(),
                                        maxValueSpinBox->value() );
  if ( nodes.isEmpty() )
  {
    mLegendPreview->show();
    return;
  }

  int widthMax = 0;
  int i = 0;
  Q_FOREACH ( QgsSymbolLegendNode* node, nodes )
  {
    const QSize minSize( node->minimumIconSize() );
    node->setIconSize( minSize );
    widthMax = qMax( minSize.width(), widthMax );
    mPreviewList.appendRow( new QStandardItem( node->data( Qt::DecorationRole ).value<QPixmap>(), QString::number( breaks[i] ) ) );
    delete node;
    i++;
  }
  // center icon and align text left by giving icons the same width
  // @todo maybe add some space so that icons don't touch
  for ( int i = 0; i < breaks.length(); i++ )
  {
    QPixmap img( mPreviewList.item( i )->icon().pixmap( mPreviewList.item( i )->icon().actualSize( QSize( 512, 512 ) ) ) );
    QPixmap enlarged( widthMax, img.height() );
    // fill transparent and add original image
    enlarged.fill( Qt::transparent );
    QPainter p( &enlarged );
    p.drawPixmap( QPoint(( widthMax - img.width() ) / 2, 0 ), img );
    p.end();
    mPreviewList.item( i )->setIcon( enlarged );
  }
  mLegendPreview->show();
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

  if ( definition.standardTemplate() == QgsPropertyDefinition::Size )
  {
    scaleMethodComboBox->addItem( tr( "Flannery" ), QgsSizeScaleTransformer::Flannery );
    scaleMethodComboBox->addItem( tr( "Surface" ), QgsSizeScaleTransformer::Area );
    scaleMethodComboBox->addItem( tr( "Radius" ), QgsSizeScaleTransformer::Linear );
    scaleMethodComboBox->addItem( tr( "Exponential" ), QgsSizeScaleTransformer::Exponential );
  }
  else if ( definition.standardTemplate() == QgsPropertyDefinition::StrokeWidth )
  {
    scaleMethodComboBox->addItem( tr( "Exponential" ), QgsSizeScaleTransformer::Exponential );
    scaleMethodComboBox->addItem( tr( "Linear" ), QgsSizeScaleTransformer::Linear );
  }

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

QgsSizeScaleTransformer* QgsPropertySizeAssistantWidget::createTransformer( double minValue, double maxValue ) const
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

QList< QgsSymbolLegendNode* > QgsPropertySizeAssistantWidget::generatePreviews( const QList<double>& breaks, QgsLayerTreeLayer* parent, const QgsSymbol* symbol, double minValue, double maxValue ) const
{
  QList< QgsSymbolLegendNode* > nodes;

  const QgsSymbol* legendSymbol = symbol;
  std::unique_ptr< QgsSymbol > tempSymbol;

  if ( !legendSymbol )
  {
    if ( mDefinition.standardTemplate() == QgsPropertyDefinition::Size )
    {
      tempSymbol.reset( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
    }
    else if ( mDefinition.standardTemplate() == QgsPropertyDefinition::StrokeWidth )
    {
      tempSymbol.reset( QgsLineSymbol::createSimple( QgsStringMap() ) );
    }
    legendSymbol = tempSymbol.get();
  }
  if ( !legendSymbol )
    return nodes;

  std::unique_ptr< QgsSizeScaleTransformer > t( createTransformer( minValue, maxValue ) );

  for ( int i = 0; i < breaks.length(); i++ )
  {
    std::unique_ptr< QgsSymbolLegendNode > node;
    if ( dynamic_cast<const QgsMarkerSymbol*>( legendSymbol ) )
    {
      std::unique_ptr< QgsMarkerSymbol > symbolClone( static_cast<QgsMarkerSymbol*>( legendSymbol->clone() ) );
      symbolClone->setDataDefinedSize( QgsProperty() );
      symbolClone->setDataDefinedAngle( QgsProperty() ); // to avoid symbol not being drawn
      symbolClone->setSize( t->size( breaks[i] ) );
      node.reset( new QgsSymbolLegendNode( parent, QgsLegendSymbolItem( symbolClone.get(), QString::number( i ), QString() ) ) );
    }
    else if ( dynamic_cast<const QgsLineSymbol*>( legendSymbol ) )
    {
      std::unique_ptr< QgsLineSymbol > symbolClone( static_cast<QgsLineSymbol*>( legendSymbol->clone() ) );
      symbolClone->setDataDefinedWidth( QgsProperty() );
      symbolClone->setWidth( t->size( breaks[i] ) );
      node.reset( new QgsSymbolLegendNode( parent, QgsLegendSymbolItem( symbolClone.get(), QString::number( i ), QString() ) ) );
    }
    if ( node )
      nodes << node.release();
  }
  return nodes;
}

QList<QgsSymbolLegendNode*> QgsPropertyAbstractTransformerWidget::generatePreviews( const QList<double>& , QgsLayerTreeLayer* , const QgsSymbol*, double, double ) const
{
  return QList< QgsSymbolLegendNode* >();
}

QgsPropertyColorAssistantWidget::QgsPropertyColorAssistantWidget( QWidget* parent, const QgsPropertyDefinition& definition, const QgsProperty& initialState )
    : QgsPropertyAbstractTransformerWidget( parent, definition )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );
  layout()->setMargin( 0 );

  bool supportsAlpha = definition.standardTemplate() == QgsPropertyDefinition::ColorWithAlpha;
  mNullColorButton->setAllowAlpha( supportsAlpha );

  if ( const QgsColorRampTransformer* colorTransform = dynamic_cast< const QgsColorRampTransformer* >( initialState.transformer() ) )
  {
    mNullColorButton->setColor( colorTransform->nullColor() );
    mColorRampButton->setColorRamp( colorTransform->colorRamp() );
  }

  connect( mNullColorButton, &QgsColorButton::colorChanged, this, &QgsPropertyColorAssistantWidget::widgetChanged );
  connect( mColorRampButton, &QgsColorRampButton::colorRampChanged, this, &QgsPropertyColorAssistantWidget::widgetChanged );
}

QgsColorRampTransformer* QgsPropertyColorAssistantWidget::createTransformer( double minValue, double maxValue ) const
{
  QgsColorRampTransformer* transformer = new QgsColorRampTransformer(
    minValue,
    maxValue,
    mColorRampButton->colorRamp(),
    mNullColorButton->color() );
  return transformer;
}

QList<QgsSymbolLegendNode*> QgsPropertyColorAssistantWidget::generatePreviews( const QList<double>& breaks, QgsLayerTreeLayer* parent, const QgsSymbol* symbol, double minValue, double maxValue ) const
{
  QList< QgsSymbolLegendNode* > nodes;

  const QgsMarkerSymbol* legendSymbol = dynamic_cast<const QgsMarkerSymbol*>( symbol );
  std::unique_ptr< QgsMarkerSymbol > tempSymbol;

  if ( !legendSymbol )
  {
    tempSymbol.reset( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
    legendSymbol = tempSymbol.get();
  }
  if ( !legendSymbol )
    return nodes;

  std::unique_ptr< QgsColorRampTransformer > t( createTransformer( minValue, maxValue ) );

  for ( int i = 0; i < breaks.length(); i++ )
  {
    std::unique_ptr< QgsSymbolLegendNode > node;
    std::unique_ptr< QgsMarkerSymbol > symbolClone( static_cast<QgsMarkerSymbol*>( legendSymbol->clone() ) );
    symbolClone->setColor( t->color( breaks[i] ) );
    node.reset( new QgsSymbolLegendNode( parent, QgsLegendSymbolItem( symbolClone.get(), QString::number( i ), QString() ) ) );
    if ( node )
      nodes << node.release();
  }
  return nodes;
}
