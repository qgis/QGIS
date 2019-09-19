/***************************************************************************
  qgsline3dsymbolwidget.cpp
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

#include "qgsline3dsymbolwidget.h"

#include "qgsline3dsymbol.h"


QgsLine3DSymbolWidget::QgsLine3DSymbolWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  spinHeight->setClearValue( 0.0 );
  spinWidth->setClearValue( 0.0 );
  spinExtrusion->setClearValue( 0.0 );

  setSymbol( QgsLine3DSymbol() );

  connect( spinWidth, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( spinHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( spinExtrusion, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( cboAltBinding, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( chkSimpleLines, &QCheckBox::clicked, this, &QgsLine3DSymbolWidget::changed );
  connect( chkSimpleLines, &QCheckBox::clicked, this, &QgsLine3DSymbolWidget::updateGuiState );
  connect( widgetMaterial, &QgsPhongMaterialWidget::changed, this, &QgsLine3DSymbolWidget::changed );
}

void QgsLine3DSymbolWidget::setSymbol( const QgsLine3DSymbol &symbol )
{
  spinWidth->setValue( symbol.width() );
  spinHeight->setValue( symbol.height() );
  spinExtrusion->setValue( symbol.extrusionHeight() );
  cboAltClamping->setCurrentIndex( static_cast<int>( symbol.altitudeClamping() ) );
  cboAltBinding->setCurrentIndex( static_cast<int>( symbol.altitudeBinding() ) );
  chkSimpleLines->setChecked( symbol.renderAsSimpleLines() );
  widgetMaterial->setMaterial( symbol.material() );
  updateGuiState();
}

QgsLine3DSymbol QgsLine3DSymbolWidget::symbol() const
{
  QgsLine3DSymbol sym;
  sym.setWidth( spinWidth->value() );
  sym.setHeight( spinHeight->value() );
  sym.setExtrusionHeight( spinExtrusion->value() );
  sym.setAltitudeClamping( static_cast<Qgs3DTypes::AltitudeClamping>( cboAltClamping->currentIndex() ) );
  sym.setAltitudeBinding( static_cast<Qgs3DTypes::AltitudeBinding>( cboAltBinding->currentIndex() ) );
  sym.setRenderAsSimpleLines( chkSimpleLines->isChecked() );
  sym.setMaterial( widgetMaterial->material() );
  return sym;
}

void QgsLine3DSymbolWidget::updateGuiState()
{
  bool simple = chkSimpleLines->isChecked();
  //spinWidth->setEnabled( !simple );
  spinExtrusion->setEnabled( !simple );
}
