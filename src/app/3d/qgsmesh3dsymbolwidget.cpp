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

QgsMesh3dSymbolWidget::QgsMesh3dSymbolWidget( QgsMeshLayer *meshLayer, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  setLayer( meshLayer );

  connect( chkSmoothTriangles, &QCheckBox::clicked, this, &QgsMesh3dSymbolWidget::changed );
  connect( chkWireframe, &QCheckBox::clicked, this, &QgsMesh3dSymbolWidget::changed );
  connect( colorButtonWireframe, &QgsColorButton::colorChanged, this, &QgsMesh3dSymbolWidget::changed );
  connect( spinBoxWireframeLineWidth, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ),
           this, &QgsMesh3dSymbolWidget::changed );

  connect( mColorRampShaderMinMaxReloadButton, &QPushButton::clicked, this, &QgsMesh3dSymbolWidget::reloadColorRampShaderMinMax );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsMesh3dSymbolWidget::changed );

  connect( mColorRampShaderMinEdit, &QLineEdit::editingFinished, this, &QgsMesh3dSymbolWidget::onColorRampShaderMinMaxChanged );
  connect( mColorRampShaderMaxEdit, &QLineEdit::editingFinished, this, &QgsMesh3dSymbolWidget::onColorRampShaderMinMaxChanged );

  connect( mComboBoxTextureType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3dSymbolWidget::onColoringTypeChanged );
  connect( mComboBoxTextureType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3dSymbolWidget::changed );

  connect( mMeshUniqueColorButton, &QgsColorButton::colorChanged, this, &QgsMesh3dSymbolWidget::changed );

  connect( spinBoxVerticaleScale, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ),
           this, &QgsMesh3dSymbolWidget::changed );

  onColoringTypeChanged();
}

void QgsMesh3dSymbolWidget::setSymbol( const QgsMesh3DSymbol &symbol )
{
  // Advanced symbology
  chkSmoothTriangles->setChecked( symbol.smoothedTriangles() );
  chkWireframe->setChecked( symbol.wireframeEnabled() );
  colorButtonWireframe->setColor( symbol.wireframeLineColor() );
  spinBoxWireframeLineWidth->setValue( symbol.wireframeLineWidth() );
  spinBoxVerticaleScale->setValue( symbol.verticaleScale() );
  mComboBoxTextureType->setCurrentIndex( symbol.renderingStyle() );
  mMeshUniqueColorButton->setColor( symbol.uniqueMeshColor() );
  mColorRampShaderWidget->setFromShader( symbol.colorRampShader() );

  setColorRampMinMax( symbol.colorRampShader().minimumValue(), symbol.colorRampShader().maximumValue() );
}

void QgsMesh3dSymbolWidget::setLayer( QgsMeshLayer *meshLayer, bool updateSymbol )
{
  mLayer = meshLayer;

  if ( !updateSymbol )
    return;

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
  reloadColorRampShaderMinMax(); //As the symbol is new, the Color ramp shader needs to be initialized with min max value
}

double QgsMesh3dSymbolWidget::lineEditValue( const QLineEdit *lineEdit ) const
{
  if ( lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return lineEdit->text().toDouble();
}

QgsMesh3DSymbol QgsMesh3dSymbolWidget::symbol() const
{
  QgsMesh3DSymbol sym;

  sym.setSmoothedTriangles( chkSmoothTriangles->isChecked() );
  sym.setWireframeEnabled( chkWireframe->isChecked() );
  sym.setWireframeLineColor( colorButtonWireframe->color() );
  sym.setWireframeLineWidth( spinBoxWireframeLineWidth->value() );
  sym.setVerticaleScale( spinBoxVerticaleScale->value() );
  sym.setRenderingStyle( static_cast<QgsMesh3DSymbol::RenderingStyle>( mComboBoxTextureType->currentIndex() ) );
  sym.setUniqueMeshColor( mMeshUniqueColorButton->color() );
  sym.setColorRampShader( mColorRampShaderWidget->shader() );

  return sym;
}

void QgsMesh3dSymbolWidget::reloadColorRampShaderMinMax()
{
  if ( !mLayer )
    return;

  QgsTriangularMesh *triangleMesh = mLayer->triangularMesh();
  if ( !triangleMesh )
    return;

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
  setColorRampMinMax( min, max );

  mColorRampShaderWidget->setMinimumMaximum( min, max );
  mColorRampShaderWidget->classify();
}

void QgsMesh3dSymbolWidget::onColorRampShaderMinMaxChanged()
{
  double min = lineEditValue( mColorRampShaderMinEdit );
  double max = lineEditValue( mColorRampShaderMaxEdit );
  mColorRampShaderWidget->setMinimumMaximum( min, max );
  mColorRampShaderWidget->classify();
}

void QgsMesh3dSymbolWidget::onColoringTypeChanged()
{
  mGroupBoxColorRampShader->setVisible( mComboBoxTextureType->currentIndex() == QgsMesh3DSymbol::ColorRamp );
  mMeshUniqueColorWidget->setVisible( mComboBoxTextureType->currentIndex() == QgsMesh3DSymbol::UniqueColor );
}

void QgsMesh3dSymbolWidget::setColorRampMinMax( double min, double max )
{
  whileBlocking( mColorRampShaderMinEdit )->setText( QString::number( min ) );
  whileBlocking( mColorRampShaderMaxEdit )->setText( QString::number( max ) );
}

void QgsMesh3dSymbolWidget::enableVerticalSetting( bool isEnable )
{
  mGroupBoxVerticaleSettings->setVisible( isEnable );
}

