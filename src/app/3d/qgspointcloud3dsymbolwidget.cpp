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
#include "qgspointcloudrenderer.h"
#include "qgspointcloudattributebyramprenderer.h"
#include "qgspointcloudrgbrenderer.h"

QgsPointCloud3DSymbolWidget::QgsPointCloud3DSymbolWidget( QgsPointCloudLayer *layer, QgsPointCloud3DSymbol *symbol, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  setupUi( this );

  mPointSizeSpinBox->setClearValue( 2.0 );

  mRenderingParameterComboBox->setLayer( layer );
  mRenderingParameterComboBox->setFilters( QgsPointCloudAttributeProxyModel::Numeric );
  mRenderingParameterComboBox->setAllowEmptyAttributeName( false );

  mSingleColorBtn->setAllowOpacity( false );
  mSingleColorBtn->setColorDialogTitle( tr( "Select Point Color" ) );
  mSingleColorBtn->setColor( QColor( 0, 0, 255 ) ); // default color

  mRenderingStyleComboBox->addItem( tr( "No Rendering" ), QString() );
  mRenderingStyleComboBox->addItem( tr( "Single Color" ), QStringLiteral( "single-color" ) );
  mRenderingStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "styleicons/singlebandpseudocolor.svg" ) ), tr( "Attribute by Ramp" ), QStringLiteral( "color-ramp" ) );
  mRenderingStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "styleicons/multibandcolor.svg" ) ), tr( "RGB" ), QStringLiteral( "rgb" ) );

  mRenderingStyleComboBox->setCurrentIndex( 0 );
  mStackedWidget->setCurrentIndex( 0 );

  if ( symbol )
    setSymbol( symbol );

  connect( mPointSizeSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloud3DSymbolWidget::emitChangedSignal );
  connect( mRenderingStyleComboBox, qgis::overload< int >::of( &QComboBox::currentIndexChanged ), this, &QgsPointCloud3DSymbolWidget::onRenderingStyleChanged );
  connect( mScalarRecalculateMinMaxButton, &QPushButton::clicked, this, &QgsPointCloud3DSymbolWidget::setMinMaxFromLayer );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsPointCloud3DSymbolWidget::emitChangedSignal );
  connect( mSingleColorBtn, &QgsColorButton::colorChanged, this, &QgsPointCloud3DSymbolWidget::emitChangedSignal );
  connect( mRenderingParameterComboBox, &QgsPointCloudAttributeComboBox::attributeChanged,
           this, &QgsPointCloud3DSymbolWidget::rampAttributeChanged );
  connect( mColorRampShaderMinEdit, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloud3DSymbolWidget::minMaxChanged );
  connect( mColorRampShaderMaxEdit, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloud3DSymbolWidget::minMaxChanged );

  rampAttributeChanged();
}

void QgsPointCloud3DSymbolWidget::setSymbol( QgsPointCloud3DSymbol *symbol )
{
  mBlockChangedSignals++;
  if ( !symbol )
  {
    mRenderingStyleComboBox->setCurrentIndex( 0 );
    mStackedWidget->setCurrentIndex( 0 );
    mBlockChangedSignals--;
    return;
  }

  mRenderingStyleComboBox->setCurrentIndex( mRenderingStyleComboBox->findData( symbol->symbolType() ) );
  mPointSizeSpinBox->setValue( symbol->pointSize() );

  if ( symbol->symbolType() == QLatin1String( "single-color" ) )
  {
    mStackedWidget->setCurrentIndex( 1 );
    QgsSingleColorPointCloud3DSymbol *symb = dynamic_cast<QgsSingleColorPointCloud3DSymbol *>( symbol );
    mSingleColorBtn->setColor( symb->singleColor() );
  }
  else if ( symbol->symbolType() == QLatin1String( "color-ramp" ) )
  {
    mStackedWidget->setCurrentIndex( 2 );
    QgsColorRampPointCloud3DSymbol *symb = dynamic_cast<QgsColorRampPointCloud3DSymbol *>( symbol );

    mRenderingParameterComboBox->setAttribute( symb->renderingParameter() );

    mColorRampShaderMinEdit->setValue( symb->colorRampShaderMin() );
    mColorRampShaderMaxEdit->setValue( symb->colorRampShaderMax() );

    whileBlocking( mColorRampShaderWidget )->setFromShader( symb->colorRampShader() );
    whileBlocking( mColorRampShaderWidget )->setMinimumMaximum( symb->colorRampShaderMin(), symb->colorRampShaderMax() );
  }
  else if ( symbol->symbolType() == QLatin1String( "rgb" ) )
  {
    mStackedWidget->setCurrentIndex( 3 );
  }
  else
  {
    mStackedWidget->setCurrentIndex( 0 );
  }

  mBlockChangedSignals--;
}

