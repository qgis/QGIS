/***************************************************************************
  qgspointcloud3dsymbolwidget.cpp
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloud3dsymbolwidget.h"

#include "qgspointcloudlayer.h"
#include "qgspointcloud3dsymbol.h"
#include "qgspointcloudlayer3drenderer.h"

QgsPointCloud3DSymbolWidget::QgsPointCloud3DSymbolWidget( QgsPointCloud3DSymbol *symbol, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mPointSizeSpinBox->setClearValue( 2.0 );

  if ( symbol )
    setSymbol( symbol );

  connect( mPointSizeSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloud3DSymbolWidget::changed );
  connect( mRenderingStyleComboBox, qgis::overload< int >::of( &QComboBox::currentIndexChanged ), this, &QgsPointCloud3DSymbolWidget::onRenderingStyleChanged );
  connect( mColorRampShaderMinMaxReloadButton, &QPushButton::clicked, this, &QgsPointCloud3DSymbolWidget::reloadColorRampShaderMinMax );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsPointCloud3DSymbolWidget::changed );
}

void QgsPointCloud3DSymbolWidget::setSymbol( QgsPointCloud3DSymbol *symbol )
{
  mPointSizeSpinBox->setValue( symbol->pointSize() );
  mRenderingStyleComboBox->setCurrentIndex( symbol->renderingStyle() );
  mRenderingParameterComboBox->setCurrentIndex( symbol->renderingParameter() );
  mSingleColorBtn->setColor( symbol->singleColor() );
  QgsColorRampShader shader = symbol->colorRampShader();
  setColorRampMinMax( symbol->colorRampShaderMin(), symbol->colorRampShaderMax() );
  mColorRampShaderWidget->setFromShader( symbol->colorRampShader() );

  onRenderingStyleChanged( symbol->renderingStyle() );
}

QgsPointCloud3DSymbol *QgsPointCloud3DSymbolWidget::symbol() const
{
  QgsPointCloud3DSymbol *symb = new QgsPointCloud3DSymbol;
  symb->setPointSize( mPointSizeSpinBox->value() );
  symb->setRenderingStyle( static_cast< QgsPointCloud3DSymbol::RenderingStyle >( mRenderingStyleComboBox->currentIndex() ) );
  symb->setRenderingParameter( static_cast< QgsPointCloud3DSymbol::RenderingParameter >( mRenderingParameterComboBox->currentIndex() ) );
  symb->setSingleColor( mSingleColorBtn->color() );
  symb->setColorRampShader( mColorRampShaderWidget->shader() );
  symb->setColorRampShaderMinMax( mColorRampShaderMinEdit->value(), mColorRampShaderMaxEdit->value() );
  return symb;
}

void QgsPointCloud3DSymbolWidget::setColorRampMinMax( double min, double max )
{
  whileBlocking( mColorRampShaderMinEdit )->setValue( min );
  whileBlocking( mColorRampShaderMaxEdit )->setValue( max );
}

void QgsPointCloud3DSymbolWidget::reloadColorRampShaderMinMax()
{
  double min = mColorRampShaderMinEdit->value();
  double max = mColorRampShaderMaxEdit->value();
  mColorRampShaderWidget->setMinimumMaximum( min, max );
  mColorRampShaderWidget->classify();
}

void QgsPointCloud3DSymbolWidget::onRenderingStyleChanged( int current )
{
  QgsPointCloud3DSymbol::RenderingStyle currentStyle = static_cast< QgsPointCloud3DSymbol::RenderingStyle >( current );
  switch ( currentStyle )
  {
    case QgsPointCloud3DSymbol::RenderingStyle::SingleColor:
      mColorRampGroupBox->setVisible( false );
      mSingleColorGroupBox->setVisible( true );
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::ColorRamp:
      mColorRampGroupBox->setVisible( true );
      mSingleColorGroupBox->setVisible( false );
      break;
  }
}
