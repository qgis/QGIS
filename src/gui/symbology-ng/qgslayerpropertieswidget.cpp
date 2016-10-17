/***************************************************************************
    qgslayerpropertieswidget.cpp
    ----------------------------
    begin                : June 2012
    copyright            : (C) 2012 by Arunmozhi
    email                : aruntheguy at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayerpropertieswidget.h"

#include <QFile>
#include <QStandardItem>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPicture>

#include "qgssymbollayer.h"
#include "qgssymbollayerregistry.h"

#include "qgsapplication.h"
#include "qgslogger.h"

#include "qgssymbollayerwidget.h"
#include "qgsarrowsymbollayerwidget.h"
#include "qgsellipsesymbollayerwidget.h"
#include "qgsvectorfieldsymbollayerwidget.h"
#include "qgssymbol.h" //for the unit
#include "qgspanelwidget.h"
#include "qgsdatadefined.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"

static bool _initWidgetFunction( const QString& name, QgsSymbolLayerWidgetFunc f )
{
  QgsSymbolLayerRegistry* reg = QgsSymbolLayerRegistry::instance();

  QgsSymbolLayerAbstractMetadata* abstractMetadata = reg->symbolLayerMetadata( name );
  if ( !abstractMetadata )
  {
    QgsDebugMsg( "Failed to find symbol layer's entry in registry: " + name );
    return false;
  }
  QgsSymbolLayerMetadata* metadata = dynamic_cast<QgsSymbolLayerMetadata*>( abstractMetadata );
  if ( !metadata )
  {
    QgsDebugMsg( "Failed to cast symbol layer's metadata: " + name );
    return false;
  }
  metadata->setWidgetFunction( f );
  return true;
}

static void _initWidgetFunctions()
{
  static bool initialized = false;
  if ( initialized )
    return;

  _initWidgetFunction( "SimpleLine", QgsSimpleLineSymbolLayerWidget::create );
  _initWidgetFunction( "MarkerLine", QgsMarkerLineSymbolLayerWidget::create );
  _initWidgetFunction( "ArrowLine", QgsArrowSymbolLayerWidget::create );

  _initWidgetFunction( "SimpleMarker", QgsSimpleMarkerSymbolLayerWidget::create );
  _initWidgetFunction( "FilledMarker", QgsFilledMarkerSymbolLayerWidget::create );
  _initWidgetFunction( "SvgMarker", QgsSvgMarkerSymbolLayerWidget::create );
  _initWidgetFunction( "FontMarker", QgsFontMarkerSymbolLayerWidget::create );
  _initWidgetFunction( "EllipseMarker", QgsEllipseSymbolLayerWidget::create );
  _initWidgetFunction( "VectorField", QgsVectorFieldSymbolLayerWidget::create );

  _initWidgetFunction( "SimpleFill", QgsSimpleFillSymbolLayerWidget::create );
  _initWidgetFunction( "GradientFill", QgsGradientFillSymbolLayerWidget::create );
  _initWidgetFunction( "ShapeburstFill", QgsShapeburstFillSymbolLayerWidget::create );
  _initWidgetFunction( "RasterFill", QgsRasterFillSymbolLayerWidget::create );
  _initWidgetFunction( "SVGFill", QgsSVGFillSymbolLayerWidget::create );
  _initWidgetFunction( "CentroidFill", QgsCentroidFillSymbolLayerWidget::create );
  _initWidgetFunction( "LinePatternFill", QgsLinePatternFillSymbolLayerWidget::create );
  _initWidgetFunction( "PointPatternFill", QgsPointPatternFillSymbolLayerWidget::create );

  _initWidgetFunction( "GeometryGenerator", QgsGeometryGeneratorSymbolLayerWidget::create );

  initialized = true;
}


QgsLayerPropertiesWidget::QgsLayerPropertiesWidget( QgsSymbolLayer* layer, const QgsSymbol* symbol, const QgsVectorLayer* vl, QWidget* parent )
    : QgsPanelWidget( parent )
    , mLayer( layer )
    , mSymbol( symbol )
    , mVectorLayer( vl )
{

  setupUi( this );
  // initialize the sub-widgets
  // XXX Should this thing be here this way? Initialize all the widgets just for the sake of one layer?
  // TODO Make this on demand creation
  _initWidgetFunctions();

  // TODO Algorithm
  //
  // 3. populate the combo box with the supported layer type
  // 4. set the present layer type
  // 5. create the widget for the present layer type and set inn stacked widget
  // 6. connect comboBox type changed to two things
  //     1. emit signal that type has beed changed
  //     2. remove the widget and place the new widget corresponding to the changed layer type
  //
  populateLayerTypes();
  // update layer type combo box
  int idx = cboLayerType->findData( mLayer->layerType() );
  cboLayerType->setCurrentIndex( idx );

  connect( mEnabledCheckBox, SIGNAL( toggled( bool ) ), mEnabledDDBtn, SLOT( setEnabled( bool ) ) );
  mEnabledCheckBox->setChecked( mLayer->enabled() );

  // set the corresponding widget
  updateSymbolLayerWidget( layer );
  connect( cboLayerType, SIGNAL( currentIndexChanged( int ) ), this, SLOT( layerTypeChanged() ) );

  connect( mEffectWidget, SIGNAL( changed() ), this, SLOT( emitSignalChanged() ) );

  this->connectChildPanel( mEffectWidget );

  mEffectWidget->setPaintEffect( mLayer->paintEffect() );

  const QgsDataDefined* dd = mLayer->getDataDefinedProperty( QgsSymbolLayer::EXPR_LAYER_ENABLED );
  mEnabledDDBtn->init( mVectorLayer, dd, QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );
  connect( mEnabledDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedEnable() ) );
  connect( mEnabledDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedEnable() ) );
  mEnabledDDBtn->registerExpressionContextGenerator( this );
}

void QgsLayerPropertiesWidget::updateDataDefinedEnable()
{
  QgsDataDefined* dd = mLayer->getDataDefinedProperty( QgsSymbolLayer::EXPR_LAYER_ENABLED );
  if ( !dd )
  {
    dd = new QgsDataDefined();
    mLayer->setDataDefinedProperty( QgsSymbolLayer::EXPR_LAYER_ENABLED, dd );
  }
  mEnabledDDBtn->updateDataDefined( dd );

  emit changed();
}

void QgsLayerPropertiesWidget::setContext( const QgsSymbolWidgetContext& context )
{
  mContext = context;

  QgsSymbolLayerWidget* w = dynamic_cast< QgsSymbolLayerWidget* >( stackedWidget->currentWidget() );
  if ( w )
    w->setContext( mContext );
}

QgsSymbolWidgetContext QgsLayerPropertiesWidget::context() const
{
  return mContext;
}

void QgsLayerPropertiesWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );
  mEffectWidget->setDockMode( this->dockMode() );
}

void QgsLayerPropertiesWidget::populateLayerTypes()
{
  QStringList symbolLayerIds = QgsSymbolLayerRegistry::instance()->symbolLayersForType( mSymbol->type() );

  Q_FOREACH ( const QString& symbolLayerId, symbolLayerIds )
    cboLayerType->addItem( QgsSymbolLayerRegistry::instance()->symbolLayerMetadata( symbolLayerId )->visibleName(), symbolLayerId );

  if ( mSymbol->type() == QgsSymbol::Fill )
  {
    QStringList lineLayerIds = QgsSymbolLayerRegistry::instance()->symbolLayersForType( QgsSymbol::Line );
    Q_FOREACH ( const QString& lineLayerId, lineLayerIds )
    {
      QgsSymbolLayerAbstractMetadata* layerInfo = QgsSymbolLayerRegistry::instance()->symbolLayerMetadata( lineLayerId );
      if ( layerInfo->type() != QgsSymbol::Hybrid )
      {
        QString visibleName = layerInfo->visibleName();
        QString name = QString( tr( "Outline: %1" ) ).arg( visibleName );
        cboLayerType->addItem( name, lineLayerId );
      }
    }
  }
}

void QgsLayerPropertiesWidget::updateSymbolLayerWidget( QgsSymbolLayer* layer )
{
  if ( stackedWidget->currentWidget() != pageDummy )
  {
    // stop updating from the original widget
    disconnect( stackedWidget->currentWidget(), SIGNAL( changed() ), this, SLOT( emitSignalChanged() ) );
    stackedWidget->removeWidget( stackedWidget->currentWidget() );
  }

  QgsSymbolLayerRegistry* pReg = QgsSymbolLayerRegistry::instance();

  QString layerType = layer->layerType();
  QgsSymbolLayerAbstractMetadata* am = pReg->symbolLayerMetadata( layerType );
  if ( am )
  {
    QgsSymbolLayerWidget* w = am->createSymbolLayerWidget( mVectorLayer );
    if ( w )
    {
      w->setContext( mContext );
      w->setSymbolLayer( layer );
      stackedWidget->addWidget( w );
      stackedWidget->setCurrentWidget( w );
      // start receiving updates from widget
      connect( w, SIGNAL( changed() ), this, SLOT( emitSignalChanged() ) );
      connect( w, SIGNAL( symbolChanged() ), this, SLOT( reloadLayer() ) );
      return;
    }
  }
  // When anything is not right
  stackedWidget->setCurrentWidget( pageDummy );
}

QgsExpressionContext QgsLayerPropertiesWidget::createExpressionContext() const
{
  if ( mContext.expressionContext() )
    return *mContext.expressionContext();

  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::atlasScope( nullptr );

  if ( mContext.mapCanvas() )
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( mContext.mapCanvas()->mapSettings() )
    << new QgsExpressionContextScope( mContext.mapCanvas()->expressionContextScope() );
  }
  else
  {
    expContext << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  expContext << QgsExpressionContextUtils::layerScope( mVectorLayer );

  QgsExpressionContextScope* symbolScope = QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );
  if ( mLayer )
  {
    //cheat a bit - set the symbol color variable to match the symbol layer's color (when we should really be using the *symbols*
    //color, but that's not accessible here). 99% of the time these will be the same anyway
    symbolScope->setVariable( QgsExpressionContext::EXPR_SYMBOL_COLOR, mLayer->color() );
  }
  expContext << symbolScope;
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_PART_NUM, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT, 1, true ) );
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, 1, true ) );

  // additional scopes
  Q_FOREACH ( const QgsExpressionContextScope& scope, mContext.additionalExpressionContextScopes() )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  //TODO - show actual value
  expContext.setOriginalValueVariable( QVariant() );

  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE << QgsExpressionContext::EXPR_SYMBOL_COLOR
                                      << QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT << QgsExpressionContext::EXPR_GEOMETRY_PART_NUM
                                      << QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT << QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM
                                      << QgsExpressionContext::EXPR_CLUSTER_COLOR << QgsExpressionContext::EXPR_CLUSTER_SIZE );

  return expContext;
}

void QgsLayerPropertiesWidget::layerTypeChanged()
{
  QgsSymbolLayer* layer = mLayer;
  if ( !layer )
    return;
  QString newLayerType = cboLayerType->currentData().toString();
  if ( layer->layerType() == newLayerType )
    return;

  // get creation function for new layer from registry
  QgsSymbolLayerRegistry* pReg = QgsSymbolLayerRegistry::instance();
  QgsSymbolLayerAbstractMetadata* am = pReg->symbolLayerMetadata( newLayerType );
  if ( !am ) // check whether the metadata is assigned
    return;

  // change layer to a new (with different type)
  // base new layer on existing layer's properties
  QgsSymbolLayer* newLayer = am->createSymbolLayer( layer->properties() );
  if ( !newLayer )
    return;

  updateSymbolLayerWidget( newLayer );
  emit changeLayer( newLayer );
}

void QgsLayerPropertiesWidget::emitSignalChanged()
{
  emit changed();

  // also update paint effect preview
  mEffectWidget->setPreviewPicture( QgsSymbolLayerUtils::symbolLayerPreviewPicture( mLayer, QgsUnitTypes::RenderMillimeters, QSize( 80, 80 ) ) );
  emit widgetChanged();
}

void QgsLayerPropertiesWidget::reloadLayer()
{
  emit changeLayer( mLayer );
}

void QgsLayerPropertiesWidget::on_mEnabledCheckBox_toggled( bool enabled )
{
  mLayer->setEnabled( enabled );
  emitSignalChanged();
}
