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
#include "qgsapplication.h"

QgsPointCloud3DSymbolWidget::QgsPointCloud3DSymbolWidget( QgsPointCloudLayer *layer, QgsPointCloud3DSymbol *symbol, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  setupUi( this );

  mPointSizeSpinBox->setClearValue( 2.0 );

  mRenderingParameterComboBox->setLayer( layer );

  mRenderingStyleComboBox->addItem( tr( "No Rendering" ), QgsPointCloud3DSymbol::NoRendering );
  mRenderingStyleComboBox->addItem( tr( "Single Color" ), QgsPointCloud3DSymbol::SingleColor );
  mRenderingStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "styleicons/singlebandpseudocolor.svg" ) ), tr( "Attribute by Ramp" ), QgsPointCloud3DSymbol::ColorRamp );
  mRenderingStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "styleicons/multibandcolor.svg" ) ), tr( "RGB" ), QgsPointCloud3DSymbol::RgbRendering );

  if ( symbol )
    setSymbol( symbol );

  connect( mPointSizeSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloud3DSymbolWidget::emitChangedSignal );
  connect( mRenderingStyleComboBox, qgis::overload< int >::of( &QComboBox::currentIndexChanged ), this, &QgsPointCloud3DSymbolWidget::onRenderingStyleChanged );
  connect( mColorRampShaderMinMaxReloadButton, &QPushButton::clicked, this, &QgsPointCloud3DSymbolWidget::reloadColorRampShaderMinMax );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsPointCloud3DSymbolWidget::emitChangedSignal );
}

void QgsPointCloud3DSymbolWidget::setSymbol( QgsPointCloud3DSymbol *symbol )
{
  mBlockChangedSignals++;
  mRenderingStyleComboBox->setCurrentIndex( mRenderingStyleComboBox->findData( symbol->renderingStyle() ) );
  if ( symbol )
  {
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
      case QgsPointCloud3DSymbol::RenderingStyle::RgbRendering:
      {
        QgsRgbPointCloud3DSymbol *symb = dynamic_cast<QgsRgbPointCloud3DSymbol *>( symbol );
        mPointSizeSpinBox->setValue( symb->pointSize() );
        break;
      }
    }
  }

  onRenderingStyleChanged();
  mBlockChangedSignals--;
}

QgsPointCloud3DSymbol *QgsPointCloud3DSymbolWidget::symbol() const
{
  QgsPointCloud3DSymbol *retSymb = nullptr;
  QgsPointCloud3DSymbol::RenderingStyle renderingStyle = static_cast< QgsPointCloud3DSymbol::RenderingStyle >( mRenderingStyleComboBox->currentData().toInt() );

  switch ( renderingStyle )
  {
    case QgsPointCloud3DSymbol::RenderingStyle::NoRendering:
    {
      retSymb = nullptr;
      break;
    }
    case QgsPointCloud3DSymbol::RenderingStyle::SingleColor:
    {
      QgsSingleColorPointCloud3DSymbol *symb = new QgsSingleColorPointCloud3DSymbol;
      symb->setPointSize( mPointSizeSpinBox->value() );
      symb->setSingleColor( mSingleColorBtn->color() );
      retSymb = symb;
      break;
    }
    case QgsPointCloud3DSymbol::RenderingStyle::ColorRamp:
    {
      QgsColorRampPointCloud3DSymbol *symb = new QgsColorRampPointCloud3DSymbol;
      symb->setRenderingParameter( mRenderingParameterComboBox->currentText() );
      symb->setPointSize( mPointSizeSpinBox->value() );
      symb->setColorRampShader( mColorRampShaderWidget->shader() );
      symb->setColorRampShaderMinMax( mColorRampShaderMinEdit->value(), mColorRampShaderMaxEdit->value() );
      retSymb = symb;
      break;
    }
    case QgsPointCloud3DSymbol::RenderingStyle::RgbRendering:
    {
      QgsRgbPointCloud3DSymbol *symb = new QgsRgbPointCloud3DSymbol;
      symb->setPointSize( mPointSizeSpinBox->value() );
      retSymb = symb;
      break;
    }
  }
  return retSymb;
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

void QgsPointCloud3DSymbolWidget::onRenderingStyleChanged()
{
  const QgsPointCloud3DSymbol::RenderingStyle currentStyle = static_cast< QgsPointCloud3DSymbol::RenderingStyle>( mRenderingStyleComboBox->currentData().toInt() );
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
    case QgsPointCloud3DSymbol::RenderingStyle::RgbRendering:
      mColorRampFrame->setVisible( false );
      mSingleColorFrame->setVisible( false );
      mPointSizeFrame->setVisible( true );
      break;
  }
}

void QgsPointCloud3DSymbolWidget::emitChangedSignal()
{
  if ( mBlockChangedSignals )
    return;

  emit changed();
}
