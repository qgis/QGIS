/***************************************************************************
  qgspolygon3dsymbolwidget.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspolygon3dsymbolwidget.h"

#include "qgspolygon3dsymbol.h"
#include "qgsphongmaterialsettings.h"

QgsPolygon3DSymbolWidget::QgsPolygon3DSymbolWidget( QWidget *parent )
  : Qgs3DSymbolWidget( parent )
{
  setupUi( this );
  spinHeight->setClearValue( 0.0 );
  spinExtrusion->setClearValue( 0.0 );
  spinEdgeWidth->setClearValue( 1.0 );

  QgsPolygon3DSymbol defaultSymbol;
  setSymbol( &defaultSymbol, nullptr );

  connect( spinHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( spinExtrusion, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboAltBinding, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboCullingMode, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboRenderedFacade, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( chkAddBackFaces, &QCheckBox::clicked, this, &QgsPolygon3DSymbolWidget::changed );
  connect( chkInvertNormals, &QCheckBox::clicked, this, &QgsPolygon3DSymbolWidget::changed );
  connect( widgetMaterial, &QgsMaterialWidget::changed, this, &QgsPolygon3DSymbolWidget::changed );
  connect( btnHeightDD, &QgsPropertyOverrideButton::changed, this, &QgsPolygon3DSymbolWidget::changed );
  connect( btnExtrusionDD, &QgsPropertyOverrideButton::changed, this, &QgsPolygon3DSymbolWidget::changed );
  connect( groupEdges, &QGroupBox::clicked, this, &QgsPolygon3DSymbolWidget::changed );
  connect( btnEdgeColor, &QgsColorButton::colorChanged, this, &QgsPolygon3DSymbolWidget::changed );
  connect( spinEdgeWidth, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );

  widgetMaterial->setTechnique( QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined );
}

Qgs3DSymbolWidget *QgsPolygon3DSymbolWidget::create( QgsVectorLayer * )
{
  return new QgsPolygon3DSymbolWidget();
}

void QgsPolygon3DSymbolWidget::setSymbol( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *layer )
{
  const QgsPolygon3DSymbol *polygonSymbol = dynamic_cast< const QgsPolygon3DSymbol * >( symbol );
  if ( !polygonSymbol )
    return;

  spinHeight->setValue( polygonSymbol->height() );
  spinExtrusion->setValue( polygonSymbol->extrusionHeight() );
  cboAltClamping->setCurrentIndex( static_cast<int>( polygonSymbol->altitudeClamping() ) );
  cboAltBinding->setCurrentIndex( static_cast<int>( polygonSymbol->altitudeBinding() ) );
  cboCullingMode->setCurrentIndex( static_cast<int>( polygonSymbol->cullingMode() ) );
  cboRenderedFacade->setCurrentIndex( polygonSymbol->renderedFacade() );

  chkAddBackFaces->setChecked( polygonSymbol->addBackFaces() );
  chkInvertNormals->setChecked( polygonSymbol->invertNormals() );

  widgetMaterial->setSettings( polygonSymbol->material(), layer );

  btnHeightDD->init( QgsAbstract3DSymbol::PropertyHeight, polygonSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), layer, true );
  btnExtrusionDD->init( QgsAbstract3DSymbol::PropertyExtrusionHeight, polygonSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), layer, true );

  groupEdges->setChecked( polygonSymbol->edgesEnabled() );
  spinEdgeWidth->setValue( polygonSymbol->edgeWidth() );
  btnEdgeColor->setColor( polygonSymbol->edgeColor() );
}

QgsAbstract3DSymbol *QgsPolygon3DSymbolWidget::symbol()
{
  std::unique_ptr< QgsPolygon3DSymbol > sym = std::make_unique< QgsPolygon3DSymbol >();
  sym->setHeight( spinHeight->value() );
  sym->setExtrusionHeight( spinExtrusion->value() );
  sym->setAltitudeClamping( static_cast<Qgis::AltitudeClamping>( cboAltClamping->currentIndex() ) );
  sym->setAltitudeBinding( static_cast<Qgis::AltitudeBinding>( cboAltBinding->currentIndex() ) );
  sym->setCullingMode( static_cast<Qgs3DTypes::CullingMode>( cboCullingMode->currentIndex() ) );
  sym->setRenderedFacade( cboRenderedFacade->currentIndex() );
  sym->setAddBackFaces( chkAddBackFaces->isChecked() );
  sym->setInvertNormals( chkInvertNormals->isChecked() );
  sym->setMaterial( widgetMaterial->settings() );

  QgsPropertyCollection ddp;
  ddp.setProperty( QgsAbstract3DSymbol::PropertyHeight, btnHeightDD->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::PropertyExtrusionHeight, btnExtrusionDD->toProperty() );
  sym->setDataDefinedProperties( ddp );

  sym->setEdgesEnabled( groupEdges->isChecked() );
  sym->setEdgeWidth( spinEdgeWidth->value() );
  sym->setEdgeColor( btnEdgeColor->color() );

  return sym.release();
}

QString QgsPolygon3DSymbolWidget::symbolType() const
{
  return QStringLiteral( "polygon" );
}
