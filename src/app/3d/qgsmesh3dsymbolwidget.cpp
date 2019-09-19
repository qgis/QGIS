/***************************************************************************
  qgsmesh3dsymbolwidget.cpp
  -------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3dsymbolwidget.h"
#include "qgsmeshlayer.h"
#include "qgsmesh3dsymbol.h"


QgsMesh3DSymbolWidget::QgsMesh3DSymbolWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  spinHeight->setClearValue( 0.0 );

  setSymbol( QgsMesh3DSymbol(), nullptr );

  connect( spinHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsMesh3DSymbolWidget::changed );
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsMesh3DSymbolWidget::changed );
  connect( chkAddBackFaces, &QCheckBox::clicked, this, &QgsMesh3DSymbolWidget::changed );
  connect( widgetMaterial, &QgsPhongMaterialWidget::changed, this, &QgsMesh3DSymbolWidget::changed );
  connect( btnHeightDD, &QgsPropertyOverrideButton::changed, this, &QgsMesh3DSymbolWidget::changed );
}

void QgsMesh3DSymbolWidget::setSymbol( const QgsMesh3DSymbol &symbol, QgsMeshLayer *layer )
{
  Q_UNUSED( layer )

  spinHeight->setValue( symbol.height() );
  cboAltClamping->setCurrentIndex( static_cast<int>( symbol.altitudeClamping() ) );
  chkAddBackFaces->setChecked( symbol.addBackFaces() );
  widgetMaterial->setMaterial( symbol.material() );

  btnHeightDD->init( QgsAbstract3DSymbol::PropertyHeight, symbol.dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), nullptr, true );
}

QgsMesh3DSymbol QgsMesh3DSymbolWidget::symbol() const
{
  QgsMesh3DSymbol sym;
  sym.setHeight( spinHeight->value() );
  sym.setAltitudeClamping( static_cast<Qgs3DTypes::AltitudeClamping>( cboAltClamping->currentIndex() ) );
  sym.setAddBackFaces( chkAddBackFaces->isChecked() );
  sym.setMaterial( widgetMaterial->material() );

  QgsPropertyCollection ddp;
  ddp.setProperty( QgsAbstract3DSymbol::PropertyHeight, btnHeightDD->toProperty() );
  sym.setDataDefinedProperties( ddp );

  return sym;
}
