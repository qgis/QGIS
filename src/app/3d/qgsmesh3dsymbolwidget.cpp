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
#include "qgstriangularmesh.h"
#include "qgsmeshdataprovider.h"
#include "qgsmesh3dsymbol.h"
#include "qgsmeshlayer3drenderer.h"

QgsMesh3DSymbolWidget::QgsMesh3DSymbolWidget( QgsMeshLayer *meshLayer, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  spinHeight->setClearValue( 0.0 );

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
  connect( chkSmoothTriangles, &QCheckBox::clicked, this, &QgsMesh3DSymbolWidget::changed );
  connect( chkWireframe, &QCheckBox::clicked, this, &QgsMesh3DSymbolWidget::changed );
  connect( colorButtonWireframe, &QgsColorButton::colorChanged, this, &QgsMesh3DSymbolWidget::changed );
  connect( spinBoxWireframeLineWidth, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ),
           this, &QgsMesh3DSymbolWidget::changed );

  connect( mColorRampShaderMinMaxReloadButton, &QPushButton::clicked, this, &QgsMesh3DSymbolWidget::reloadColorRampShaderMinMax );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsMesh3DSymbolWidget::changed );

  connect( mColorRampShaderMinEdit, &QLineEdit::editingFinished, this, &QgsMesh3DSymbolWidget::onColorRampShaderMinMaxChanged );
  connect( mColorRampShaderMaxEdit, &QLineEdit::editingFinished, this, &QgsMesh3DSymbolWidget::onColorRampShaderMinMaxChanged );

  connect( mComboBoxTextureType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3DSymbolWidget::onColoringTypeChanged );
  connect( mComboBoxTextureType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3DSymbolWidget::changed );

  connect( mMeshUniqueColorButton, &QgsColorButton::colorChanged, this, &QgsMesh3DSymbolWidget::changed );

  connect( spinBoxVerticaleScale, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ),
           this, &QgsMesh3DSymbolWidget::changed );

  onSymbologyTypeChanged();
  onColoringTypeChanged();
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
  chkSmoothTriangles->setChecked( symbol.smoothedTriangles() );
  chkWireframe->setChecked( symbol.wireframeEnabled() );
  colorButtonWireframe->setColor( symbol.wireframeLineColor() );
  spinBoxWireframeLineWidth->setValue( symbol.wireframeLineWidth() );
  spinBoxVerticaleScale->setValue( symbol.verticaleScale() );
  mComboBoxTextureType->setCurrentIndex( symbol.meshTextureType() );
  mMeshUniqueColorButton->setColor( symbol.uniqueMeshColor() );
  mColorRampShaderWidget->setFromShader( symbol.colorRampShader() );

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
  // Simple symbology
  sym.setRendererType( static_cast<QgsMesh3DSymbol::RendererType>( mComboBoxSymbologyType->currentIndex() ) );
  sym.setHeight( spinHeight->value() );
  sym.setAltitudeClamping( static_cast<Qgs3DTypes::AltitudeClamping>( cboAltClamping->currentIndex() ) );
  sym.setAddBackFaces( chkAddBackFaces->isChecked() );
  sym.setMaterial( widgetMaterial->material() );

  // Advanced symbology
  sym.setSmoothedTriangles( chkSmoothTriangles->isChecked() );
  sym.setWireframeEnabled( chkWireframe->isChecked() );
  sym.setWireframeLineColor( colorButtonWireframe->color() );
  sym.setWireframeLineWidth( spinBoxWireframeLineWidth->value() );
  sym.setVerticaleScale( spinBoxVerticaleScale->value() );
  sym.setMeshTextureType( static_cast<QgsMesh3DSymbol::TextureType>( mComboBoxTextureType->currentIndex() ) );
  sym.setUniqueMeshColor( mMeshUniqueColorButton->color() );
  sym.setColorRampShader( mColorRampShaderWidget->shader() );

  QgsPropertyCollection ddp;
  ddp.setProperty( QgsAbstract3DSymbol::PropertyHeight, btnHeightDD->toProperty() );
  sym.setDataDefinedProperties( ddp );

  return sym;
}

void QgsMesh3DSymbolWidget::setLayer( QgsMeshLayer *meshLayer )
{
  mLayer = meshLayer;

  if ( meshLayer )
  {
    QgsMeshLayer3DRenderer *renderer = static_cast<QgsMeshLayer3DRenderer *>( meshLayer->renderer3D() );
    if ( renderer &&  renderer->type() == QLatin1String( "mesh" ) )
    {
      if ( renderer->symbol() &&  renderer->symbol()->type() == QLatin1String( "mesh" ) )
      {
        setSymbol( *static_cast<const QgsMesh3DSymbol *>( renderer->symbol() ) );
        return;
      }
    }
  }
  setSymbol( QgsMesh3DSymbol() );
  reloadColorRampShaderMinMax(); //As the symbol is new, the Color ramp shader need to be initialise with min max value
}

int QgsMesh3DSymbolWidget::rendererTypeComboBoxIndex() const
{
  return mComboBoxSymbologyType->currentIndex();
}

void QgsMesh3DSymbolWidget::setRendererTypeComboBoxIndex( int index )
{
  mComboBoxTextureType->setCurrentIndex( index );
}


void QgsMesh3DSymbolWidget::reloadColorRampShaderMinMax()
{
  if ( !mLayer )
    return;

  QgsTriangularMesh *triangleMesh = mLayer->triangularMesh();
  double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::min();
  for ( int i = 0; i < triangleMesh->vertices().count(); ++i )
  {
    double zValue = triangleMesh->vertices().at( i ).z();
    if ( zValue > max )
      max = zValue;
    if ( zValue < min )
      min = zValue;
  }
  mColorRampShaderMaxEdit->blockSignals( true );
  mColorRampShaderMinEdit->blockSignals( true );
  mColorRampShaderMinEdit->setText( QString::number( min ) );
  mColorRampShaderMaxEdit->setText( QString::number( max ) );
  mColorRampShaderMaxEdit->blockSignals( false );
  mColorRampShaderMinEdit->blockSignals( false );

  mColorRampShaderWidget->setMinimumMaximum( min, max );
  mColorRampShaderWidget->classify();
}

void QgsMesh3DSymbolWidget::onSymbologyTypeChanged()
{
  mGroupBoxSimpleSettings->setVisible( mComboBoxSymbologyType->currentIndex() == 0 );
  mGroupBoxAdvancedSettings->setVisible( mComboBoxSymbologyType->currentIndex() == 1 );
}

void QgsMesh3DSymbolWidget::onColorRampShaderMinMaxChanged()
{
  double min = lineEditValue( mColorRampShaderMinEdit );
  double max = lineEditValue( mColorRampShaderMaxEdit );
  mColorRampShaderWidget->setMinimumMaximum( min, max );
  mColorRampShaderWidget->classify();
}

void QgsMesh3DSymbolWidget::onColoringTypeChanged()
{
  mGroupBoxColorRampShader->setVisible( mComboBoxTextureType->currentIndex() == QgsMesh3DSymbol::colorRamp );
  mMeshUniqueColorWidget->setVisible( mComboBoxTextureType->currentIndex() == QgsMesh3DSymbol::uniqueColor );
}
