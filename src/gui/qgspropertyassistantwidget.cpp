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
#include "qgsexpressioncontextutils.h"
#include "qgsstyle.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"

QgsPropertyAssistantWidget::QgsPropertyAssistantWidget( QWidget *parent,
    const QgsPropertyDefinition &definition, const QgsProperty &initialState,
    const QgsVectorLayer *layer )
  : QgsPanelWidget( parent )
  , mDefinition( definition )
  , mLayer( layer )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  setPanelTitle( mDefinition.description() );

  mLegendPreview->hide();

  minValueSpinBox->setShowClearButton( false );
  maxValueSpinBox->setShowClearButton( false );

  // TODO expression widget shouldn't require a non-const layer
  mExpressionWidget->setLayer( const_cast< QgsVectorLayer * >( mLayer ) );
  mExpressionWidget->setFilters( QgsFieldProxyModel::Numeric );
  mExpressionWidget->setField( initialState.propertyType() == QgsProperty::ExpressionBasedProperty ? initialState.expressionString() : initialState.field() );

  if ( auto *lTransformer = initialState.transformer() )
  {
    minValueSpinBox->setValue( lTransformer->minValue() );
    maxValueSpinBox->setValue( lTransformer->maxValue() );

    if ( lTransformer->curveTransform() )
    {
      mTransformCurveCheckBox->setChecked( true );
      mTransformCurveCheckBox->setCollapsed( false );
      mCurveEditor->setCurve( *lTransformer->curveTransform() );
    }
  }

  connect( computeValuesButton, &QPushButton::clicked, this, &QgsPropertyAssistantWidget::computeValuesFromLayer );

  if ( mLayer )
  {
    mLayerTreeLayer = new QgsLayerTreeLayer( const_cast< QgsVectorLayer * >( mLayer ) );
    mRoot.addChildNode( mLayerTreeLayer ); // takes ownership
  }
  mLegendPreview->setModel( &mPreviewList );
  mLegendPreview->setItemDelegate( new QgsAssistantPreviewItemDelegate( &mPreviewList ) );
  mLegendPreview->setHeaderHidden( true );
  mLegendPreview->expandAll();
  mLegendVerticalFrame->setLayout( new QVBoxLayout() );
  mLegendVerticalFrame->layout()->setContentsMargins( 0, 0, 0, 0 );
  mLegendVerticalFrame->hide();

  switch ( definition.standardTemplate() )
  {
    case QgsPropertyDefinition::Size:
    case QgsPropertyDefinition::StrokeWidth:
    {
      mTransformerWidget = new QgsPropertySizeAssistantWidget( this, mDefinition, initialState );
      mLegendPreview->show();
      break;
    }

    case QgsPropertyDefinition::ColorNoAlpha:
    case QgsPropertyDefinition::ColorWithAlpha:
    {
      mTransformerWidget = new QgsPropertyColorAssistantWidget( this, mDefinition, initialState );
      mLegendPreview->show();
      break;
    }

    case QgsPropertyDefinition::Rotation:
    {
      mTransformerWidget = new QgsPropertyGenericNumericAssistantWidget( this, mDefinition, initialState );
      break;
    }

    default:
    {
      if ( mDefinition.dataType() == QgsPropertyDefinition::DataTypeNumeric )
      {
        mTransformerWidget = new QgsPropertyGenericNumericAssistantWidget( this, mDefinition, initialState );
      }
      break;
    }
  }

  if ( mTransformerWidget )
  {
    mOutputWidget->layout()->addWidget( mTransformerWidget );
    connect( mTransformerWidget, &QgsPropertyAbstractTransformerWidget::widgetChanged, this, &QgsPropertyAssistantWidget::widgetChanged );

    mCurveEditor->setMinHistogramValueRange( minValueSpinBox->value() );
    mCurveEditor->setMaxHistogramValueRange( maxValueSpinBox->value() );

    mCurveEditor->setHistogramSource( mLayer, mExpressionWidget->currentField() );
    connect( mExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) > ( &QgsFieldExpressionWidget::fieldChanged ), this, [ = ]( const QString & expression )
    {
      mCurveEditor->setHistogramSource( mLayer, expression );
    }
           );
    connect( minValueSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), mCurveEditor, &QgsCurveEditorWidget::setMinHistogramValueRange );
    connect( maxValueSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), mCurveEditor, &QgsCurveEditorWidget::setMaxHistogramValueRange );
  }
  mTransformCurveCheckBox->setVisible( mTransformerWidget );

  connect( minValueSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertyAssistantWidget::widgetChanged );
  connect( maxValueSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertyAssistantWidget::widgetChanged );
  connect( mExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) > ( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsPropertyAssistantWidget::widgetChanged );
  connect( mCurveEditor, &QgsCurveEditorWidget::changed, this, &QgsPropertyAssistantWidget::widgetChanged );
  connect( this, &QgsPropertyAssistantWidget::widgetChanged, this, &QgsPropertyAssistantWidget::updatePreview );
  updatePreview();
}

