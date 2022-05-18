/***************************************************************************
                              qgspointclusterrendererwidget.cpp
                              ---------------------------------
  begin                : February 2016
  copyright            : (C) 2016 by Nyall Dawson
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

#include "qgspointclusterrendererwidget.h"
#include "qgspointclusterrenderer.h"
#include "qgsrendererregistry.h"
#include "qgsfield.h"
#include "qgsstyle.h"
#include "qgssymbolselectordialog.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"
#include "qgsguiutils.h"
#include "qgsapplication.h"
#include "qgsmarkersymbol.h"

QgsRendererWidget *QgsPointClusterRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsPointClusterRendererWidget( layer, style, renderer );
}

QgsPointClusterRendererWidget::QgsPointClusterRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )

{
  if ( !layer )
  {
    return;
  }

  //the renderer only applies to point vector layers
  if ( QgsWkbTypes::geometryType( layer->wkbType() ) != QgsWkbTypes::PointGeometry )
  {
    //setup blank dialog
    mRenderer = nullptr;
    setupBlankUi( layer->name() );
    return;
  }
  setupUi( this );
  connect( mRendererComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointClusterRendererWidget::mRendererComboBox_currentIndexChanged );
  connect( mDistanceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsPointClusterRendererWidget::mDistanceSpinBox_valueChanged );
  connect( mDistanceUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsPointClusterRendererWidget::mDistanceUnitWidget_changed );
  connect( mRendererSettingsButton, &QPushButton::clicked, this, &QgsPointClusterRendererWidget::mRendererSettingsButton_clicked );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );

  mDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                 << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  mCenterSymbolToolButton->setSymbolType( Qgis::SymbolType::Marker );

  if ( renderer )
  {
    mRenderer.reset( QgsPointClusterRenderer::convertFromRenderer( renderer ) );
  }
  if ( !mRenderer )
  {
    mRenderer = std::make_unique< QgsPointClusterRenderer >();
    if ( renderer )
      renderer->copyRendererData( mRenderer.get() );
  }

  blockAllSignals( true );

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

  mDistanceSpinBox->setValue( mRenderer->tolerance() );
  mDistanceUnitWidget->setUnit( mRenderer->toleranceUnit() );
  mDistanceUnitWidget->setMapUnitScale( mRenderer->toleranceMapUnitScale() );
  mCenterSymbolToolButton->setSymbol( mRenderer->clusterSymbol()->clone() );

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

  connect( mCenterSymbolToolButton, &QgsSymbolButton::changed, this, &QgsPointClusterRendererWidget::centerSymbolChanged );
  mCenterSymbolToolButton->setDialogTitle( tr( "Cluster symbol" ) );
  mCenterSymbolToolButton->setLayer( mLayer );
  mCenterSymbolToolButton->registerExpressionContextGenerator( this );
}

QgsPointClusterRendererWidget::~QgsPointClusterRendererWidget() = default;

QgsFeatureRenderer *QgsPointClusterRendererWidget::renderer()
{
  return mRenderer.get();
}

void QgsPointClusterRendererWidget::setContext( const QgsSymbolWidgetContext &context )
{
  QgsRendererWidget::setContext( context );
  if ( mDistanceUnitWidget )
    mDistanceUnitWidget->setMapCanvas( context.mapCanvas() );
  if ( mCenterSymbolToolButton )
  {
    mCenterSymbolToolButton->setMapCanvas( context.mapCanvas() );
    mCenterSymbolToolButton->setMessageBar( context.messageBar() );
  }
}

void QgsPointClusterRendererWidget::mRendererComboBox_currentIndexChanged( int index )
{
  const QString rendererId = mRendererComboBox->itemData( index ).toString();
  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererId );
  if ( m )
  {
    // unfortunately renderer conversion is only available through the creation of a widget...
    const std::unique_ptr< QgsFeatureRenderer > oldRenderer( mRenderer->embeddedRenderer()->clone() );
    QgsRendererWidget *tempRenderWidget = m->createRendererWidget( mLayer, mStyle, oldRenderer.get() );
    mRenderer->setEmbeddedRenderer( tempRenderWidget->renderer()->clone() );
    delete tempRenderWidget;
  }
  emit widgetChanged();
}

void QgsPointClusterRendererWidget::mRendererSettingsButton_clicked()
{
  if ( !mRenderer )
    return;

  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( mRenderer->embeddedRenderer()->type() );
  if ( m )
  {
    QgsRendererWidget *w = m->createRendererWidget( mLayer, mStyle, mRenderer->embeddedRenderer()->clone() );
    w->setPanelTitle( tr( "Renderer Settings" ) );

    QgsExpressionContextScope scope;
    scope.addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_COLOR, "", true ) );
    scope.addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_SIZE, 0, true ) );
    QList< QgsExpressionContextScope > scopes = mContext.additionalExpressionContextScopes();
    scopes << scope;
    QgsSymbolWidgetContext context = mContext;
    context.setAdditionalExpressionContextScopes( scopes );
    w->setContext( context );
    w->disableSymbolLevels();
    connect( w, &QgsPanelWidget::widgetChanged, this, &QgsPointClusterRendererWidget::updateRendererFromWidget );
    w->setDockMode( this->dockMode() );
    openPanel( w );
  }
}

void QgsPointClusterRendererWidget::mDistanceSpinBox_valueChanged( double d )
{
  if ( mRenderer )
  {
    mRenderer->setTolerance( d );
  }
  emit widgetChanged();
}

void QgsPointClusterRendererWidget::mDistanceUnitWidget_changed()
{
  if ( mRenderer )
  {
    mRenderer->setToleranceUnit( mDistanceUnitWidget->unit() );
    mRenderer->setToleranceMapUnitScale( mDistanceUnitWidget->getMapUnitScale() );
  }
  emit widgetChanged();
}

void QgsPointClusterRendererWidget::blockAllSignals( bool block )
{
  mRendererComboBox->blockSignals( block );
  mCenterSymbolToolButton->blockSignals( block );
  mDistanceSpinBox->blockSignals( block );
  mDistanceUnitWidget->blockSignals( block );
}

QgsExpressionContext QgsPointClusterRendererWidget::createExpressionContext() const
{
  QgsExpressionContext context;
  if ( auto *lExpressionContext = mContext.expressionContext() )
    context = *lExpressionContext;
  else
    context.appendScopes( mContext.globalProjectAtlasMapLayerScopes( mLayer ) );
  QgsExpressionContextScope scope;
  scope.addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_COLOR, "", true ) );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_CLUSTER_SIZE, 0, true ) );
  QList< QgsExpressionContextScope > scopes = mContext.additionalExpressionContextScopes();
  scopes << scope;
  const auto constScopes = scopes;
  for ( const QgsExpressionContextScope &s : constScopes )
  {
    context << new QgsExpressionContextScope( s );
  }
  return context;
}

void QgsPointClusterRendererWidget::centerSymbolChanged()
{
  mRenderer->setClusterSymbol( mCenterSymbolToolButton->clonedSymbol< QgsMarkerSymbol >() );
  emit widgetChanged();
}

void QgsPointClusterRendererWidget::updateRendererFromWidget()
{
  QgsRendererWidget *w = qobject_cast<QgsRendererWidget *>( sender() );
  if ( !w )
    return;

  mRenderer->setEmbeddedRenderer( w->renderer()->clone() );
  emit widgetChanged();
}

void QgsPointClusterRendererWidget::setupBlankUi( const QString &layerName )
{
  QGridLayout *layout = new QGridLayout( this );
  QLabel *label = new QLabel( tr( "The point cluster renderer only applies to (single) point layers. \n'%1' is not a (single) point layer and cannot be displayed by the point cluster renderer." ).arg( layerName ), this );
  layout->addWidget( label );
}
