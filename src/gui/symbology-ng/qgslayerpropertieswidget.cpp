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

#include "qgssymbollayerv2.h"
#include "qgssymbollayerv2registry.h"

#include "qgsapplication.h"
#include "qgslogger.h"

#include "qgssymbollayerv2widget.h"
#include "qgsarrowsymbollayerwidget.h"
#include "qgsellipsesymbollayerv2widget.h"
#include "qgsvectorfieldsymbollayerwidget.h"
#include "qgssymbolv2.h" //for the unit
#include "qgspanelwidget.h"

static bool _initWidgetFunction( const QString& name, QgsSymbolLayerV2WidgetFunc f )
{
  QgsSymbolLayerV2Registry* reg = QgsSymbolLayerV2Registry::instance();

  QgsSymbolLayerV2AbstractMetadata* abstractMetadata = reg->symbolLayerMetadata( name );
  if ( !abstractMetadata )
  {
    QgsDebugMsg( "Failed to find symbol layer's entry in registry: " + name );
    return false;
  }
  QgsSymbolLayerV2Metadata* metadata = dynamic_cast<QgsSymbolLayerV2Metadata*>( abstractMetadata );
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

  _initWidgetFunction( "SimpleLine", QgsSimpleLineSymbolLayerV2Widget::create );
  _initWidgetFunction( "MarkerLine", QgsMarkerLineSymbolLayerV2Widget::create );
  _initWidgetFunction( "ArrowLine", QgsArrowSymbolLayerWidget::create );

  _initWidgetFunction( "SimpleMarker", QgsSimpleMarkerSymbolLayerV2Widget::create );
  _initWidgetFunction( "FilledMarker", QgsFilledMarkerSymbolLayerWidget::create );
  _initWidgetFunction( "SvgMarker", QgsSvgMarkerSymbolLayerV2Widget::create );
  _initWidgetFunction( "FontMarker", QgsFontMarkerSymbolLayerV2Widget::create );
  _initWidgetFunction( "EllipseMarker", QgsEllipseSymbolLayerV2Widget::create );
  _initWidgetFunction( "VectorField", QgsVectorFieldSymbolLayerWidget::create );

  _initWidgetFunction( "SimpleFill", QgsSimpleFillSymbolLayerV2Widget::create );
  _initWidgetFunction( "GradientFill", QgsGradientFillSymbolLayerV2Widget::create );
  _initWidgetFunction( "ShapeburstFill", QgsShapeburstFillSymbolLayerV2Widget::create );
  _initWidgetFunction( "RasterFill", QgsRasterFillSymbolLayerWidget::create );
  _initWidgetFunction( "SVGFill", QgsSVGFillSymbolLayerWidget::create );
  _initWidgetFunction( "CentroidFill", QgsCentroidFillSymbolLayerV2Widget::create );
  _initWidgetFunction( "LinePatternFill", QgsLinePatternFillSymbolLayerWidget::create );
  _initWidgetFunction( "PointPatternFill", QgsPointPatternFillSymbolLayerWidget::create );

  _initWidgetFunction( "GeometryGenerator", QgsGeometryGeneratorSymbolLayerWidget::create );

  initialized = true;
}


QgsLayerPropertiesWidget::QgsLayerPropertiesWidget( QgsSymbolLayerV2* layer, const QgsSymbolV2* symbol, const QgsVectorLayer* vl, QWidget* parent )
    : QgsPanelWidget( parent )
    , mPresetExpressionContext( nullptr )
    , mMapCanvas( nullptr )
{

  mLayer = layer;
  mSymbol = symbol;
  mVectorLayer = vl;

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
  // set the corresponding widget
  updateSymbolLayerWidget( layer );
  connect( cboLayerType, SIGNAL( currentIndexChanged( int ) ), this, SLOT( layerTypeChanged() ) );

  connect( mEffectWidget, SIGNAL( changed() ), this, SLOT( emitSignalChanged() ) );

  this->connectChildPanel( mEffectWidget );

  mEffectWidget->setPaintEffect( mLayer->paintEffect() );
}

void QgsLayerPropertiesWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
  QgsSymbolLayerV2Widget* w = dynamic_cast< QgsSymbolLayerV2Widget* >( stackedWidget->currentWidget() );
  if ( w )
    w->setMapCanvas( mMapCanvas );
}

void QgsLayerPropertiesWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );
  mEffectWidget->setDockMode( this->dockMode() );
}

void QgsLayerPropertiesWidget::setExpressionContext( QgsExpressionContext *context )
{
  mPresetExpressionContext = context;

  QgsSymbolLayerV2Widget* w = dynamic_cast< QgsSymbolLayerV2Widget* >( stackedWidget->currentWidget() );
  if ( w )
    w->setExpressionContext( mPresetExpressionContext );
}

void QgsLayerPropertiesWidget::populateLayerTypes()
{
  QStringList symbolLayerIds = QgsSymbolLayerV2Registry::instance()->symbolLayersForType( mSymbol->type() );

  Q_FOREACH ( const QString& symbolLayerId, symbolLayerIds )
    cboLayerType->addItem( QgsSymbolLayerV2Registry::instance()->symbolLayerMetadata( symbolLayerId )->visibleName(), symbolLayerId );

  if ( mSymbol->type() == QgsSymbolV2::Fill )
  {
    QStringList lineLayerIds = QgsSymbolLayerV2Registry::instance()->symbolLayersForType( QgsSymbolV2::Line );
    Q_FOREACH ( const QString& lineLayerId, lineLayerIds )
    {
      QgsSymbolLayerV2AbstractMetadata* layerInfo = QgsSymbolLayerV2Registry::instance()->symbolLayerMetadata( lineLayerId );
      if ( layerInfo->type() != QgsSymbolV2::Hybrid )
      {
        QString visibleName = layerInfo->visibleName();
        QString name = QString( tr( "Outline: %1" ) ).arg( visibleName );
        cboLayerType->addItem( name, lineLayerId );
      }
    }
  }
}

void QgsLayerPropertiesWidget::updateSymbolLayerWidget( QgsSymbolLayerV2* layer )
{
  if ( stackedWidget->currentWidget() != pageDummy )
  {
    // stop updating from the original widget
    disconnect( stackedWidget->currentWidget(), SIGNAL( changed() ), this, SLOT( emitSignalChanged() ) );
    stackedWidget->removeWidget( stackedWidget->currentWidget() );
  }

  QgsSymbolLayerV2Registry* pReg = QgsSymbolLayerV2Registry::instance();

  QString layerType = layer->layerType();
  QgsSymbolLayerV2AbstractMetadata* am = pReg->symbolLayerMetadata( layerType );
  if ( am )
  {
    QgsSymbolLayerV2Widget* w = am->createSymbolLayerWidget( mVectorLayer );
    if ( w )
    {
      w->setExpressionContext( mPresetExpressionContext );
      if ( mMapCanvas )
        w->setMapCanvas( mMapCanvas );
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

void QgsLayerPropertiesWidget::layerTypeChanged()
{
  QgsSymbolLayerV2* layer = mLayer;
  if ( !layer )
    return;
  QString newLayerType = cboLayerType->itemData( cboLayerType->currentIndex() ).toString();
  if ( layer->layerType() == newLayerType )
    return;

  // get creation function for new layer from registry
  QgsSymbolLayerV2Registry* pReg = QgsSymbolLayerV2Registry::instance();
  QgsSymbolLayerV2AbstractMetadata* am = pReg->symbolLayerMetadata( newLayerType );
  if ( !am ) // check whether the metadata is assigned
    return;

  // change layer to a new (with different type)
  // base new layer on existing layer's properties
  QgsSymbolLayerV2* newLayer = am->createSymbolLayer( layer->properties() );
  if ( !newLayer )
    return;

  updateSymbolLayerWidget( newLayer );
  emit changeLayer( newLayer );
}

void QgsLayerPropertiesWidget::emitSignalChanged()
{
  emit changed();

  // also update paint effect preview
  mEffectWidget->setPreviewPicture( QgsSymbolLayerV2Utils::symbolLayerPreviewPicture( mLayer, QgsSymbolV2::MM, QSize( 80, 80 ) ) );
  emit widgetChanged();
}

void QgsLayerPropertiesWidget::reloadLayer()
{
  emit changeLayer( mLayer );
}