void QgsPropertyAssistantWidget::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator = generator;
  mExpressionWidget->registerExpressionContextGenerator( generator );
}

void QgsPropertyAssistantWidget::updateProperty( QgsProperty &property )
{
  property.setActive( !mExpressionWidget->currentText().isEmpty() );
  if ( mExpressionWidget->isExpression() )
    property.setExpressionString( mExpressionWidget->currentField() );
  else
    property.setField( mExpressionWidget->currentField() );

  if ( mTransformerWidget )
  {
    std::unique_ptr< QgsPropertyTransformer> t( mTransformerWidget->createTransformer( minValueSpinBox->value(), maxValueSpinBox->value() ) );
    if ( mTransformCurveCheckBox->isChecked() )
    {
      t->setCurveTransform( new QgsCurveTransform( mCurveEditor->curve() ) );
    }
    else
    {
      t->setCurveTransform( nullptr );
    }
    property.setTransformer( t.release() );
  }
}

void QgsPropertyAssistantWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );

  if ( dockMode && mLegendVerticalFrame->isHidden() )
  {
    mLegendVerticalFrame->layout()->addWidget( mLegendPreview );
    mLegendVerticalFrame->show();
  }
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

  mCurveEditor->setMinHistogramValueRange( minValueSpinBox->value() );
  mCurveEditor->setMaxHistogramValueRange( maxValueSpinBox->value() );

  emit widgetChanged();
}

void QgsPropertyAssistantWidget::updatePreview()
{
  if ( mLegendPreview->isHidden() || !mTransformerWidget || !mLayer ) // TODO - make this work OK without a layer
    return;

  mLegendPreview->setIconSize( QSize( 512, 512 ) );
  mPreviewList.clear();

  QList<double> breaks = QgsSymbolLayerUtils::prettyBreaks( minValueSpinBox->value(),
                         maxValueSpinBox->value(), 8 );

  QgsCurveTransform curve = mCurveEditor->curve();
  const QList< QgsSymbolLegendNode * > nodes = mTransformerWidget->generatePreviews( breaks, mLayerTreeLayer, mSymbol.get(), minValueSpinBox->value(),
      maxValueSpinBox->value(), mTransformCurveCheckBox->isChecked() ? &curve : nullptr );

  int widthMax = 0;
  int i = 0;
  const auto constNodes = nodes;
  for ( QgsSymbolLegendNode *node : constNodes )
  {
    const QSize minSize( node->minimumIconSize() );
    node->setIconSize( minSize );
    widthMax = std::max( minSize.width(), widthMax );
    QStandardItem *item = new QStandardItem( node->data( Qt::DecorationRole ).value<QPixmap>(), QLocale().toString( breaks[i] ) );
    item->setEditable( false );
    mPreviewList.appendRow( item );
    delete node;
    i++;
  }
  // center icon and align text left by giving icons the same width
  // TODO maybe add some space so that icons don't touch
  for ( int i = 0; i < breaks.length(); i++ )
  {
    const QPixmap img( mPreviewList.item( i )->icon().pixmap( mPreviewList.item( i )->icon().actualSize( QSize( 512, 512 ) ) ) );
    QPixmap enlarged( widthMax, img.height() );
    // fill transparent and add original image
    enlarged.fill( Qt::transparent );
    QPainter p( &enlarged );
    p.drawPixmap( QPoint( ( widthMax - img.width() ) / 2, 0 ), img );
    p.end();
    mPreviewList.item( i )->setIcon( enlarged );
  }
}

