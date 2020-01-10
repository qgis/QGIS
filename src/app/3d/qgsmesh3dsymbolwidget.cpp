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
#include "qgsmesh3dsymbolpropertieswidget.h"
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsmeshdataprovider.h"
#include "qgsmesh3dsymbol.h"
#include "qgsmeshlayer3drenderer.h"

QgsMesh3DSymbolWidget::QgsMesh3DSymbolWidget( QgsMeshLayer *meshLayer, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  spinHeight->setClearValue( 0.0 );

  mAdvancedSettingsWidget = new QgsMesh3dSymbolPropertiesWidget( meshLayer, this );
  mGroupBoxAdvancedSettings->layout()->addWidget( mAdvancedSettingsWidget );

  setLayer( meshLayer );

  connect( mComboBoxSymbologyType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3DSymbolWidget::onSymbologyTypeChanged );
  connect( mComboBoxSymbologyType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3DSymbolWidget::changed );

  connect( spinHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ),
           this, &QgsMesh3DSymbolWidget::changed );
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3DSymbolWidget::changed );
  connect( chkAddBackFaces, &QCheckBox::clicked, this, &QgsMesh3DSymbolWidget::changed );
  connect( widgetMaterial, &QgsPhongMaterialWidget::changed, this, &QgsMesh3DSymbolWidget::changed );
  connect( btnHeightDD, &QgsPropertyOverrideButton::changed, this, &QgsMesh3DSymbolWidget::changed );

  connect( mAdvancedSettingsWidget, &QgsMesh3dSymbolPropertiesWidget::changed,
           this, &QgsMesh3DSymbolWidget::changed );

  onSymbologyTypeChanged();
}

void QgsMesh3DSymbolWidget::setSymbol( const QgsMesh3DSymbol &symbol )
{
  mComboBoxSymbologyType->setCurrentIndex( symbol.renderType() );

  // Simple symbology
  spinHeight->setValue( symbol.height() );
  cboAltClamping->setCurrentIndex( static_cast<int>( symbol.altitudeClamping() ) );
  chkAddBackFaces->setChecked( symbol.addBackFaces() );
  widgetMaterial->setMaterial( symbol.material() );

  // Advanced symbology
  mAdvancedSettingsWidget->setSymbol( symbol );

  btnHeightDD->init( QgsAbstract3DSymbol::PropertyHeight, symbol.dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), nullptr, true );
}

double QgsMesh3DSymbolWidget::lineEditValue( const QLineEdit *lineEdit ) const
{
  if ( lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return lineEdit->text().toDouble();
}

QgsMesh3DSymbol QgsMesh3DSymbolWidget::symbol() const
{
  QgsMesh3DSymbol sym;

  // Advanced symbology
  sym = mAdvancedSettingsWidget->symbol();

  // Simple symbology
  sym.setRendererType( static_cast<QgsMesh3DSymbol::RendererType>( mComboBoxSymbologyType->currentIndex() ) );
  sym.setHeight( spinHeight->value() );
  sym.setAltitudeClamping( static_cast<Qgs3DTypes::AltitudeClamping>( cboAltClamping->currentIndex() ) );
  sym.setAddBackFaces( chkAddBackFaces->isChecked() );
  sym.setMaterial( widgetMaterial->material() );

  QgsPropertyCollection ddp;
  ddp.setProperty( QgsAbstract3DSymbol::PropertyHeight, btnHeightDD->toProperty() );
  sym.setDataDefinedProperties( ddp );

  return sym;
}

void QgsMesh3DSymbolWidget::setLayer( QgsMeshLayer *meshLayer )
{
  mLayer = meshLayer;
}

int QgsMesh3DSymbolWidget::rendererTypeComboBoxIndex() const
{
  return mComboBoxSymbologyType->currentIndex();
}

void QgsMesh3DSymbolWidget::setRendererTypeComboBoxIndex( int index )
{
  mComboBoxSymbologyType->setCurrentIndex( index );
}

void QgsMesh3DSymbolWidget::onSymbologyTypeChanged()
{
  mGroupBoxSimpleSettings->setVisible( mComboBoxSymbologyType->currentIndex() == 0 );
  mGroupBoxAdvancedSettings->setVisible( mComboBoxSymbologyType->currentIndex() == 1 );
}

