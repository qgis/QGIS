/***************************************************************************
                         qgspointcloudrgbrendererwidget.cpp
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudrgbrendererwidget.h"
#include "qgscontrastenhancement.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudrgbrenderer.h"
#include "qgsdoublevalidator.h"

///@cond PRIVATE

QgsPointCloudRgbRendererWidget::QgsPointCloudRgbRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style )
  : QgsPointCloudRendererWidget( layer, style )
{
  setupUi( this );
  connect( mRedMinLineEdit, &QLineEdit::textChanged, this, &QgsPointCloudRgbRendererWidget::mRedMinLineEdit_textChanged );
  connect( mRedMaxLineEdit, &QLineEdit::textChanged, this, &QgsPointCloudRgbRendererWidget::mRedMaxLineEdit_textChanged );
  connect( mGreenMinLineEdit, &QLineEdit::textChanged, this, &QgsPointCloudRgbRendererWidget::mGreenMinLineEdit_textChanged );
  connect( mGreenMaxLineEdit, &QLineEdit::textChanged, this, &QgsPointCloudRgbRendererWidget::mGreenMaxLineEdit_textChanged );
  connect( mBlueMinLineEdit, &QLineEdit::textChanged, this, &QgsPointCloudRgbRendererWidget::mBlueMinLineEdit_textChanged );
  connect( mBlueMaxLineEdit, &QLineEdit::textChanged, this, &QgsPointCloudRgbRendererWidget::mBlueMaxLineEdit_textChanged );
  createValidators();

  mRedAttributeComboBox->setAllowEmptyAttributeName( true );
  mGreenAttributeComboBox->setAllowEmptyAttributeName( true );
  mBlueAttributeComboBox->setAllowEmptyAttributeName( true );

  //contrast enhancement algorithms
  mContrastEnhancementAlgorithmComboBox->addItem( tr( "No Enhancement" ), QgsContrastEnhancement::NoEnhancement );
  mContrastEnhancementAlgorithmComboBox->addItem( tr( "Stretch to MinMax" ), QgsContrastEnhancement::StretchToMinimumMaximum );
  mContrastEnhancementAlgorithmComboBox->addItem( tr( "Stretch and Clip to MinMax" ), QgsContrastEnhancement::StretchAndClipToMinimumMaximum );
  mContrastEnhancementAlgorithmComboBox->addItem( tr( "Clip to MinMax" ), QgsContrastEnhancement::ClipToMinimumMaximum );

  if ( layer )
  {
    mRedAttributeComboBox->setLayer( layer );
    mGreenAttributeComboBox->setLayer( layer );
    mBlueAttributeComboBox->setLayer( layer );

    setFromRenderer( layer->renderer() );
  }

  connect( mRedAttributeComboBox, &QgsPointCloudAttributeComboBox::attributeChanged,
           this, &QgsPointCloudRgbRendererWidget::redAttributeChanged );
  connect( mGreenAttributeComboBox, &QgsPointCloudAttributeComboBox::attributeChanged,
           this, &QgsPointCloudRgbRendererWidget::greenAttributeChanged );
  connect( mBlueAttributeComboBox, &QgsPointCloudAttributeComboBox::attributeChanged,
           this, &QgsPointCloudRgbRendererWidget::blueAttributeChanged );
  connect( mContrastEnhancementAlgorithmComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloudRgbRendererWidget::emitWidgetChanged );

  if ( layer )
  {
    // set nice initial values
    redAttributeChanged();
    greenAttributeChanged();
    blueAttributeChanged();
  }
}

QgsPointCloudRendererWidget *QgsPointCloudRgbRendererWidget::create( QgsPointCloudLayer *layer, QgsStyle *style, QgsPointCloudRenderer * )
{
  return new QgsPointCloudRgbRendererWidget( layer, style );
}

QgsPointCloudRenderer *QgsPointCloudRgbRendererWidget::renderer()
{
  if ( !mLayer )
  {
    return nullptr;
  }

  std::unique_ptr< QgsPointCloudRgbRenderer > renderer = std::make_unique< QgsPointCloudRgbRenderer >();
  renderer->setRedAttribute( mRedAttributeComboBox->currentAttribute() );
  renderer->setGreenAttribute( mGreenAttributeComboBox->currentAttribute() );
  renderer->setBlueAttribute( mBlueAttributeComboBox->currentAttribute() );

  setCustomMinMaxValues( renderer.get() );
  return renderer.release();
}

void QgsPointCloudRgbRendererWidget::createValidators()
{
  mRedMinLineEdit->setValidator( new QgsDoubleValidator( mRedMinLineEdit ) );
  mRedMaxLineEdit->setValidator( new QgsDoubleValidator( mRedMinLineEdit ) );
  mGreenMinLineEdit->setValidator( new QgsDoubleValidator( mGreenMinLineEdit ) );
  mGreenMaxLineEdit->setValidator( new QgsDoubleValidator( mGreenMinLineEdit ) );
  mBlueMinLineEdit->setValidator( new QgsDoubleValidator( mBlueMinLineEdit ) );
  mBlueMaxLineEdit->setValidator( new QgsDoubleValidator( mBlueMinLineEdit ) );
}

void QgsPointCloudRgbRendererWidget::setCustomMinMaxValues( QgsPointCloudRgbRenderer *r )
{
  if ( !r )
  {
    return;
  }

  if ( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ==
       QgsContrastEnhancement::NoEnhancement )
  {
    r->setRedContrastEnhancement( nullptr );
    r->setGreenContrastEnhancement( nullptr );
    r->setBlueContrastEnhancement( nullptr );
    return;
  }

  QgsContrastEnhancement *redEnhancement = nullptr;
  QgsContrastEnhancement *greenEnhancement = nullptr;
  QgsContrastEnhancement *blueEnhancement = nullptr;

  bool redMinOk, redMaxOk;
  const double redMin = QgsDoubleValidator::toDouble( mRedMinLineEdit->text(), &redMinOk );
  const double redMax = QgsDoubleValidator::toDouble( mRedMaxLineEdit->text(), &redMaxOk );
  if ( redMinOk && redMaxOk && !mRedAttributeComboBox->currentAttribute().isEmpty() )
  {
    redEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    redEnhancement->setMinimumValue( redMin );
    redEnhancement->setMaximumValue( redMax );
  }

  bool greenMinOk, greenMaxOk;
  const double greenMin = QgsDoubleValidator::toDouble( mGreenMinLineEdit->text(), &greenMinOk );
  const double greenMax = QgsDoubleValidator::toDouble( mGreenMaxLineEdit->text(), &greenMaxOk );
  if ( greenMinOk && greenMaxOk && !mGreenAttributeComboBox->currentAttribute().isEmpty() )
  {
    greenEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    greenEnhancement->setMinimumValue( greenMin );
    greenEnhancement->setMaximumValue( greenMax );
  }

  bool blueMinOk, blueMaxOk;
  const double blueMin = QgsDoubleValidator::toDouble( mBlueMinLineEdit->text(), &blueMinOk );
  const double blueMax = QgsDoubleValidator::toDouble( mBlueMaxLineEdit->text(), &blueMaxOk );
  if ( blueMinOk && blueMaxOk && !mBlueAttributeComboBox->currentAttribute().isEmpty() )
  {
    blueEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    blueEnhancement->setMinimumValue( blueMin );
    blueEnhancement->setMaximumValue( blueMax );
  }

  if ( redEnhancement )
  {
    redEnhancement->setContrastEnhancementAlgorithm( static_cast< QgsContrastEnhancement::ContrastEnhancementAlgorithm >(
          ( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ) ) );
  }
  if ( greenEnhancement )
  {
    greenEnhancement->setContrastEnhancementAlgorithm( static_cast< QgsContrastEnhancement::ContrastEnhancementAlgorithm >(
          ( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ) ) );
  }
  if ( blueEnhancement )
  {
    blueEnhancement->setContrastEnhancementAlgorithm( static_cast< QgsContrastEnhancement::ContrastEnhancementAlgorithm >(
          ( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ) ) );
  }
  r->setRedContrastEnhancement( redEnhancement );
  r->setGreenContrastEnhancement( greenEnhancement );
  r->setBlueContrastEnhancement( blueEnhancement );
}

void QgsPointCloudRgbRendererWidget::mRedMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloudRgbRendererWidget::mRedMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloudRgbRendererWidget::mGreenMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloudRgbRendererWidget::mGreenMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloudRgbRendererWidget::mBlueMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloudRgbRendererWidget::mBlueMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloudRgbRendererWidget::emitWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit widgetChanged();
}

void QgsPointCloudRgbRendererWidget::redAttributeChanged()
{
  if ( mLayer && mLayer->dataProvider() )
  {
    const QVariant max = mLayer->dataProvider()->metadataStatistic( mRedAttributeComboBox->currentAttribute(), QgsStatisticalSummary::Max );
    if ( max.isValid() )
    {
      const int maxValue = max.toInt();
      mDisableMinMaxWidgetRefresh++;
      mRedMinLineEdit->setText( QLocale().toString( 0 ) );

      // try and guess suitable range from input max values -- we don't just take the provider max value directly here, but rather see if it's
      // likely to be 8 bit or 16 bit color values
      mRedMaxLineEdit->setText( QLocale().toString( maxValue > 255 ? 65535 : 255 ) );
      mDisableMinMaxWidgetRefresh--;
      emitWidgetChanged();
    }
  }
}

void QgsPointCloudRgbRendererWidget::greenAttributeChanged()
{
  if ( mLayer && mLayer->dataProvider() )
  {
    const QVariant max = mLayer->dataProvider()->metadataStatistic( mGreenAttributeComboBox->currentAttribute(), QgsStatisticalSummary::Max );
    if ( max.isValid() )
    {
      const int maxValue = max.toInt();
      mDisableMinMaxWidgetRefresh++;
      mGreenMinLineEdit->setText( QLocale().toString( 0 ) );

      // try and guess suitable range from input max values -- we don't just take the provider max value directly here, but rather see if it's
      // likely to be 8 bit or 16 bit color values
      mGreenMaxLineEdit->setText( QLocale().toString( maxValue > 255 ? 65535 : 255 ) );
      mDisableMinMaxWidgetRefresh--;
      emitWidgetChanged();
    }
  }
}

void QgsPointCloudRgbRendererWidget::blueAttributeChanged()
{
  if ( mLayer && mLayer->dataProvider() )
  {
    const QVariant max = mLayer->dataProvider()->metadataStatistic( mBlueAttributeComboBox->currentAttribute(), QgsStatisticalSummary::Max );
    if ( max.isValid() )
    {
      const int maxValue = max.toInt();
      mDisableMinMaxWidgetRefresh++;
      mBlueMinLineEdit->setText( QLocale().toString( 0 ) );

      // try and guess suitable range from input max values -- we don't just take the provider max value directly here, but rather see if it's
      // likely to be 8 bit or 16 bit color values
      mBlueMaxLineEdit->setText( QLocale().toString( maxValue > 255 ? 65535 : 255 ) );
      mDisableMinMaxWidgetRefresh--;
      emitWidgetChanged();
    }
  }
}

void QgsPointCloudRgbRendererWidget::minMaxModified()
{
  if ( !mDisableMinMaxWidgetRefresh )
  {
    if ( ( QgsContrastEnhancement::ContrastEnhancementAlgorithm )( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ) == QgsContrastEnhancement::NoEnhancement )
    {
      mContrastEnhancementAlgorithmComboBox->setCurrentIndex(
        mContrastEnhancementAlgorithmComboBox->findData( ( int ) QgsContrastEnhancement::StretchToMinimumMaximum ) );
    }
    emitWidgetChanged();
  }
}

void QgsPointCloudRgbRendererWidget::setMinMaxValue( const QgsContrastEnhancement *ce, QLineEdit *minEdit, QLineEdit *maxEdit )
{
  if ( !minEdit || !maxEdit )
  {
    return;
  }

  if ( !ce )
  {
    minEdit->clear();
    maxEdit->clear();
    return;
  }

  minEdit->setText( QLocale().toString( ce->minimumValue() ) );
  maxEdit->setText( QLocale().toString( ce->maximumValue() ) );

  // QgsMultiBandColorRenderer is using individual contrast enhancements for each
  // band, but this widget GUI has one for all
  mContrastEnhancementAlgorithmComboBox->setCurrentIndex( mContrastEnhancementAlgorithmComboBox->findData(
        static_cast< int >( ce->contrastEnhancementAlgorithm() ) ) );
}

void QgsPointCloudRgbRendererWidget::setFromRenderer( const QgsPointCloudRenderer *r )
{
  mBlockChangedSignal = true;
  const QgsPointCloudRgbRenderer *mbcr = dynamic_cast<const QgsPointCloudRgbRenderer *>( r );
  if ( mbcr )
  {
    mRedAttributeComboBox->setAttribute( mbcr->redAttribute() );
    mGreenAttributeComboBox->setAttribute( mbcr->greenAttribute() );
    mBlueAttributeComboBox->setAttribute( mbcr->blueAttribute() );

    mDisableMinMaxWidgetRefresh++;
    setMinMaxValue( mbcr->redContrastEnhancement(), mRedMinLineEdit, mRedMaxLineEdit );
    setMinMaxValue( mbcr->greenContrastEnhancement(), mGreenMinLineEdit, mGreenMaxLineEdit );
    setMinMaxValue( mbcr->blueContrastEnhancement(), mBlueMinLineEdit, mBlueMaxLineEdit );
    mDisableMinMaxWidgetRefresh--;
  }
  else
  {
    if ( mRedAttributeComboBox->findText( QStringLiteral( "Red" ) ) > -1 && mRedAttributeComboBox->findText( QStringLiteral( "Green" ) ) > -1 &&
         mRedAttributeComboBox->findText( QStringLiteral( "Blue" ) ) > -1 )
    {
      mRedAttributeComboBox->setAttribute( QStringLiteral( "Red" ) );
      mGreenAttributeComboBox->setAttribute( QStringLiteral( "Green" ) );
      mBlueAttributeComboBox->setAttribute( QStringLiteral( "Blue" ) );
    }
    else
    {
      mRedAttributeComboBox->setCurrentIndex( mRedAttributeComboBox->count() > 1 ? 1 : 0 );
      mGreenAttributeComboBox->setCurrentIndex( mGreenAttributeComboBox->count() > 2 ? 2 : 0 );
      mBlueAttributeComboBox->setCurrentIndex( mBlueAttributeComboBox->count() > 3 ? 3 : 0 );
    }
  }
  mBlockChangedSignal = false;
}

///@endcond