bool QgsPropertyAssistantWidget::computeValuesFromExpression( const QString &expression, double &minValue, double &maxValue ) const
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

  const QSet<QString> referencedCols( e.referencedColumns() );

  QgsFeatureIterator fit = mLayer->getFeatures(
                             QgsFeatureRequest().setFlags( e.needsGeometry()
                                 ? QgsFeatureRequest::NoFlags
                                 : QgsFeatureRequest::NoGeometry )
                             .setSubsetOfAttributes( referencedCols, mLayer->fields() ) );

  // create list of non-null attribute values
  double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::lowest();
  QgsFeature f;
  bool found = false;
  while ( fit.nextFeature( f ) )
  {
    bool ok;
    context.setFeature( f );
    const double value = e.evaluate( &context ).toDouble( &ok );
    if ( ok )
    {
      max = std::max( max, value );
      min = std::min( min, value );
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

bool QgsPropertyAssistantWidget::computeValuesFromField( const QString &fieldName, double &minValue, double &maxValue ) const
{
  const int fieldIndex = mLayer->fields().lookupField( fieldName );
  if ( fieldIndex < 0 )
  {
    return false;
  }

  QVariant min;
  QVariant max;
  mLayer->minimumAndMaximumValue( fieldIndex, min, max );

  bool ok = false;
  const double minDouble = min.toDouble( &ok );
  if ( !ok )
    return false;

  const double maxDouble = max.toDouble( &ok );
  if ( !ok )
    return false;

  minValue = minDouble;
  maxValue = maxDouble;
  return true;
}

///@cond PRIVATE

//
// QgsPropertySizeAssistantWidget
//

QgsPropertySizeAssistantWidget::QgsPropertySizeAssistantWidget( QWidget *parent, const QgsPropertyDefinition &definition, const QgsProperty &initialState )
  : QgsPropertyAbstractTransformerWidget( parent, definition )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

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

  if ( const QgsSizeScaleTransformer *sizeTransform = dynamic_cast< const QgsSizeScaleTransformer * >( initialState.transformer() ) )
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
           [ = ]
  {
    exponentSpinBox->setEnabled( scaleMethodComboBox->currentData().toInt() == QgsSizeScaleTransformer::Exponential );
  }
         );
}

QgsSizeScaleTransformer *QgsPropertySizeAssistantWidget::createTransformer( double minValue, double maxValue ) const
{
  QgsSizeScaleTransformer *transformer = new QgsSizeScaleTransformer(
    static_cast< QgsSizeScaleTransformer::ScaleType >( scaleMethodComboBox->currentData().toInt() ),
    minValue,
    maxValue,
    minSizeSpinBox->value(),
    maxSizeSpinBox->value(),
    nullSizeSpinBox->value(),
    exponentSpinBox->value() );
  return transformer;
}

QList< QgsSymbolLegendNode * > QgsPropertySizeAssistantWidget::generatePreviews( const QList<double> &breaks, QgsLayerTreeLayer *parent, const QgsSymbol *symbol, double minValue, double maxValue, QgsCurveTransform *curve ) const
{
  QList< QgsSymbolLegendNode * > nodes;

  const QgsSymbol *legendSymbol = symbol;
  std::unique_ptr< QgsSymbol > tempSymbol;

  if ( !legendSymbol )
  {
    if ( mDefinition.standardTemplate() == QgsPropertyDefinition::Size )
    {
      tempSymbol.reset( QgsMarkerSymbol::createSimple( QVariantMap() ) );
    }
    else if ( mDefinition.standardTemplate() == QgsPropertyDefinition::StrokeWidth )
    {
      tempSymbol.reset( QgsLineSymbol::createSimple( QVariantMap() ) );
    }
    legendSymbol = tempSymbol.get();
  }
  if ( !legendSymbol )
    return nodes;

  std::unique_ptr< QgsSizeScaleTransformer > t( createTransformer( minValue, maxValue ) );
  if ( curve )
    t->setCurveTransform( new QgsCurveTransform( *curve ) );

  for ( int i = 0; i < breaks.length(); i++ )
  {
    std::unique_ptr< QgsSymbolLegendNode > node;
    if ( dynamic_cast<const QgsMarkerSymbol *>( legendSymbol ) )
    {
      std::unique_ptr< QgsMarkerSymbol > symbolClone( static_cast<QgsMarkerSymbol *>( legendSymbol->clone() ) );
      symbolClone->setDataDefinedSize( QgsProperty() );
      symbolClone->setDataDefinedAngle( QgsProperty() ); // to avoid symbol not being drawn
      symbolClone->setSize( t->size( breaks[i] ) );
      node.reset( new QgsSymbolLegendNode( parent, QgsLegendSymbolItem( symbolClone.get(), QString::number( i ), QString() ) ) );
    }
    else if ( dynamic_cast<const QgsLineSymbol *>( legendSymbol ) )
    {
      std::unique_ptr< QgsLineSymbol > symbolClone( static_cast<QgsLineSymbol *>( legendSymbol->clone() ) );
      symbolClone->setDataDefinedWidth( QgsProperty() );
      symbolClone->setWidth( t->size( breaks[i] ) );
      node.reset( new QgsSymbolLegendNode( parent, QgsLegendSymbolItem( symbolClone.get(), QString::number( i ), QString() ) ) );
    }
    if ( node )
      nodes << node.release();
  }
  return nodes;
}

