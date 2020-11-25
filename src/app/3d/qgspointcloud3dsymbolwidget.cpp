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

QgsPointCloud3DSymbolWidget::QgsPointCloud3DSymbolWidget( QgsPointCloudLayer *layer, QgsPointCloud3DSymbol *symbol, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  setupUi( this );

  mPointSizeSpinBox->setClearValue( 2.0 );

  for ( QgsPointCloudAttribute attr : layer->attributes().attributes() )
  {
    mRenderingParameterComboBox->addItem( attr.name() );
  }

  if ( symbol )
    setSymbol( symbol );

  connect( mPointSizeSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloud3DSymbolWidget::changed );
  connect( mRenderingStyleComboBox, qgis::overload< int >::of( &QComboBox::currentIndexChanged ), this, &QgsPointCloud3DSymbolWidget::onRenderingStyleChanged );
  connect( mColorRampShaderMinMaxReloadButton, &QPushButton::clicked, this, &QgsPointCloud3DSymbolWidget::reloadColorRampShaderMinMax );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsPointCloud3DSymbolWidget::changed );
}

void QgsPointCloud3DSymbolWidget::setSymbol( QgsPointCloud3DSymbol *symbol )
{
  mRenderingStyleComboBox->setCurrentIndex( symbol->renderingStyle() );
  switch ( symbol->renderingStyle() )
  {
    case QgsPointCloud3DSymbol::RenderingStyle::NoRendering:
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::SingleColor:
    {
      QgsSingleColorPointCloud3DSymbol *symb = dynamic_cast<QgsSingleColorPointCloud3DSymbol *>( symbol );
      mPointSizeSpinBox->setValue( symb->pointSize() );
      mSingleColorBtn->setColor( symb->singleColor() );
      break;
    }
    case QgsPointCloud3DSymbol::RenderingStyle::ColorRamp:
    {
      QgsColorRampPointCloud3DSymbol *symb = dynamic_cast<QgsColorRampPointCloud3DSymbol *>( symbol );
      mPointSizeSpinBox->setValue( symb->pointSize() );
      mRenderingParameterComboBox->setCurrentText( symb->renderingParameter() );
      QgsColorRampShader shader = symb->colorRampShader();
      setColorRampMinMax( symb->colorRampShaderMin(), symb->colorRampShaderMax() );
      mColorRampShaderWidget->setFromShader( symb->colorRampShader() );
      break;
    }
    case QgsPointCloud3DSymbol::RenderingStyle::RGBRendering:
    {
      QgsRGBPointCloud3DSymbol *symb = dynamic_cast<QgsRGBPointCloud3DSymbol *>( symbol );
      mPointSizeSpinBox->setValue( symb->pointSize() );
      break;
    }
  }

  onRenderingStyleChanged( symbol->renderingStyle() );
}

QgsPointCloud3DSymbol *QgsPointCloud3DSymbolWidget::symbol() const
{

  QgsPointCloud3DSymbol *ret_symb = nullptr;
  QgsPointCloud3DSymbol::RenderingStyle renderingStyle = static_cast< QgsPointCloud3DSymbol::RenderingStyle >( mRenderingStyleComboBox->currentIndex() );

  switch ( renderingStyle )
  {
    case QgsPointCloud3DSymbol::RenderingStyle::NoRendering:
    {
      ret_symb = new QgsNoRenderingPointCloud3DSymbol( mLayer );
      break;
    }
    case QgsPointCloud3DSymbol::RenderingStyle::SingleColor:
    {
      QgsSingleColorPointCloud3DSymbol *symb = new QgsSingleColorPointCloud3DSymbol( mLayer );
      symb->setPointSize( mPointSizeSpinBox->value() );
      symb->setSingleColor( mSingleColorBtn->color() );
      ret_symb = symb;
      break;
    }
    case QgsPointCloud3DSymbol::RenderingStyle::ColorRamp:
    {
      QgsColorRampPointCloud3DSymbol *symb = new QgsColorRampPointCloud3DSymbol( mLayer );
      symb->setRenderingParameter( mRenderingParameterComboBox->currentText() );
      symb->setPointSize( mPointSizeSpinBox->value() );
      symb->setColorRampShader( mColorRampShaderWidget->shader() );
      symb->setColorRampShaderMinMax( mColorRampShaderMinEdit->value(), mColorRampShaderMaxEdit->value() );
      ret_symb = symb;
      break;
    }
    case QgsPointCloud3DSymbol::RenderingStyle::RGBRendering:
    {
      QgsRGBPointCloud3DSymbol *symb = new QgsRGBPointCloud3DSymbol( mLayer );
      symb->setPointSize( mPointSizeSpinBox->value() );
      ret_symb = symb;
      break;
    }
  }
  return ret_symb;
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
    case QgsPointCloud3DSymbol::RenderingStyle::NoRendering:
      mColorRampFrame->setVisible( false );
      mSingleColorFrame->setVisible( false );
      mPointSizeFrame->setVisible( false );
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::SingleColor:
      mColorRampFrame->setVisible( false );
      mSingleColorFrame->setVisible( true );
      mPointSizeFrame->setVisible( true );
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::ColorRamp:
      mColorRampFrame->setVisible( true );
      mSingleColorFrame->setVisible( false );
      mPointSizeFrame->setVisible( true );
      break;
    case QgsPointCloud3DSymbol::RenderingStyle::RGBRendering:
      mColorRampFrame->setVisible( false );
      mSingleColorFrame->setVisible( false );
      mPointSizeFrame->setVisible( true );
      break;
  }
}
