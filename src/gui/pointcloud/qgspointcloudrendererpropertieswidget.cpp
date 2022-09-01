/***************************************************************************
    qgspointcloudrendererpropertieswidget.cpp
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspointcloudrendererpropertieswidget.h"

#include "qgis.h"
#include "qgspointcloudrendererregistry.h"
#include "qgsapplication.h"
#include "qgssymbolwidgetcontext.h"
#include "qgspointcloudrendererwidget.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudrenderer.h"
#include "qgspointcloudrgbrendererwidget.h"
#include "qgspointcloudattributebyramprendererwidget.h"
#include "qgspointcloudclassifiedrendererwidget.h"
#include "qgspointcloudextentrendererwidget.h"

#include "qgspointcloudrgbrenderer.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"

static bool _initRenderer( const QString &name, QgsPointCloudRendererWidgetFunc f, const QString &iconName = QString() )
{
  QgsPointCloudRendererAbstractMetadata *rendererAbstractMetadata = QgsApplication::pointCloudRendererRegistry()->rendererMetadata( name );
  if ( !rendererAbstractMetadata )
    return false;
  QgsPointCloudRendererMetadata *rendererMetadata = dynamic_cast<QgsPointCloudRendererMetadata *>( rendererAbstractMetadata );
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

static void _initRendererWidgetFunctions()
{
  static bool sInitialized = false;
  if ( sInitialized )
    return;

  _initRenderer( QStringLiteral( "extent" ), QgsPointCloudExtentRendererWidget::create, QStringLiteral( "styleicons/pointcloudextent.svg" ) );
  _initRenderer( QStringLiteral( "rgb" ), QgsPointCloudRgbRendererWidget::create, QStringLiteral( "styleicons/multibandcolor.svg" ) );
  _initRenderer( QStringLiteral( "ramp" ), QgsPointCloudAttributeByRampRendererWidget::create, QStringLiteral( "styleicons/singlebandpseudocolor.svg" ) );
  _initRenderer( QStringLiteral( "classified" ), QgsPointCloudClassifiedRendererWidget::create, QStringLiteral( "styleicons/paletted.svg" ) );

  sInitialized = true;
}

QgsPointCloudRendererPropertiesWidget::QgsPointCloudRendererPropertiesWidget( QgsPointCloudLayer *layer, QgsStyle *style, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, nullptr, parent )
  , mLayer( layer )
  , mStyle( style )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  // initialize registry's widget functions
  _initRendererWidgetFunctions();

  QgsPointCloudRendererRegistry *reg = QgsApplication::pointCloudRendererRegistry();
  const QStringList renderers = reg->renderersList();
  for ( const QString &name : renderers )
  {
    if ( QgsPointCloudRendererAbstractMetadata *m = reg->rendererMetadata( name ) )
      cboRenderers->addItem( m->icon(), m->visibleName(), name );
  }

  cboRenderers->setCurrentIndex( -1 ); // set no current renderer

  mPointStyleComboBox->addItem( tr( "Square" ), static_cast< int >( Qgis::PointCloudSymbol::Square ) );
  mPointStyleComboBox->addItem( tr( "Circle" ), static_cast< int >( Qgis::PointCloudSymbol::Circle ) );

  connect( cboRenderers, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudRendererPropertiesWidget::rendererChanged );

  connect( mBlendModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );

  mPointSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                  << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  connect( mPointSizeSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mPointSizeUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );

  mDrawOrderComboBox->addItem( tr( "Default" ), static_cast< int >( Qgis::PointCloudDrawOrder::Default ) );
  mDrawOrderComboBox->addItem( tr( "Bottom to Top" ), static_cast< int >( Qgis::PointCloudDrawOrder::BottomToTop ) );
  mDrawOrderComboBox->addItem( tr( "Top to Bottom" ), static_cast< int >( Qgis::PointCloudDrawOrder::TopToBottom ) );

  mMaxErrorUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                 << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mMaxErrorSpinBox->setClearValue( 0.3 );

  mEdlStrength->setClearValue( 1000 );
  mEdlDistance->setClearValue( 0.5 );

  mEdlDistanceUnit->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                              << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  connect( mMaxErrorSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mMaxErrorUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );

  connect( mPointStyleComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mDrawOrderComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mDrawOrderComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    mEdlWarningLabel->setVisible( mDrawOrderComboBox->currentIndex() == 0 );  // visible on default draw order
  } );
  connect( mEyeDomeLightingGroupBox, &QGroupBox::toggled, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mEdlStrength, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mEdlDistance, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mEdlDistanceUnit, &QgsUnitSelectionWidget::changed, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  syncToLayer( layer );
}

void QgsPointCloudRendererPropertiesWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mMapCanvas = context.mapCanvas();
  mMessageBar = context.messageBar();
  if ( mActiveWidget )
  {
    mActiveWidget->setContext( context );
  }
}

void QgsPointCloudRendererPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast< QgsPointCloudLayer * >( layer );

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

    mPointSizeSpinBox->setValue( mLayer->renderer()->pointSize() );
    mPointSizeUnitWidget->setUnit( mLayer->renderer()->pointSizeUnit() );
    mPointSizeUnitWidget->setMapUnitScale( mLayer->renderer()->pointSizeMapUnitScale() );

    mPointStyleComboBox->setCurrentIndex( mPointStyleComboBox->findData( static_cast< int >( mLayer->renderer()->pointSymbol() ) ) );
    mDrawOrderComboBox->setCurrentIndex( mDrawOrderComboBox->findData( static_cast< int >( mLayer->renderer()->drawOrder2d() ) ) );

    mMaxErrorSpinBox->setValue( mLayer->renderer()->maximumScreenError() );
    mMaxErrorUnitWidget->setUnit( mLayer->renderer()->maximumScreenErrorUnit() );

    mEyeDomeLightingGroupBox->setChecked( mLayer->renderer()->eyeDomeLightingEnabled() );
    mEdlStrength->setValue( mLayer->renderer()->eyeDomeLightingStrength() );
    mEdlDistance->setValue( mLayer->renderer()->eyeDomeLightingDistance() );
    mEdlDistanceUnit->setUnit( mLayer->renderer()->eyeDomeLightingDistanceUnit() );
    mEdlWarningLabel->setVisible( mLayer->renderer()->drawOrder2d() == Qgis::PointCloudDrawOrder::Default );
  }

  mBlockChangedSignal = false;
}

void QgsPointCloudRendererPropertiesWidget::setDockMode( bool dockMode )
{
  if ( mActiveWidget )
    mActiveWidget->setDockMode( dockMode );
  QgsMapLayerConfigWidget::setDockMode( dockMode );
}

void QgsPointCloudRendererPropertiesWidget::apply()
{
  mLayer->setOpacity( mOpacityWidget->opacity() );
  mLayer->setBlendMode( mBlendModeComboBox->blendMode() );

  if ( mActiveWidget )
    mLayer->setRenderer( mActiveWidget->renderer() );
  else if ( !cboRenderers->currentData().toString().isEmpty() )
  {
    QDomElement elem;
    mLayer->setRenderer( QgsApplication::pointCloudRendererRegistry()->rendererMetadata( cboRenderers->currentData().toString() )->createRenderer( elem, QgsReadWriteContext() ) );
  }

  mLayer->renderer()->setPointSize( mPointSizeSpinBox->value() );
  mLayer->renderer()->setPointSizeUnit( mPointSizeUnitWidget->unit() );
  mLayer->renderer()->setPointSizeMapUnitScale( mPointSizeUnitWidget->getMapUnitScale() );

  mLayer->renderer()->setPointSymbol( static_cast< Qgis::PointCloudSymbol >( mPointStyleComboBox->currentData().toInt() ) );

  mLayer->renderer()->setMaximumScreenError( mMaxErrorSpinBox->value() );
  mLayer->renderer()->setMaximumScreenErrorUnit( mMaxErrorUnitWidget->unit() );
  mLayer->renderer()->setDrawOrder2d( static_cast< Qgis::PointCloudDrawOrder >( mDrawOrderComboBox->currentData().toInt() ) );
  mLayer->renderer()->setEyeDomeLightingEnabled( mEyeDomeLightingGroupBox->isChecked() );
  mLayer->renderer()->setEyeDomeLightingStrength( mEdlStrength->value() );
  mLayer->renderer()->setEyeDomeLightingDistance( mEdlDistance->value() );
  mLayer->renderer()->setEyeDomeLightingDistanceUnit( mEdlDistanceUnit->unit() );
}

void QgsPointCloudRendererPropertiesWidget::rendererChanged()
{
  if ( cboRenderers->currentIndex() == -1 )
  {
    QgsDebugMsg( QStringLiteral( "No current item -- this should never happen!" ) );
    return;
  }

  const QString rendererName = cboRenderers->currentData().toString();

  //Retrieve the previous renderer: from the old active widget if possible, otherwise from the layer
  std::unique_ptr< QgsPointCloudRenderer > oldRenderer;
  std::unique_ptr< QgsPointCloudRenderer > newRenderer;
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

  QgsPointCloudRendererWidget *widget = nullptr;
  QgsPointCloudRendererAbstractMetadata *rendererMetadata = QgsApplication::pointCloudRendererRegistry()->rendererMetadata( rendererName );
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

    connect( mActiveWidget, &QgsPanelWidget::widgetChanged, this, &QgsPointCloudRendererPropertiesWidget::widgetChanged );
    connect( mActiveWidget, &QgsPanelWidget::showPanel, this, &QgsPointCloudRendererPropertiesWidget::openPanel );
    widget->setDockMode( dockMode() );
  }
  else
  {
    // set default "no edit widget available" page
    stackedWidget->setCurrentWidget( pageNoWidget );
  }
  emitWidgetChanged();
}

void QgsPointCloudRendererPropertiesWidget::emitWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit widgetChanged();
}

