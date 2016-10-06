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
#include "qgisgui.h"

QgsRendererWidget* QgsPointClusterRendererWidget::create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer )
{
  return new QgsPointClusterRendererWidget( layer, style, renderer );
}

QgsPointClusterRendererWidget::QgsPointClusterRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer )
    : QgsRendererWidget( layer, style )
    , mRenderer( nullptr )
{
  if ( !layer )
  {
    return;
  }

  //the renderer only applies to point vector layers
  if ( QgsWkbTypes::flatType( layer->wkbType() ) != QgsWkbTypes::Point )
  {
    //setup blank dialog
    mRenderer = nullptr;
    setupBlankUi( layer->name() );
    return;
  }
  setupUi( this );
  mDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );

  if ( renderer )
  {
    mRenderer = QgsPointClusterRenderer::convertFromRenderer( renderer );
  }
  if ( !mRenderer )
  {
    mRenderer = new QgsPointClusterRenderer();
  }

  blockAllSignals( true );

  //insert possible renderer types
  QStringList rendererList = QgsRendererRegistry::instance()->renderersList( QgsRendererAbstractMetadata::PointLayer );
  QStringList::const_iterator it = rendererList.constBegin();
  for ( ; it != rendererList.constEnd(); ++it )
  {
    if ( *it != "pointDisplacement" && *it != "pointCluster" && *it != "heatmapRenderer" )
    {
      QgsRendererAbstractMetadata* m = QgsRendererRegistry::instance()->rendererMetadata( *it );
      mRendererComboBox->addItem( m->icon(), m->visibleName(), *it );
    }
  }

  mDistanceSpinBox->setValue( mRenderer->tolerance() );
  mDistanceUnitWidget->setUnit( mRenderer->toleranceUnit() );
  mDistanceUnitWidget->setMapUnitScale( mRenderer->toleranceMapUnitScale() );

  blockAllSignals( false );

  //set the appropriate renderer dialog
  if ( mRenderer->embeddedRenderer() )
  {
    QString rendererName = mRenderer->embeddedRenderer()->type();
    int rendererIndex = mRendererComboBox->findData( rendererName );
    if ( rendererIndex != -1 )
    {
      mRendererComboBox->setCurrentIndex( rendererIndex );
      on_mRendererComboBox_currentIndexChanged( rendererIndex );
    }
  }

  updateCenterIcon();
}

QgsPointClusterRendererWidget::~QgsPointClusterRendererWidget()
{
  delete mRenderer;
}

QgsFeatureRenderer* QgsPointClusterRendererWidget::renderer()
{
  return mRenderer;
}

void QgsPointClusterRendererWidget::setContext( const QgsSymbolWidgetContext& context )
{
  QgsRendererWidget::setContext( context );
  if ( mDistanceUnitWidget )
    mDistanceUnitWidget->setMapCanvas( context.mapCanvas() );
}

void QgsPointClusterRendererWidget::on_mRendererComboBox_currentIndexChanged( int index )
{
  QString rendererId = mRendererComboBox->itemData( index ).toString();
  QgsRendererAbstractMetadata* m = QgsRendererRegistry::instance()->rendererMetadata( rendererId );
  if ( m )
  {
    // unfortunately renderer conversion is only available through the creation of a widget...
    QgsRendererWidget* tempRenderWidget = m->createRendererWidget( mLayer, mStyle, mRenderer->embeddedRenderer()->clone() );
    mRenderer->setEmbeddedRenderer( tempRenderWidget->renderer()->clone() );
    delete tempRenderWidget;
  }
  emit widgetChanged();
}

void QgsPointClusterRendererWidget::on_mRendererSettingsButton_clicked()
{
  if ( !mRenderer )
    return;

  QgsRendererAbstractMetadata* m = QgsRendererRegistry::instance()->rendererMetadata( mRenderer->embeddedRenderer()->type() );
  if ( m )
  {
    QgsRendererWidget* w = m->createRendererWidget( mLayer, mStyle, mRenderer->embeddedRenderer()->clone() );
    w->setPanelTitle( tr( "Renderer settings" ) );

    QgsExpressionContextScope scope;
    scope.setVariable( QgsExpressionContext::EXPR_CLUSTER_COLOR, "" );
    scope.setVariable( QgsExpressionContext::EXPR_CLUSTER_SIZE, 0 );
    QList< QgsExpressionContextScope > scopes = mContext.additionalExpressionContextScopes();
    scopes << scope;
    QgsSymbolWidgetContext context = mContext;
    context.setAdditionalExpressionContextScopes( scopes );
    w->setContext( context );
    connect( w, SIGNAL( widgetChanged() ), this, SLOT( updateRendererFromWidget() ) );
    w->setDockMode( this->dockMode() );
    openPanel( w );
  }
}