QList<QgsSymbolLegendNode *> QgsPropertyAbstractTransformerWidget::generatePreviews( const QList<double> &, QgsLayerTreeLayer *, const QgsSymbol *, double, double, QgsCurveTransform * ) const
{
  return QList< QgsSymbolLegendNode * >();
}

QgsPropertyColorAssistantWidget::QgsPropertyColorAssistantWidget( QWidget *parent, const QgsPropertyDefinition &definition, const QgsProperty &initialState )
  : QgsPropertyAbstractTransformerWidget( parent, definition )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  const bool supportsAlpha = definition.standardTemplate() == QgsPropertyDefinition::ColorWithAlpha;
  mNullColorButton->setAllowOpacity( supportsAlpha );
  mNullColorButton->setShowNoColor( true );
  mNullColorButton->setColorDialogTitle( tr( "Color For Null Values" ) );
  mNullColorButton->setContext( QStringLiteral( "symbology" ) );
  mNullColorButton->setNoColorString( tr( "Transparent" ) );

  if ( const QgsColorRampTransformer *colorTransform = dynamic_cast< const QgsColorRampTransformer * >( initialState.transformer() ) )
  {
    mNullColorButton->setColor( colorTransform->nullColor() );
    if ( colorTransform->colorRamp() )
      mColorRampButton->setColorRamp( colorTransform->colorRamp() );
  }

  connect( mNullColorButton, &QgsColorButton::colorChanged, this, &QgsPropertyColorAssistantWidget::widgetChanged );
  connect( mColorRampButton, &QgsColorRampButton::colorRampChanged, this, &QgsPropertyColorAssistantWidget::widgetChanged );

  if ( !mColorRampButton->colorRamp() )
  {
    // set a default ramp
    const QString defaultRampName = QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/ColorRamp" ), QString() );
    const std::unique_ptr< QgsColorRamp > defaultRamp( QgsStyle::defaultStyle()->colorRamp( !defaultRampName.isEmpty() ? defaultRampName : QStringLiteral( "Blues" ) ) );
    if ( defaultRamp )
      mColorRampButton->setColorRamp( defaultRamp.get() );
  }
}

QgsColorRampTransformer *QgsPropertyColorAssistantWidget::createTransformer( double minValue, double maxValue ) const
{
  QgsColorRampTransformer *transformer = new QgsColorRampTransformer(
    minValue,
    maxValue,
    mColorRampButton->colorRamp(),
    mNullColorButton->color() );
  return transformer;
}

QList<QgsSymbolLegendNode *> QgsPropertyColorAssistantWidget::generatePreviews( const QList<double> &breaks, QgsLayerTreeLayer *parent, const QgsSymbol *symbol, double minValue, double maxValue, QgsCurveTransform *curve ) const
{
  QList< QgsSymbolLegendNode * > nodes;

  const QgsMarkerSymbol *legendSymbol = dynamic_cast<const QgsMarkerSymbol *>( symbol );
  std::unique_ptr< QgsMarkerSymbol > tempSymbol;

  if ( !legendSymbol )
  {
    tempSymbol.reset( QgsMarkerSymbol::createSimple( QVariantMap() ) );
    legendSymbol = tempSymbol.get();
  }
  if ( !legendSymbol )
    return nodes;

  std::unique_ptr< QgsColorRampTransformer > t( createTransformer( minValue, maxValue ) );
  if ( curve )
    t->setCurveTransform( new QgsCurveTransform( *curve ) );

  for ( int i = 0; i < breaks.length(); i++ )
  {
    std::unique_ptr< QgsSymbolLegendNode > node;
    std::unique_ptr< QgsMarkerSymbol > symbolClone( static_cast<QgsMarkerSymbol *>( legendSymbol->clone() ) );
    symbolClone->setColor( t->color( breaks[i] ) );
    node.reset( new QgsSymbolLegendNode( parent, QgsLegendSymbolItem( symbolClone.get(), QString::number( i ), QString() ) ) );
    if ( node )
      nodes << node.release();
  }
  return nodes;
}

