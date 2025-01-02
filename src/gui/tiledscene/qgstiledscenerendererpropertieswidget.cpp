/***************************************************************************
    qgstiledscenerendererpropertieswidget.cpp
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstiledscenerendererpropertieswidget.h"
#include "moc_qgstiledscenerendererpropertieswidget.cpp"

#include "qgis.h"
#include "qgstiledscenerendererregistry.h"
#include "qgsapplication.h"
#include "qgssymbolwidgetcontext.h"
#include "qgstiledscenerendererwidget.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenerenderer.h"
#include "qgstiledscenetexturerendererwidget.h"
#include "qgstiledscenewireframerendererwidget.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"

static bool initTiledSceneRenderer( const QString &name, QgsTiledSceneRendererWidgetFunc f, const QString &iconName = QString() )
{
  QgsTiledSceneRendererAbstractMetadata *rendererAbstractMetadata = QgsApplication::tiledSceneRendererRegistry()->rendererMetadata( name );
  if ( !rendererAbstractMetadata )
    return false;
  QgsTiledSceneRendererMetadata *rendererMetadata = dynamic_cast<QgsTiledSceneRendererMetadata *>( rendererAbstractMetadata );
  if ( !rendererMetadata )
    return false;

  rendererMetadata->setWidgetFunction( f );

  if ( !iconName.isEmpty() )
  {
    rendererMetadata->setIcon( QgsApplication::getThemeIcon( iconName ) );
  }

  QgsDebugMsgLevel( "Set for " + name, 2 );
  return true;
}

void QgsTiledSceneRendererPropertiesWidget::initRendererWidgetFunctions()
{
  static bool sInitialized = false;
  if ( sInitialized )
    return;

  initTiledSceneRenderer( QStringLiteral( "texture" ), QgsTiledSceneTextureRendererWidget::create, QStringLiteral( "styleicons/tiledscenetexture.svg" ) );
  initTiledSceneRenderer( QStringLiteral( "wireframe" ), QgsTiledSceneWireframeRendererWidget::create, QStringLiteral( "styleicons/tiledscenewireframe.svg" ) );

  sInitialized = true;
}

QgsTiledSceneRendererPropertiesWidget::QgsTiledSceneRendererPropertiesWidget( QgsTiledSceneLayer *layer, QgsStyle *style, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, nullptr, parent )
  , mLayer( layer )
  , mStyle( style )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  // initialize registry's widget functions
  initRendererWidgetFunctions();

  QgsTiledSceneRendererRegistry *reg = QgsApplication::tiledSceneRendererRegistry();
  const QStringList renderers = reg->renderersList();
  for ( const QString &name : renderers )
  {
    if ( QgsTiledSceneRendererAbstractMetadata *m = reg->rendererMetadata( name ) )
      cboRenderers->addItem( m->icon(), m->visibleName(), name );
  }

  cboRenderers->setCurrentIndex( -1 ); // set no current renderer

  connect( cboRenderers, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsTiledSceneRendererPropertiesWidget::rendererChanged );

  connect( mBlendModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsTiledSceneRendererPropertiesWidget::emitWidgetChanged );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsTiledSceneRendererPropertiesWidget::emitWidgetChanged );

  mMaxErrorUnitWidget->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::MetersInMapUnits, Qgis::RenderUnit::MapUnits, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::Points, Qgis::RenderUnit::Inches } );
  mMaxErrorSpinBox->setClearValue( 3 );

  connect( mMaxErrorSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsTiledSceneRendererPropertiesWidget::emitWidgetChanged );
  connect( mMaxErrorUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsTiledSceneRendererPropertiesWidget::emitWidgetChanged );

  syncToLayer( layer );
}

void QgsTiledSceneRendererPropertiesWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mMapCanvas = context.mapCanvas();
  mMessageBar = context.messageBar();
  if ( mActiveWidget )
  {
    mActiveWidget->setContext( context );
  }
}

void QgsTiledSceneRendererPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast<QgsTiledSceneLayer *>( layer );

  mBlockChangedSignal = true;
  mOpacityWidget->setOpacity( mLayer->opacity() );
  mBlendModeComboBox->setShowClippingModes( QgsProjectUtils::layerIsContainedInGroupLayer( QgsProject::instance(), mLayer ) );
  mBlendModeComboBox->setBlendMode( mLayer->blendMode() );

  if ( mLayer->renderer() )
  {
    // set current renderer from layer
    const QString rendererName = mLayer->renderer()->type();

    const int rendererIdx = cboRenderers->findData( rendererName );
    cboRenderers->setCurrentIndex( rendererIdx );

    // no renderer found... this mustn't happen
    Q_ASSERT( rendererIdx != -1 && "there must be a renderer!" );

    mMaxErrorSpinBox->setValue( mLayer->renderer()->maximumScreenError() );
    mMaxErrorUnitWidget->setUnit( mLayer->renderer()->maximumScreenErrorUnit() );
  }

  mBlockChangedSignal = false;
}

void QgsTiledSceneRendererPropertiesWidget::setDockMode( bool dockMode )
{
  if ( mActiveWidget )
    mActiveWidget->setDockMode( dockMode );
  QgsMapLayerConfigWidget::setDockMode( dockMode );
}

void QgsTiledSceneRendererPropertiesWidget::apply()
{
  mLayer->setOpacity( mOpacityWidget->opacity() );
  mLayer->setBlendMode( mBlendModeComboBox->blendMode() );

  if ( mActiveWidget )
    mLayer->setRenderer( mActiveWidget->renderer() );
  else if ( !cboRenderers->currentData().toString().isEmpty() )
  {
    QDomElement elem;
    mLayer->setRenderer( QgsApplication::tiledSceneRendererRegistry()->rendererMetadata( cboRenderers->currentData().toString() )->createRenderer( elem, QgsReadWriteContext() ) );
  }

  mLayer->renderer()->setMaximumScreenError( mMaxErrorSpinBox->value() );
  mLayer->renderer()->setMaximumScreenErrorUnit( mMaxErrorUnitWidget->unit() );
}

void QgsTiledSceneRendererPropertiesWidget::rendererChanged()
{
  if ( cboRenderers->currentIndex() == -1 )
  {
    QgsDebugError( QStringLiteral( "No current item -- this should never happen!" ) );
    return;
  }

  const QString rendererName = cboRenderers->currentData().toString();

  //Retrieve the previous renderer: from the old active widget if possible, otherwise from the layer
  std::unique_ptr<QgsTiledSceneRenderer> oldRenderer;
  std::unique_ptr<QgsTiledSceneRenderer> newRenderer;
  if ( mActiveWidget )
    newRenderer.reset( mActiveWidget->renderer() );

  if ( newRenderer )
  {
    oldRenderer = std::move( newRenderer );
  }
  else
  {
    oldRenderer.reset( mLayer->renderer()->clone() );
  }

  // get rid of old active widget (if any)
  if ( mActiveWidget )
  {
    stackedWidget->removeWidget( mActiveWidget );

    delete mActiveWidget;
    mActiveWidget = nullptr;
  }

  QgsTiledSceneRendererWidget *widget = nullptr;
  QgsTiledSceneRendererAbstractMetadata *rendererMetadata = QgsApplication::tiledSceneRendererRegistry()->rendererMetadata( rendererName );
  if ( rendererMetadata )
    widget = rendererMetadata->createRendererWidget( mLayer, mStyle, oldRenderer.get() );
  oldRenderer.reset();

  if ( widget )
  {
    // instantiate the widget and set as active
    mActiveWidget = widget;
    stackedWidget->addWidget( mActiveWidget );
    stackedWidget->setCurrentWidget( mActiveWidget );

    if ( mMapCanvas || mMessageBar )
    {
      QgsSymbolWidgetContext context;
      context.setMapCanvas( mMapCanvas );
      context.setMessageBar( mMessageBar );
      mActiveWidget->setContext( context );
    }

    connect( mActiveWidget, &QgsPanelWidget::widgetChanged, this, &QgsTiledSceneRendererPropertiesWidget::widgetChanged );
    connect( mActiveWidget, &QgsPanelWidget::showPanel, this, &QgsTiledSceneRendererPropertiesWidget::openPanel );
    widget->setDockMode( dockMode() );
  }
  else
  {
    // set default "no edit widget available" page
    stackedWidget->setCurrentWidget( pageNoWidget );
  }
  emitWidgetChanged();
}

void QgsTiledSceneRendererPropertiesWidget::emitWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit widgetChanged();
}