void QgsPointClusterRendererWidget::on_mDistanceSpinBox_valueChanged( double d )
{
  if ( mRenderer )
  {
    mRenderer->setTolerance( d );
  }
  emit widgetChanged();
}

void QgsPointClusterRendererWidget::on_mDistanceUnitWidget_changed()
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
  mCenterSymbolPushButton->blockSignals( block );
  mDistanceSpinBox->blockSignals( block );
  mDistanceUnitWidget->blockSignals( block );
}

void QgsPointClusterRendererWidget::on_mCenterSymbolPushButton_clicked()
{
  if ( !mRenderer || !mRenderer->clusterSymbol() )
  {
    return;
  }
  QgsMarkerSymbol* markerSymbol = mRenderer->clusterSymbol()->clone();
  QgsSymbolSelectorWidget* dlg = new QgsSymbolSelectorWidget( markerSymbol, QgsStyle::defaultStyle(), mLayer, this );
  dlg->setPanelTitle( tr( "Cluster symbol" ) );
  dlg->setDockMode( this->dockMode() );

  QgsSymbolWidgetContext context = mContext;
  QgsExpressionContextScope scope;
  scope.setVariable( QgsExpressionContext::EXPR_CLUSTER_COLOR, "" );
  scope.setVariable( QgsExpressionContext::EXPR_CLUSTER_SIZE, 0 );
  QList< QgsExpressionContextScope > scopes = context.additionalExpressionContextScopes();
  scopes << scope;
  context.setAdditionalExpressionContextScopes( scopes );

  dlg->setContext( context );

  connect( dlg, SIGNAL( widgetChanged() ), this, SLOT( updateCenterSymbolFromWidget() ) );
  connect( dlg, SIGNAL( panelAccepted( QgsPanelWidget* ) ), this, SLOT( cleanUpSymbolSelector( QgsPanelWidget* ) ) );
  openPanel( dlg );
}

void QgsPointClusterRendererWidget::updateCenterSymbolFromWidget()
{
  QgsSymbolSelectorWidget* dlg = qobject_cast<QgsSymbolSelectorWidget*>( sender() );
  QgsSymbol* symbol = dlg->symbol()->clone();
  mRenderer->setClusterSymbol( static_cast< QgsMarkerSymbol* >( symbol ) );
  updateCenterIcon();
  emit widgetChanged();
}

void QgsPointClusterRendererWidget::cleanUpSymbolSelector( QgsPanelWidget *container )
{
  if ( container )
  {
    QgsSymbolSelectorWidget* dlg = qobject_cast<QgsSymbolSelectorWidget*>( container );
    delete dlg->symbol();
  }
}

void QgsPointClusterRendererWidget::updateRendererFromWidget()
{
  QgsRendererWidget* w = qobject_cast<QgsRendererWidget*>( sender() );
  if ( !w )
    return;

  mRenderer->setEmbeddedRenderer( w->renderer()->clone() );
  emit widgetChanged();
}

void QgsPointClusterRendererWidget::updateCenterIcon()
{
  QgsMarkerSymbol* symbol = mRenderer->clusterSymbol();
  if ( !symbol )
  {
    return;
  }
  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( symbol, mCenterSymbolPushButton->iconSize() );
  mCenterSymbolPushButton->setIcon( icon );
}

void QgsPointClusterRendererWidget::setupBlankUi( const QString& layerName )
{
  QGridLayout* layout = new QGridLayout( this );
  QLabel* label = new QLabel( tr( "The point cluster renderer only applies to (single) point layers. \n'%1' is not a point layer and cannot be displayed by the point cluster renderer" ).arg( layerName ), this );
  layout->addWidget( label );
}
