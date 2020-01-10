/***************************************************************************
  qgsmesh3dsymbolpropertieswidget.cpp
  -------------------------
  Date                 : January 2020
  Copyright            : (C) 2020 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3dsymbolpropertieswidget.h"
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsmeshdataprovider.h"
#include "qgsmesh3dsymbol.h"
#include "qgsmeshlayer3drenderer.h"

QgsMesh3dSymbolPropertiesWidget::QgsMesh3dSymbolPropertiesWidget( QgsMeshLayer *meshLayer, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  setLayer( meshLayer );

  connect( chkSmoothTriangles, &QCheckBox::clicked, this, &QgsMesh3dSymbolPropertiesWidget::changed );
  connect( chkWireframe, &QCheckBox::clicked, this, &QgsMesh3dSymbolPropertiesWidget::changed );
  connect( colorButtonWireframe, &QgsColorButton::colorChanged, this, &QgsMesh3dSymbolPropertiesWidget::changed );
  connect( spinBoxWireframeLineWidth, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ),
           this, &QgsMesh3dSymbolPropertiesWidget::changed );

  connect( mColorRampShaderMinMaxReloadButton, &QPushButton::clicked, this, &QgsMesh3dSymbolPropertiesWidget::reloadColorRampShaderMinMax );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsMesh3dSymbolPropertiesWidget::changed );

  connect( mColorRampShaderMinEdit, &QLineEdit::editingFinished, this, &QgsMesh3dSymbolPropertiesWidget::onColorRampShaderMinMaxChanged );
  connect( mColorRampShaderMaxEdit, &QLineEdit::editingFinished, this, &QgsMesh3dSymbolPropertiesWidget::onColorRampShaderMinMaxChanged );

  connect( mComboBoxTextureType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3dSymbolPropertiesWidget::onColoringTypeChanged );
  connect( mComboBoxTextureType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3dSymbolPropertiesWidget::changed );

  connect( mMeshUniqueColorButton, &QgsColorButton::colorChanged, this, &QgsMesh3dSymbolPropertiesWidget::changed );

  connect( spinBoxVerticaleScale, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ),
           this, &QgsMesh3dSymbolPropertiesWidget::changed );

  onColoringTypeChanged();
}

void QgsMesh3dSymbolPropertiesWidget::setSymbol( const QgsMesh3DSymbol &symbol )
{
  chkSmoothTriangles->setChecked( symbol.smoothedTriangles() );
  chkWireframe->setChecked( symbol.wireframeEnabled() );
  colorButtonWireframe->setColor( symbol.wireframeLineColor() );
  spinBoxWireframeLineWidth->setValue( symbol.wireframeLineWidth() );
  spinBoxVerticaleScale->setValue( symbol.verticaleScale() );
  mComboBoxTextureType->setCurrentIndex( symbol.meshTextureType() );
  mMeshUniqueColorButton->setColor( symbol.uniqueMeshColor() );
  mColorRampShaderWidget->setFromShader( symbol.colorRampShader() );
}

void QgsMesh3dSymbolPropertiesWidget::enableVerticalSetting( bool isEnable )
{
  mGroupBoxVerticaleSettings->setVisible( isEnable );
}

double QgsMesh3dSymbolPropertiesWidget::lineEditValue( const QLineEdit *lineEdit ) const
{
  if ( lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return lineEdit->text().toDouble();
}

QgsMesh3DSymbol QgsMesh3dSymbolPropertiesWidget::symbol() const
{
  QgsMesh3DSymbol sym;

  // Advanced symbology
  sym.setSmoothedTriangles( chkSmoothTriangles->isChecked() );
  sym.setWireframeEnabled( chkWireframe->isChecked() );
  sym.setWireframeLineColor( colorButtonWireframe->color() );
  sym.setWireframeLineWidth( spinBoxWireframeLineWidth->value() );
  sym.setVerticaleScale( spinBoxVerticaleScale->value() );
  sym.setMeshTextureType( static_cast<QgsMesh3DSymbol::TextureType>( mComboBoxTextureType->currentIndex() ) );
  sym.setUniqueMeshColor( mMeshUniqueColorButton->color() );
  sym.setColorRampShader( mColorRampShaderWidget->shader() );

  return sym;
}

void QgsMesh3dSymbolPropertiesWidget::setLayer( QgsMeshLayer *meshLayer, bool updateSymbol )
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
  reloadColorRampShaderMinMax(); //As the symbol is new, the Color ramp shader need to be initialise with min max value
}


void QgsMesh3dSymbolPropertiesWidget::reloadColorRampShaderMinMax()
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
  mColorRampShaderMaxEdit->blockSignals( true );
  mColorRampShaderMinEdit->blockSignals( true );
  mColorRampShaderMinEdit->setText( QString::number( min ) );
  mColorRampShaderMaxEdit->setText( QString::number( max ) );
  mColorRampShaderMaxEdit->blockSignals( false );
  mColorRampShaderMinEdit->blockSignals( false );

  mColorRampShaderWidget->setMinimumMaximum( min, max );
  mColorRampShaderWidget->classify();
}


void QgsMesh3dSymbolPropertiesWidget::onColorRampShaderMinMaxChanged()
{
  double min = lineEditValue( mColorRampShaderMinEdit );
  double max = lineEditValue( mColorRampShaderMaxEdit );
  mColorRampShaderWidget->setMinimumMaximum( min, max );
  mColorRampShaderWidget->classify();
}

void QgsMesh3dSymbolPropertiesWidget::onColoringTypeChanged()
{
  mGroupBoxColorRampShader->setVisible( mComboBoxTextureType->currentIndex() == QgsMesh3DSymbol::colorRamp );
  mMeshUniqueColorWidget->setVisible( mComboBoxTextureType->currentIndex() == QgsMesh3DSymbol::uniqueColor );
}
