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

#include "qgssymbollayerv2.h"
#include "qgssymbollayerv2registry.h"

#include "qgsapplication.h"
#include "qgslogger.h"

#include "qgssymbollayerv2widget.h"
#include "qgsellipsesymbollayerv2widget.h"
#include "qgsvectorfieldsymbollayerwidget.h"
#include "qgssymbolv2.h" //for the unit

static bool _initWidgetFunction( QString name, QgsSymbolLayerV2WidgetFunc f )
{
  QgsSymbolLayerV2Registry* reg = QgsSymbolLayerV2Registry::instance();

  QgsSymbolLayerV2AbstractMetadata* abstractMetadata = reg->symbolLayerMetadata( name );
  if ( abstractMetadata == NULL )
  {
    QgsDebugMsg( "Failed to find symbol layer's entry in registry: " + name );
    return false;
  }
  QgsSymbolLayerV2Metadata* metadata = dynamic_cast<QgsSymbolLayerV2Metadata*>( abstractMetadata );
  if ( metadata == NULL )
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
  _initWidgetFunction( "LineDecoration", QgsLineDecorationSymbolLayerV2Widget::create );

  _initWidgetFunction( "SimpleMarker", QgsSimpleMarkerSymbolLayerV2Widget::create );
  _initWidgetFunction( "SvgMarker", QgsSvgMarkerSymbolLayerV2Widget::create );
  _initWidgetFunction( "FontMarker", QgsFontMarkerSymbolLayerV2Widget::create );
  _initWidgetFunction( "EllipseMarker", QgsEllipseSymbolLayerV2Widget::create );
  _initWidgetFunction( "VectorField", QgsVectorFieldSymbolLayerWidget::create );

  _initWidgetFunction( "SimpleFill", QgsSimpleFillSymbolLayerV2Widget::create );
  _initWidgetFunction( "SVGFill", QgsSVGFillSymbolLayerWidget::create );
  _initWidgetFunction( "CentroidFill", QgsCentroidFillSymbolLayerV2Widget::create );
  _initWidgetFunction( "LinePatternFill", QgsLinePatternFillSymbolLayerWidget::create );
  _initWidgetFunction( "PointPatternFill", QgsPointPatternFillSymbolLayerWidget::create );

  initialized = true;
}


QgsLayerPropertiesWidget::QgsLayerPropertiesWidget( QgsSymbolLayerV2* layer, const QgsSymbolV2* symbol, const QgsVectorLayer* vl, QWidget* parent )
    : QWidget( parent )
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
}

void QgsLayerPropertiesWidget::populateLayerTypes()
{
  QStringList types = QgsSymbolLayerV2Registry::instance()->symbolLayersForType( mSymbol->type() );

  for ( int i = 0; i < types.count(); i++ )
    cboLayerType->addItem( QgsSymbolLayerV2Registry::instance()->symbolLayerMetadata( types[i] )->visibleName(), types[i] );

  if ( mSymbol->type() == QgsSymbolV2::Fill )
  {
    QStringList typesLine = QgsSymbolLayerV2Registry::instance()->symbolLayersForType( QgsSymbolV2::Line );
    for ( int i = 0; i < typesLine.count(); i++ )
    {
      QString visibleName = QgsSymbolLayerV2Registry::instance()->symbolLayerMetadata( typesLine[i] )->visibleName();
      QString name = QString( tr( "Outline: %1" ) ).arg( visibleName );
      cboLayerType->addItem( name, typesLine[i] );
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
      w->setSymbolLayer( layer );
      stackedWidget->addWidget( w );
      stackedWidget->setCurrentWidget( w );
      // start recieving updates from widget
      connect( w , SIGNAL( changed() ), this, SLOT( emitSignalChanged() ) );
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
  if ( am == NULL ) // check whether the metadata is assigned
    return;

  // change layer to a new (with different type)
  QgsSymbolLayerV2* newLayer = am->createSymbolLayer( QgsStringMap() );
  if ( newLayer == NULL )
    return;

  updateSymbolLayerWidget( newLayer );
  emit changeLayer( newLayer );
}

void QgsLayerPropertiesWidget::emitSignalChanged()
{
  emit changed();
}
