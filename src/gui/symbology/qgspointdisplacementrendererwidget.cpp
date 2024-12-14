/***************************************************************************
                              qgspointdisplacementrendererwidget.cpp
                              --------------------------------------
  begin                : January 26, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointdisplacementrendererwidget.h"
#include "moc_qgspointdisplacementrendererwidget.cpp"
#include "qgspointdisplacementrenderer.h"
#include "qgsrendererregistry.h"
#include "qgsfields.h"
#include "qgsstyle.h"
#include "qgssymbolselectordialog.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgsmarkersymbol.h"

QgsRendererWidget *QgsPointDisplacementRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsPointDisplacementRendererWidget( layer, style, renderer );
}

QgsPointDisplacementRendererWidget::QgsPointDisplacementRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )

{
  if ( !layer )
  {
    return;
  }

  //the renderer only applies to point vector layers
  if ( QgsWkbTypes::geometryType( layer->wkbType() ) != Qgis::GeometryType::Point )
  {
    //setup blank dialog
    mRenderer = nullptr;
    setupBlankUi( layer->name() );
    return;
  }
  setupUi( this );
  connect( mLabelFieldComboBox, &QComboBox::currentTextChanged, this, &QgsPointDisplacementRendererWidget::mLabelFieldComboBox_currentIndexChanged );
  connect( mRendererComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointDisplacementRendererWidget::mRendererComboBox_currentIndexChanged );
  connect( mPlacementComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointDisplacementRendererWidget::mPlacementComboBox_currentIndexChanged );
  connect( mCircleWidthSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPointDisplacementRendererWidget::mCircleWidthSpinBox_valueChanged );
  connect( mCircleColorButton, &QgsColorButton::colorChanged, this, &QgsPointDisplacementRendererWidget::mCircleColorButton_colorChanged );
  connect( mDistanceSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPointDisplacementRendererWidget::mDistanceSpinBox_valueChanged );
  connect( mDistanceUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsPointDisplacementRendererWidget::mDistanceUnitWidget_changed );
  connect( mLabelColorButton, &QgsColorButton::colorChanged, this, &QgsPointDisplacementRendererWidget::mLabelColorButton_colorChanged );
  connect( mCircleModificationSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPointDisplacementRendererWidget::mCircleModificationSpinBox_valueChanged );
  connect( mLabelDistanceFactorSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPointDisplacementRendererWidget::mLabelDistanceFactorSpinBox_valueChanged );
  connect( mScaleDependentLabelsCheckBox, &QCheckBox::stateChanged, this, &QgsPointDisplacementRendererWidget::mScaleDependentLabelsCheckBox_stateChanged );
  connect( mRendererSettingsButton, &QPushButton::clicked, this, &QgsPointDisplacementRendererWidget::mRendererSettingsButton_clicked );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );

  mLabelFontButton->setMode( QgsFontButton::ModeQFont );
  mDistanceUnitWidget->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::MetersInMapUnits, Qgis::RenderUnit::MapUnits, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::Points, Qgis::RenderUnit::Inches } );
  mCenterSymbolToolButton->setSymbolType( Qgis::SymbolType::Marker );

  if ( renderer )
  {
    mRenderer.reset( QgsPointDisplacementRenderer::convertFromRenderer( renderer ) );
  }
  if ( !mRenderer )
  {
    mRenderer = std::make_unique<QgsPointDisplacementRenderer>();
    if ( renderer )
      renderer->copyRendererData( mRenderer.get() );
  }

  blockAllSignals( true );

  mPlacementComboBox->addItem( tr( "Ring" ), QgsPointDisplacementRenderer::Ring );
  mPlacementComboBox->addItem( tr( "Concentric Rings" ), QgsPointDisplacementRenderer::ConcentricRings );
  mPlacementComboBox->addItem( tr( "Grid" ), QgsPointDisplacementRenderer::Grid );

  //insert attributes into combo box
  if ( layer )
  {
    const QgsFields layerFields = layer->fields();
    for ( const QgsField &f : layerFields )
    {
      mLabelFieldComboBox->addItem( f.name() );
    }
    mLabelFieldComboBox->addItem( tr( "None" ) );

    const QString currentLabelAttribute = mRenderer->labelAttributeName();
    if ( !currentLabelAttribute.isEmpty() )
    {
      mLabelFieldComboBox->setCurrentIndex( mLabelFieldComboBox->findText( currentLabelAttribute ) );
    }
    else
    {
      mLabelFieldComboBox->setCurrentIndex( mLabelFieldComboBox->findText( tr( "None" ) ) );
    }
  }

  //insert possible renderer types
  const QStringList rendererList = QgsApplication::rendererRegistry()->renderersList( QgsRendererAbstractMetadata::PointLayer );
  QStringList::const_iterator it = rendererList.constBegin();
  for ( ; it != rendererList.constEnd(); ++it )
  {
    if ( *it != QLatin1String( "pointDisplacement" ) && *it != QLatin1String( "pointCluster" ) && *it != QLatin1String( "heatmapRenderer" ) )
    {
      QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( *it );
      mRendererComboBox->addItem( m->icon(), m->visibleName(), *it );
    }
  }

  mCircleColorButton->setColorDialogTitle( tr( "Select Color" ) );
  mCircleColorButton->setContext( QStringLiteral( "symbology" ) );
  mCircleColorButton->setAllowOpacity( true );
  mCircleColorButton->setShowNoColor( true );
  mCircleColorButton->setNoColorString( tr( "Transparent Stroke" ) );
  mLabelColorButton->setContext( QStringLiteral( "symbology" ) );
  mLabelColorButton->setColorDialogTitle( tr( "Select Color" ) );
  mLabelColorButton->setAllowOpacity( true );

  mCircleWidthSpinBox->setValue( mRenderer->circleWidth() );
  mCircleColorButton->setColor( mRenderer->circleColor() );
  mLabelColorButton->setColor( mRenderer->labelColor() );
  mLabelFontButton->setCurrentFont( mRenderer->labelFont() );
  mCircleModificationSpinBox->setClearValue( 0.0 );
  mCircleModificationSpinBox->setValue( mRenderer->circleRadiusAddition() );
  mLabelDistanceFactorSpinBox->setClearValue( 0.5 );
  mLabelDistanceFactorSpinBox->setValue( mRenderer->labelDistanceFactor() );
  mDistanceSpinBox->setValue( mRenderer->tolerance() );
  mDistanceUnitWidget->setUnit( mRenderer->toleranceUnit() );
  mDistanceUnitWidget->setMapUnitScale( mRenderer->toleranceMapUnitScale() );
  mCenterSymbolToolButton->setSymbol( mRenderer->centerSymbol()->clone() );

  mPlacementComboBox->setCurrentIndex( mPlacementComboBox->findData( mRenderer->placement() ) );

  //scale dependent labeling
  mMinLabelScaleWidget->setScale( std::max( mRenderer->minimumLabelScale(), 0.0 ) );
  if ( mRenderer->minimumLabelScale() > 0 )
  {
    mScaleDependentLabelsCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mScaleDependentLabelsCheckBox->setCheckState( Qt::Unchecked );
    mMinLabelScaleWidget->setEnabled( false );
  }


  blockAllSignals( false );

  //set the appropriate renderer dialog
  if ( mRenderer->embeddedRenderer() )
  {
    const QString rendererName = mRenderer->embeddedRenderer()->type();
    const int rendererIndex = mRendererComboBox->findData( rendererName );
    if ( rendererIndex != -1 )
    {
      mRendererComboBox->setCurrentIndex( rendererIndex );
      mRendererComboBox_currentIndexChanged( rendererIndex );
    }
  }

  connect( mMinLabelScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsPointDisplacementRendererWidget::minLabelScaleChanged );
  connect( mLabelFontButton, &QgsFontButton::changed, this, &QgsPointDisplacementRendererWidget::labelFontChanged );
  connect( mCenterSymbolToolButton, &QgsSymbolButton::changed, this, &QgsPointDisplacementRendererWidget::centerSymbolChanged );
  mCenterSymbolToolButton->setDialogTitle( tr( "Center symbol" ) );
  mCenterSymbolToolButton->setLayer( mLayer );
  mCenterSymbolToolButton->registerExpressionContextGenerator( this );
}

QgsPointDisplacementRendererWidget::~QgsPointDisplacementRendererWidget() = default;

QgsFeatureRenderer *QgsPointDisplacementRendererWidget::renderer()
{
  return mRenderer.get();
}

void QgsPointDisplacementRendererWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsRendererWidget::setContext( context );
  if ( mDistanceUnitWidget )
    mDistanceUnitWidget->setMapCanvas( context.mapCanvas() );
  if ( mMinLabelScaleWidget )
  {
    mMinLabelScaleWidget->setMapCanvas( context.mapCanvas() );
    mMinLabelScaleWidget->setShowCurrentScaleButton( true );
  }
  if ( mCenterSymbolToolButton )
  {
    mCenterSymbolToolButton->setMapCanvas( context.mapCanvas() );
    mCenterSymbolToolButton->setMessageBar( context.messageBar() );
  }
}

QgsExpressionContext QgsPointDisplacementRendererWidget::createExpressionContext() const
{
  QgsExpressionContext context;
  if ( auto *lExpressionContext = mContext.expressionContext() )
    context = *lExpressionContext;
  else
    context.appendScopes( mContext.globalProjectAtlasMapLayerScopes( mLayer ) );
  QgsExpressionContextScope scope;
  scope.addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_COLOR, "", true ) );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_SIZE, 0, true ) );
  QList<QgsExpressionContextScope> scopes = mContext.additionalExpressionContextScopes();
  scopes << scope;
  for ( const QgsExpressionContextScope &s : std::as_const( scopes ) )
  {
    context << new QgsExpressionContextScope( s );
  }
  return context;
}

void QgsPointDisplacementRendererWidget::mLabelFieldComboBox_currentIndexChanged( const QString &text )
{
  if ( mRenderer )
  {
    if ( text == tr( "None" ) )
    {
      mRenderer->setLabelAttributeName( QString() );
    }
    else
    {
      mRenderer->setLabelAttributeName( text );
    }
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::mRendererComboBox_currentIndexChanged( int index )
{
  const QString rendererId = mRendererComboBox->itemData( index ).toString();
  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererId );
  if ( m )
  {
    // unfortunately renderer conversion is only available through the creation of a widget...
    const std::unique_ptr<QgsFeatureRenderer> oldRenderer( mRenderer->embeddedRenderer()->clone() );
    QgsRendererWidget *tempRenderWidget = m->createRendererWidget( mLayer, mStyle, oldRenderer.get() );
    mRenderer->setEmbeddedRenderer( tempRenderWidget->renderer()->clone() );
    delete tempRenderWidget;
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::mPlacementComboBox_currentIndexChanged( int index )
{
  if ( !mRenderer )
    return;

  mRenderer->setPlacement( ( QgsPointDisplacementRenderer::Placement ) mPlacementComboBox->itemData( index ).toInt() );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::mRendererSettingsButton_clicked()
{
  if ( !mRenderer )
    return;

  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( mRenderer->embeddedRenderer()->type() );
  if ( m )
  {
    QgsRendererWidget *w = m->createRendererWidget( mLayer, mStyle, mRenderer->embeddedRenderer()->clone() );
    w->setPanelTitle( tr( "Renderer Settings" ) );

    QgsSymbolWidgetContext context = mContext;

    QgsExpressionContextScope scope;
    scope.addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_COLOR, "", true ) );
    scope.addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_SIZE, 0, true ) );
    QList<QgsExpressionContextScope> scopes = context.additionalExpressionContextScopes();
    scopes << scope;
    context.setAdditionalExpressionContextScopes( scopes );
    w->disableSymbolLevels();
    w->setContext( context );

    connect( w, &QgsPanelWidget::widgetChanged, this, &QgsPointDisplacementRendererWidget::updateRendererFromWidget );
    openPanel( w );
  }
}

void QgsPointDisplacementRendererWidget::labelFontChanged()
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setLabelFont( mLabelFontButton->currentFont() );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::mCircleWidthSpinBox_valueChanged( double d )
{
  if ( mRenderer )
  {
    mRenderer->setCircleWidth( d );
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::mCircleColorButton_colorChanged( const QColor &newColor )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setCircleColor( newColor );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::mLabelColorButton_colorChanged( const QColor &newColor )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setLabelColor( newColor );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::mCircleModificationSpinBox_valueChanged( double d )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setCircleRadiusAddition( d );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::mLabelDistanceFactorSpinBox_valueChanged( double d )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setLabelDistanceFactor( d );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::mDistanceSpinBox_valueChanged( double d )
{
  if ( mRenderer )
  {
    mRenderer->setTolerance( d );
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::mDistanceUnitWidget_changed()
{
  if ( mRenderer )
  {
    mRenderer->setToleranceUnit( mDistanceUnitWidget->unit() );
    mRenderer->setToleranceMapUnitScale( mDistanceUnitWidget->getMapUnitScale() );
    emit widgetChanged();
  }
}

void QgsPointDisplacementRendererWidget::mScaleDependentLabelsCheckBox_stateChanged( int state )
{
  if ( state == Qt::Unchecked )
  {
    mMinLabelScaleWidget->setScale( 0 );
    mMinLabelScaleWidget->setEnabled( false );
  }
  else
  {
    mMinLabelScaleWidget->setEnabled( true );
  }
}

void QgsPointDisplacementRendererWidget::minLabelScaleChanged( double scale )
{
  if ( !mRenderer )
  {
    return;
  }

  mRenderer->setMinimumLabelScale( scale );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::blockAllSignals( bool block )
{
  mLabelFieldComboBox->blockSignals( block );
  mLabelFontButton->blockSignals( block );
  mCircleWidthSpinBox->blockSignals( block );
  mCircleColorButton->blockSignals( block );
  mRendererComboBox->blockSignals( block );
  mLabelColorButton->blockSignals( block );
  mCircleModificationSpinBox->blockSignals( block );
  mLabelDistanceFactorSpinBox->blockSignals( block );
  mScaleDependentLabelsCheckBox->blockSignals( block );
  mMinLabelScaleWidget->blockSignals( block );
  mCenterSymbolToolButton->blockSignals( block );
  mDistanceSpinBox->blockSignals( block );
  mDistanceUnitWidget->blockSignals( block );
  mPlacementComboBox->blockSignals( block );
}

void QgsPointDisplacementRendererWidget::centerSymbolChanged()
{
  mRenderer->setCenterSymbol( mCenterSymbolToolButton->clonedSymbol<QgsMarkerSymbol>() );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::updateRendererFromWidget()
{
  QgsRendererWidget *w = qobject_cast<QgsRendererWidget *>( sender() );
  if ( !w )
    return;

  mRenderer->setEmbeddedRenderer( w->renderer()->clone() );
  emit widgetChanged();
}

void QgsPointDisplacementRendererWidget::setupBlankUi( const QString &layerName )
{
  QLabel *label = new QLabel( tr( "The point displacement renderer only applies to (single) point layers. \n'%1' is not a (single) point layer and cannot be displayed by the point displacement renderer." ).arg( layerName ), this );
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( label );
}
