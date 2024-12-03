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
#include "moc_qgspointcloudrendererpropertieswidget.cpp"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsfontbutton.h"
#include "qgslogger.h"
#include "qgspointcloudattributebyramprendererwidget.h"
#include "qgspointcloudclassifiedrendererwidget.h"
#include "qgspointcloudextentrenderer.h"
#include "qgspointcloudextentrendererwidget.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudrenderer.h"
#include "qgspointcloudrendererregistry.h"
#include "qgspointcloudrendererwidget.h"
#include "qgspointcloudrgbrendererwidget.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"
#include "qgsstyle.h"
#include "qgssymbolwidgetcontext.h"
#include "qgstextformatwidget.h"

static bool initPointCloudRenderer( const QString &name, QgsPointCloudRendererWidgetFunc f, const QString &iconName = QString() )
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

void QgsPointCloudRendererPropertiesWidget::initRendererWidgetFunctions()
{
  static bool sInitialized = false;
  if ( sInitialized )
    return;

  initPointCloudRenderer( QStringLiteral( "extent" ), QgsPointCloudExtentRendererWidget::create, QStringLiteral( "styleicons/pointcloudextent.svg" ) );
  initPointCloudRenderer( QStringLiteral( "rgb" ), QgsPointCloudRgbRendererWidget::create, QStringLiteral( "styleicons/multibandcolor.svg" ) );
  initPointCloudRenderer( QStringLiteral( "ramp" ), QgsPointCloudAttributeByRampRendererWidget::create, QStringLiteral( "styleicons/singlebandpseudocolor.svg" ) );
  initPointCloudRenderer( QStringLiteral( "classified" ), QgsPointCloudClassifiedRendererWidget::create, QStringLiteral( "styleicons/paletted.svg" ) );

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
  initRendererWidgetFunctions();

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

  mPointSizeUnitWidget->setUnits( {Qgis::RenderUnit::Millimeters,
                                   Qgis::RenderUnit::MetersInMapUnits,
                                   Qgis::RenderUnit::MapUnits,
                                   Qgis::RenderUnit::Pixels,
                                   Qgis::RenderUnit::Points,
                                   Qgis::RenderUnit::Inches } );

  connect( mPointSizeSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mPointSizeUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );

  mDrawOrderComboBox->addItem( tr( "Default" ), static_cast< int >( Qgis::PointCloudDrawOrder::Default ) );
  mDrawOrderComboBox->addItem( tr( "Bottom to Top" ), static_cast< int >( Qgis::PointCloudDrawOrder::BottomToTop ) );
  mDrawOrderComboBox->addItem( tr( "Top to Bottom" ), static_cast< int >( Qgis::PointCloudDrawOrder::TopToBottom ) );

  mMaxErrorUnitWidget->setUnits( { Qgis::RenderUnit::Millimeters,
                                   Qgis::RenderUnit::MetersInMapUnits,
                                   Qgis::RenderUnit::MapUnits,
                                   Qgis::RenderUnit::Pixels,
                                   Qgis::RenderUnit::Points,
                                   Qgis::RenderUnit::Inches } );
  mMaxErrorSpinBox->setClearValue( 0.3 );

  mHorizontalTriangleThresholdSpinBox->setClearValue( 5.0 );
  mHorizontalTriangleUnitWidget->setUnits( { Qgis::RenderUnit::Millimeters,
      Qgis::RenderUnit::MetersInMapUnits,
      Qgis::RenderUnit::MapUnits,
      Qgis::RenderUnit::Pixels,
      Qgis::RenderUnit::Points,
      Qgis::RenderUnit::Inches } );

  connect( mMaxErrorSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mMaxErrorUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );

  connect( mPointStyleComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mDrawOrderComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );

  connect( mTriangulateGroupBox, &QGroupBox::toggled, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mHorizontalTriangleCheckBox, &QCheckBox::clicked, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mHorizontalTriangleThresholdSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mHorizontalTriangleUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );

  // show label options only for virtual point clouds
  bool showLabelOptions = !mLayer->dataProvider()->subIndexes().isEmpty();
  mLabels->setVisible( showLabelOptions );
  mLabelOptions->setVisible( showLabelOptions );
  mLabelOptions->setDialogTitle( tr( "Customize label text" ) );
  connect( mLabels, &QCheckBox::stateChanged, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );
  connect( mLabelOptions, &QgsFontButton::changed, this, &QgsPointCloudRendererPropertiesWidget::emitWidgetChanged );

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
    if ( cboRenderers->currentIndex() != rendererIdx )
    {
      cboRenderers->setCurrentIndex( rendererIdx );
    }
    else
    {
      rendererChanged();
    }

    // no renderer found... this mustn't happen
    Q_ASSERT( rendererIdx != -1 && "there must be a renderer!" );

    mPointSizeSpinBox->setValue( mLayer->renderer()->pointSize() );
    mPointSizeUnitWidget->setUnit( mLayer->renderer()->pointSizeUnit() );
    mPointSizeUnitWidget->setMapUnitScale( mLayer->renderer()->pointSizeMapUnitScale() );

    mPointStyleComboBox->setCurrentIndex( mPointStyleComboBox->findData( static_cast< int >( mLayer->renderer()->pointSymbol() ) ) );
    mDrawOrderComboBox->setCurrentIndex( mDrawOrderComboBox->findData( static_cast< int >( mLayer->renderer()->drawOrder2d() ) ) );

    mMaxErrorSpinBox->setValue( mLayer->renderer()->maximumScreenError() );
    mMaxErrorUnitWidget->setUnit( mLayer->renderer()->maximumScreenErrorUnit() );

    mTriangulateGroupBox->setChecked( mLayer->renderer()->renderAsTriangles() );
    mHorizontalTriangleCheckBox->setChecked( mLayer->renderer()->horizontalTriangleFilter() );
    mHorizontalTriangleThresholdSpinBox->setValue( mLayer->renderer()->horizontalTriangleFilterThreshold() );
    mHorizontalTriangleUnitWidget->setUnit( mLayer->renderer()->horizontalTriangleFilterUnit() );

    if ( !mLayer->dataProvider()->subIndexes().isEmpty() )
    {
      mLabels->setChecked( mLayer->renderer()->showLabels() );
      // set some better defaults for label format
      QgsTextBufferSettings settings;
      settings.setEnabled( true );
      settings.setSize( 0.5 );
      mLayer->renderer()->labelTextFormat()->setBuffer( settings );
      mLayer->renderer()->labelTextFormat()->setNamedStyle( QStringLiteral( "Regular" ) );
      mLabelOptions->setTextFormat( *mLayer->renderer()->labelTextFormat() );
    }
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

  mLayer->renderer()->setRenderAsTriangles( mTriangulateGroupBox->isChecked() );
  mLayer->renderer()->setHorizontalTriangleFilter( mHorizontalTriangleCheckBox->isChecked() );
  mLayer->renderer()->setHorizontalTriangleFilterThreshold( mHorizontalTriangleThresholdSpinBox->value() );
  mLayer->renderer()->setHorizontalTriangleFilterUnit( mHorizontalTriangleUnitWidget->unit() );

  mLayer->renderer()->setShowLabels( mLabels->isChecked() );
  mLayer->renderer()->setLabelTextFormat( new QgsTextFormat( mLabelOptions->textFormat() ) );
}

void QgsPointCloudRendererPropertiesWidget::rendererChanged()
{
  if ( cboRenderers->currentIndex() == -1 )
  {
    QgsDebugError( QStringLiteral( "No current item -- this should never happen!" ) );
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