QgsPointCloud3DSymbol *QgsPointCloud3DSymbolWidget::symbol() const
{
  QgsPointCloud3DSymbol *retSymb = nullptr;
  const QString symbolType = mRenderingStyleComboBox->currentData().toString();

  if ( symbolType == QLatin1String( "single-color" ) )
  {
    QgsSingleColorPointCloud3DSymbol *symb = new QgsSingleColorPointCloud3DSymbol;
    symb->setPointSize( mPointSizeSpinBox->value() );
    symb->setSingleColor( mSingleColorBtn->color() );
    retSymb = symb;
  }
  else if ( symbolType == QLatin1String( "color-ramp" ) )
  {
    QgsColorRampPointCloud3DSymbol *symb = new QgsColorRampPointCloud3DSymbol;
    symb->setRenderingParameter( mRenderingParameterComboBox->currentText() );
    symb->setPointSize( mPointSizeSpinBox->value() );
    symb->setColorRampShader( mColorRampShaderWidget->shader() );
    symb->setColorRampShaderMinMax( mColorRampShaderMinEdit->value(), mColorRampShaderMaxEdit->value() );
    retSymb = symb;
  }
  else if ( symbolType == QLatin1String( "rgb" ) )
  {
    QgsRgbPointCloud3DSymbol *symb = new QgsRgbPointCloud3DSymbol;
    symb->setPointSize( mPointSizeSpinBox->value() );
    retSymb = symb;
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
  if ( mBlockChangedSignals )
    return;

  mStackedWidget->setCurrentIndex( mRenderingStyleComboBox->currentIndex() );

  // copy settings from 2d renderer, if possible!
  if ( mLayer )
  {
    const QString newSymbolType = mRenderingStyleComboBox->currentData().toString();
    if ( newSymbolType == QLatin1String( "color-ramp" ) && mLayer->renderer()->type() == QLatin1String( "ramp" ) )
    {
      const QgsPointCloudAttributeByRampRenderer *renderer2d = dynamic_cast< const QgsPointCloudAttributeByRampRenderer * >( mLayer->renderer() );
      mBlockChangedSignals++;
      mRenderingParameterComboBox->setAttribute( renderer2d->attribute() );
      mColorRampShaderMinEdit->setValue( renderer2d->minimum() );
      mColorRampShaderMaxEdit->setValue( renderer2d->maximum() );
      whileBlocking( mColorRampShaderWidget )->setFromShader( renderer2d->colorRampShader() );
      whileBlocking( mColorRampShaderWidget )->setMinimumMaximum( renderer2d->minimum(), renderer2d->maximum() );
      mBlockChangedSignals--;
    }
    else if ( newSymbolType == QLatin1String( "rgb" ) )
    {
      const QgsPointCloudRgbRenderer *renderer2d = dynamic_cast< const QgsPointCloudRgbRenderer * >( mLayer->renderer() );
      mBlockChangedSignals++;
      // todo
      ( void )( renderer2d );
      mBlockChangedSignals--;
    }
  }

  emitChangedSignal();
}

void QgsPointCloud3DSymbolWidget::emitChangedSignal()
{
  if ( mBlockChangedSignals )
    return;

  emit changed();
}

void QgsPointCloud3DSymbolWidget::rampAttributeChanged()
{
  if ( mLayer && mLayer->dataProvider() )
  {
    const QVariant min = mLayer->dataProvider()->metadataStatistic( mRenderingParameterComboBox->currentAttribute(), QgsStatisticalSummary::Min );
    const QVariant max = mLayer->dataProvider()->metadataStatistic( mRenderingParameterComboBox->currentAttribute(), QgsStatisticalSummary::Max );
    if ( min.isValid() && max.isValid() )
    {
      mProviderMin = min.toDouble();
      mProviderMax = max.toDouble();
    }
    else
    {
      mProviderMin = std::numeric_limits< double >::quiet_NaN();
      mProviderMax = std::numeric_limits< double >::quiet_NaN();
    }
  }
  mScalarRecalculateMinMaxButton->setEnabled( !std::isnan( mProviderMin ) && !std::isnan( mProviderMax ) );
  emitChangedSignal();
}

void QgsPointCloud3DSymbolWidget::setMinMaxFromLayer()
{
  if ( std::isnan( mProviderMin ) || std::isnan( mProviderMax ) )
    return;

  mBlockMinMaxChanged = true;
  mColorRampShaderMinEdit->setValue( mProviderMin );
  mColorRampShaderMaxEdit->setValue( mProviderMax );
  mBlockMinMaxChanged = false;

  minMaxChanged();
}

void QgsPointCloud3DSymbolWidget::minMaxChanged()
{
  if ( mBlockMinMaxChanged )
    return;

  mColorRampShaderWidget->setMinimumMaximumAndClassify( mColorRampShaderMinEdit->value(), mColorRampShaderMaxEdit->value() );
}