QgsPropertyGenericNumericAssistantWidget::QgsPropertyGenericNumericAssistantWidget( QWidget *parent, const QgsPropertyDefinition &definition, const QgsProperty &initialState )
  : QgsPropertyAbstractTransformerWidget( parent, definition )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  nullOutputSpinBox->setShowClearButton( false );

  switch ( definition.standardTemplate() )
  {
    case QgsPropertyDefinition::Rotation:
    {
      // tweak dialog for rotation
      minOutputSpinBox->setMaximum( 360.0 );
      minOutputSpinBox->setValue( 0.0 );
      minOutputSpinBox->setShowClearButton( true );
      minOutputSpinBox->setClearValue( 0.0 );
      minOutputSpinBox->setSuffix( tr( " °" ) );
      maxOutputSpinBox->setMaximum( 360.0 );
      maxOutputSpinBox->setValue( 360.0 );
      maxOutputSpinBox->setShowClearButton( true );
      maxOutputSpinBox->setClearValue( 360.0 );
      maxOutputSpinBox->setSuffix( tr( " °" ) );
      exponentSpinBox->hide();
      mExponentLabel->hide();
      mLabelMinOutput->setText( tr( "Angle from" ) );
      mLabelNullOutput->setText( tr( "Angle when NULL" ) );
      break;
    }

    case QgsPropertyDefinition::Opacity:
    {
      // tweak dialog for opacity
      minOutputSpinBox->setMaximum( 100.0 );
      minOutputSpinBox->setValue( 0.0 );
      minOutputSpinBox->setShowClearButton( true );
      minOutputSpinBox->setClearValue( 0.0 );
      minOutputSpinBox->setSuffix( tr( " %" ) );
      maxOutputSpinBox->setMaximum( 100.0 );
      maxOutputSpinBox->setValue( 100.0 );
      maxOutputSpinBox->setShowClearButton( true );
      maxOutputSpinBox->setClearValue( 100.0 );
      maxOutputSpinBox->setSuffix( tr( " %" ) );
      mLabelMinOutput->setText( tr( "Opacity from" ) );
      mLabelNullOutput->setText( tr( "Opacity when NULL" ) );
      break;
    }

    case QgsPropertyDefinition::DoublePositive:
    case QgsPropertyDefinition::IntegerPositive:
      minOutputSpinBox->setMinimum( 0 );
      maxOutputSpinBox->setMinimum( 0 );
      minOutputSpinBox->setShowClearButton( false );
      maxOutputSpinBox->setShowClearButton( false );
      break;

    case QgsPropertyDefinition::IntegerPositiveGreaterZero:
      minOutputSpinBox->setMinimum( 1 );
      maxOutputSpinBox->setMinimum( 1 );
      minOutputSpinBox->setShowClearButton( false );
      maxOutputSpinBox->setShowClearButton( false );
      break;

    case QgsPropertyDefinition::Double0To1:
      minOutputSpinBox->setMinimum( 0 );
      maxOutputSpinBox->setMinimum( 0 );
      minOutputSpinBox->setMaximum( 1 );
      maxOutputSpinBox->setMaximum( 1 );
      minOutputSpinBox->setShowClearButton( false );
      maxOutputSpinBox->setShowClearButton( false );
      break;

    case QgsPropertyDefinition::Double:
      minOutputSpinBox->setMinimum( -99999999.000000 );
      maxOutputSpinBox->setMinimum( -99999999.000000 );
      minOutputSpinBox->setMaximum( 99999999.000000 );
      maxOutputSpinBox->setMaximum( 99999999.000000 );
      minOutputSpinBox->setShowClearButton( false );
      maxOutputSpinBox->setShowClearButton( false );
      break;

    default:
    {
      minOutputSpinBox->setShowClearButton( false );
      maxOutputSpinBox->setShowClearButton( false );
      break;
    }
  }

  if ( const QgsGenericNumericTransformer *transform = dynamic_cast< const QgsGenericNumericTransformer * >( initialState.transformer() ) )
  {
    minOutputSpinBox->setValue( transform->minOutputValue() );
    maxOutputSpinBox->setValue( transform->maxOutputValue() );
    nullOutputSpinBox->setValue( transform->nullOutputValue() );
    exponentSpinBox->setValue( transform->exponent() );
  }

  connect( minOutputSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertySizeAssistantWidget::widgetChanged );
  connect( maxOutputSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertySizeAssistantWidget::widgetChanged );
  connect( nullOutputSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertySizeAssistantWidget::widgetChanged );
  connect( exponentSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsPropertySizeAssistantWidget::widgetChanged );
}

QgsGenericNumericTransformer *QgsPropertyGenericNumericAssistantWidget::createTransformer( double minValue, double maxValue ) const
{
  QgsGenericNumericTransformer *transformer = new QgsGenericNumericTransformer(
    minValue,
    maxValue,
    minOutputSpinBox->value(),
    maxOutputSpinBox->value(),
    nullOutputSpinBox->value(),
    exponentSpinBox->value() );
  return transformer;
}

///@endcond PRIVATE
