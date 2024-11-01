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
#include "moc_qgspolygon3dsymbolwidget.cpp"

#include "qgs3dtypes.h"
#include "qgspolygon3dsymbol.h"
#include "qgsphongmaterialsettings.h"

QgsPolygon3DSymbolWidget::QgsPolygon3DSymbolWidget( QWidget *parent )
  : Qgs3DSymbolWidget( parent )
{
  setupUi( this );
  spinOffset->setClearValue( 0.0 );
  spinExtrusion->setClearValue( 0.0 );
  spinEdgeWidth->setClearValue( 1.0 );

  cboCullingMode->addItem( tr( "No Culling" ), Qgs3DTypes::NoCulling );
  cboCullingMode->addItem( tr( "Front" ), Qgs3DTypes::Front );
  cboCullingMode->addItem( tr( "Back" ), Qgs3DTypes::Back );

  cboCullingMode->setItemData( 0, tr( "Both sides of the shapes are visible" ), Qt::ToolTipRole );
  cboCullingMode->setItemData( 1, tr( "Only the back of the shapes is visible" ), Qt::ToolTipRole );
  cboCullingMode->setItemData( 2, tr( "Only the front of the shapes is visible" ), Qt::ToolTipRole );

  QgsPolygon3DSymbol defaultSymbol;
  setSymbol( &defaultSymbol, nullptr );

  connect( spinOffset, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( spinExtrusion, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPolygon3DSymbolWidget::changed );
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
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::changed );
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPolygon3DSymbolWidget::updateGuiState );

  widgetMaterial->setTechnique( QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined );
}

Qgs3DSymbolWidget *QgsPolygon3DSymbolWidget::create( QgsVectorLayer * )
{
  return new QgsPolygon3DSymbolWidget();
}

void QgsPolygon3DSymbolWidget::setSymbol( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *layer )
{
  const QgsPolygon3DSymbol *polygonSymbol = dynamic_cast<const QgsPolygon3DSymbol *>( symbol );
  if ( !polygonSymbol )
    return;

  spinOffset->setValue( polygonSymbol->offset() );
  spinExtrusion->setValue( polygonSymbol->extrusionHeight() );
  cboAltClamping->setCurrentIndex( static_cast<int>( polygonSymbol->altitudeClamping() ) );
  cboAltBinding->setCurrentIndex( static_cast<int>( polygonSymbol->altitudeBinding() ) );
  cboCullingMode->setCurrentIndex( cboCullingMode->findData( polygonSymbol->cullingMode() ) );
  cboRenderedFacade->setCurrentIndex( polygonSymbol->renderedFacade() );

  chkAddBackFaces->setChecked( polygonSymbol->addBackFaces() );
  chkInvertNormals->setChecked( polygonSymbol->invertNormals() );

  widgetMaterial->setSettings( polygonSymbol->materialSettings(), layer );

  btnHeightDD->init( static_cast<int>( QgsAbstract3DSymbol::Property::Height ), polygonSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), layer, true );
  btnExtrusionDD->init( static_cast<int>( QgsAbstract3DSymbol::Property::ExtrusionHeight ), polygonSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), layer, true );

  groupEdges->setChecked( polygonSymbol->edgesEnabled() );
  spinEdgeWidth->setValue( polygonSymbol->edgeWidth() );
  btnEdgeColor->setColor( polygonSymbol->edgeColor() );
}

QgsAbstract3DSymbol *QgsPolygon3DSymbolWidget::symbol()
{
  std::unique_ptr<QgsPolygon3DSymbol> sym = std::make_unique<QgsPolygon3DSymbol>();
  sym->setOffset( static_cast<float>( spinOffset->value() ) );
  sym->setExtrusionHeight( spinExtrusion->value() );
  sym->setAltitudeClamping( static_cast<Qgis::AltitudeClamping>( cboAltClamping->currentIndex() ) );
  sym->setAltitudeBinding( static_cast<Qgis::AltitudeBinding>( cboAltBinding->currentIndex() ) );
  sym->setCullingMode( static_cast<Qgs3DTypes::CullingMode>( cboCullingMode->currentData().toInt() ) );
  sym->setRenderedFacade( cboRenderedFacade->currentIndex() );
  sym->setAddBackFaces( chkAddBackFaces->isChecked() );
  sym->setInvertNormals( chkInvertNormals->isChecked() );
  sym->setMaterialSettings( widgetMaterial->settings() );

  QgsPropertyCollection ddp;
  ddp.setProperty( QgsAbstract3DSymbol::Property::Height, btnHeightDD->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::Property::ExtrusionHeight, btnExtrusionDD->toProperty() );
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

void QgsPolygon3DSymbolWidget::updateGuiState()
{
  // Altitude binding is not taken into account if altitude clamping is absolute.
  // See: Qgs3DUtils::clampAltitudes()
  const bool absoluteClamping = cboAltClamping->currentIndex() == static_cast<int>( Qgis::AltitudeClamping::Absolute );
  cboAltBinding->setEnabled( !absoluteClamping );
}
