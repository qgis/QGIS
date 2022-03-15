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
#include "qgspointcloudclassifiedrenderer.h"
#include "qgsdoublevalidator.h"
#include "qgspointcloudclassifiedrendererwidget.h"
#include "qgspointcloudlayerelevationproperties.h"

QgsPointCloud3DSymbolWidget::QgsPointCloud3DSymbolWidget( QgsPointCloudLayer *layer, QgsPointCloud3DSymbol *symbol, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  setupUi( this );

  mPointSizeSpinBox->setClearValue( 2.0 );
  mMaxScreenErrorSpinBox->setClearValue( 1.0 );

  mColorRampShaderMinEdit->setShowClearButton( false );
  mColorRampShaderMaxEdit->setShowClearButton( false );

  mRenderingParameterComboBox->setLayer( layer );
  mRenderingParameterComboBox->setFilters( QgsPointCloudAttributeProxyModel::AllTypes );
  mRenderingParameterComboBox->setAllowEmptyAttributeName( false );

  mSingleColorBtn->setAllowOpacity( false );
  mSingleColorBtn->setColorDialogTitle( tr( "Select Point Color" ) );
  mSingleColorBtn->setColor( QColor( 0, 0, 255 ) ); // default color

  mRenderingStyleComboBox->addItem( tr( "No Rendering" ), QString() );
  mRenderingStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "styleicons/singlecolor.svg" ) ), tr( "Single Color" ), QStringLiteral( "single-color" ) );
  mRenderingStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "styleicons/singlebandpseudocolor.svg" ) ), tr( "Attribute by Ramp" ), QStringLiteral( "color-ramp" ) );
  mRenderingStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "styleicons/multibandcolor.svg" ) ), tr( "RGB" ), QStringLiteral( "rgb" ) );
  mRenderingStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "styleicons/paletted.svg" ) ), tr( "Classification" ), QStringLiteral( "classification" ) );

  connect( mRedMinLineEdit, &QLineEdit::textChanged, this, &QgsPointCloud3DSymbolWidget::mRedMinLineEdit_textChanged );
  connect( mRedMaxLineEdit, &QLineEdit::textChanged, this, &QgsPointCloud3DSymbolWidget::mRedMaxLineEdit_textChanged );
  connect( mGreenMinLineEdit, &QLineEdit::textChanged, this, &QgsPointCloud3DSymbolWidget::mGreenMinLineEdit_textChanged );
  connect( mGreenMaxLineEdit, &QLineEdit::textChanged, this, &QgsPointCloud3DSymbolWidget::mGreenMaxLineEdit_textChanged );
  connect( mBlueMinLineEdit, &QLineEdit::textChanged, this, &QgsPointCloud3DSymbolWidget::mBlueMinLineEdit_textChanged );
  connect( mBlueMaxLineEdit, &QLineEdit::textChanged, this, &QgsPointCloud3DSymbolWidget::mBlueMaxLineEdit_textChanged );
  createValidators();

  mRedAttributeComboBox->setAllowEmptyAttributeName( true );
  mGreenAttributeComboBox->setAllowEmptyAttributeName( true );
  mBlueAttributeComboBox->setAllowEmptyAttributeName( true );

  //contrast enhancement algorithms
  mContrastEnhancementAlgorithmComboBox->addItem( tr( "No Enhancement" ), QgsContrastEnhancement::NoEnhancement );
  mContrastEnhancementAlgorithmComboBox->addItem( tr( "Stretch to MinMax" ), QgsContrastEnhancement::StretchToMinimumMaximum );
  mContrastEnhancementAlgorithmComboBox->addItem( tr( "Stretch and Clip to MinMax" ), QgsContrastEnhancement::StretchAndClipToMinimumMaximum );
  mContrastEnhancementAlgorithmComboBox->addItem( tr( "Clip to MinMax" ), QgsContrastEnhancement::ClipToMinimumMaximum );

  mRedAttributeComboBox->setLayer( layer );
  mGreenAttributeComboBox->setLayer( layer );
  mBlueAttributeComboBox->setLayer( layer );

  connect( mRedAttributeComboBox, &QgsPointCloudAttributeComboBox::attributeChanged, this, &QgsPointCloud3DSymbolWidget::redAttributeChanged );
  connect( mGreenAttributeComboBox, &QgsPointCloudAttributeComboBox::attributeChanged, this, &QgsPointCloud3DSymbolWidget::greenAttributeChanged );
  connect( mBlueAttributeComboBox, &QgsPointCloudAttributeComboBox::attributeChanged, this, &QgsPointCloud3DSymbolWidget::blueAttributeChanged );
  connect( mContrastEnhancementAlgorithmComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPointCloud3DSymbolWidget::emitChangedSignal );

  // set nice initial values
  redAttributeChanged();
  greenAttributeChanged();
  blueAttributeChanged();

  mRenderingStyleComboBox->setCurrentIndex( 0 );
  mStackedWidget->setCurrentIndex( 0 );

  whileBlocking( mPointBudgetSpinBox )->setMinimum( std::min( mLayer->pointCount() / 2, ( qint64 )100000 ) );
  whileBlocking( mPointBudgetSpinBox )->setMaximum( mLayer->pointCount() + 1 );
  whileBlocking( mPointBudgetSpinBox )->setValue( 1000000 );

  if ( symbol )
    setSymbol( symbol );

  connect( mPointSizeSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloud3DSymbolWidget::emitChangedSignal );
  connect( mRenderingStyleComboBox, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsPointCloud3DSymbolWidget::onRenderingStyleChanged );
  connect( mScalarRecalculateMinMaxButton, &QPushButton::clicked, this, &QgsPointCloud3DSymbolWidget::setMinMaxFromLayer );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsPointCloud3DSymbolWidget::emitChangedSignal );
  connect( mSingleColorBtn, &QgsColorButton::colorChanged, this, &QgsPointCloud3DSymbolWidget::emitChangedSignal );
  connect( mRenderingParameterComboBox, &QgsPointCloudAttributeComboBox::attributeChanged, this, &QgsPointCloud3DSymbolWidget::rampAttributeChanged );
  connect( mColorRampShaderMinEdit, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloud3DSymbolWidget::minMaxChanged );
  connect( mColorRampShaderMaxEdit, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsPointCloud3DSymbolWidget::minMaxChanged );

  connect( mMaxScreenErrorSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [&]() { emitChangedSignal(); } );
  connect( mShowBoundingBoxesCheckBox, &QCheckBox::stateChanged, this, [&]() { emitChangedSignal(); } );
  connect( mPointBudgetSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [&]() { emitChangedSignal(); } );

  connect( mTriangulateGroupBox, &QGroupBox::toggled, this, [&]() { emitChangedSignal(); } );
  connect( mTriangulateGroupBox, &QGroupBox::toggled, this, [&]() {mPointSizeSpinBox->setEnabled( !mTriangulateGroupBox->isChecked() ); } );

  connect( mHorizontalTriangleCheckBox, &QCheckBox::stateChanged, this, [&]() { emitChangedSignal(); } );
  connect( mHorizontalTriangleCheckBox, &QCheckBox::stateChanged, this, [&]()
  { mHorizontalTriangleThresholdSpinBox->setEnabled( mHorizontalTriangleCheckBox->isChecked() ); } );
  connect( mHorizontalTriangleThresholdSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [&]() { emitChangedSignal(); } );

  connect( mVerticalTriangleCheckBox, &QCheckBox::stateChanged, this, [&]() { emitChangedSignal(); } );
  connect( mVerticalTriangleCheckBox, &QCheckBox::stateChanged, this, [&]()
  { mVerticalTriangleThresholdSpinBox->setEnabled( mVerticalTriangleCheckBox->isChecked() ); } );
  connect( mVerticalTriangleThresholdSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [&]() { emitChangedSignal(); } );

  mPointSizeSpinBox->setEnabled( !mTriangulateGroupBox->isChecked() );
  mHorizontalTriangleThresholdSpinBox->setEnabled( mHorizontalTriangleCheckBox->isChecked() );
  mVerticalTriangleThresholdSpinBox->setEnabled( mVerticalTriangleCheckBox->isChecked() );

  if ( !symbol ) // if we have a symbol, this was already handled in setSymbol above
    rampAttributeChanged();

  mClassifiedRendererWidget = new QgsPointCloudClassifiedRendererWidget( layer, nullptr );
  mClassifiedRendererWidget->setParent( this );
  mClassifiedRenderingLayout->addWidget( mClassifiedRendererWidget );

  connect( mClassifiedRendererWidget, &QgsPointCloudClassifiedRendererWidget::widgetChanged, this, &QgsPointCloud3DSymbolWidget::emitChangedSignal );
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
  mTriangulateGroupBox->setChecked( symbol->renderAsTriangles() );
  mHorizontalTriangleCheckBox->setChecked( symbol->horizontalTriangleFilter() );
  mHorizontalTriangleThresholdSpinBox->setValue( symbol->horizontalFilterThreshold() );
  mVerticalTriangleCheckBox->setChecked( symbol->verticalTriangleFilter() );
  mVerticalTriangleThresholdSpinBox->setValue( symbol->verticalFilterThreshold() );

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

    // we will be restoring the existing ramp classes -- we don't want to regenerate any automatically!
    mBlockSetMinMaxFromLayer = true;
    mRenderingParameterComboBox->setAttribute( symb->attribute() );

    mColorRampShaderMinEdit->setValue( symb->colorRampShaderMin() );
    mColorRampShaderMaxEdit->setValue( symb->colorRampShaderMax() );

    whileBlocking( mColorRampShaderWidget )->setFromShader( symb->colorRampShader() );
    whileBlocking( mColorRampShaderWidget )->setMinimumMaximum( symb->colorRampShaderMin(), symb->colorRampShaderMax() );
    mBlockSetMinMaxFromLayer = false;
  }
  else if ( symbol->symbolType() == QLatin1String( "rgb" ) )
  {
    mStackedWidget->setCurrentIndex( 3 );

    QgsRgbPointCloud3DSymbol *symb = dynamic_cast<QgsRgbPointCloud3DSymbol *>( symbol );
    mRedAttributeComboBox->setAttribute( symb->redAttribute() );
    mGreenAttributeComboBox->setAttribute( symb->greenAttribute() );
    mBlueAttributeComboBox->setAttribute( symb->blueAttribute() );

    mDisableMinMaxWidgetRefresh++;
    setMinMaxValue( symb->redContrastEnhancement(), mRedMinLineEdit, mRedMaxLineEdit );
    setMinMaxValue( symb->greenContrastEnhancement(), mGreenMinLineEdit, mGreenMaxLineEdit );
    setMinMaxValue( symb->blueContrastEnhancement(), mBlueMinLineEdit, mBlueMaxLineEdit );
    mDisableMinMaxWidgetRefresh--;
  }
  else if ( symbol->symbolType() == QLatin1String( "classification" ) )
  {
    mStackedWidget->setCurrentIndex( 4 );
    QgsClassificationPointCloud3DSymbol *symb = dynamic_cast<QgsClassificationPointCloud3DSymbol *>( symbol );
    mClassifiedRendererWidget->setFromCategories( symb->categoriesList(), symb->attribute() );
  }
  else
  {
    mStackedWidget->setCurrentIndex( 0 );
  }

  mBlockChangedSignals--;
}

void QgsPointCloud3DSymbolWidget::setDockMode( bool dockMode )
{
  if ( mClassifiedRendererWidget )
    mClassifiedRendererWidget->setDockMode( dockMode );
}

QgsPointCloud3DSymbol *QgsPointCloud3DSymbolWidget::symbol() const
{
  QgsPointCloud3DSymbol *retSymb = nullptr;
  const QString symbolType = mRenderingStyleComboBox->currentData().toString();

  if ( symbolType == QLatin1String( "single-color" ) )
  {
    QgsSingleColorPointCloud3DSymbol *symb = new QgsSingleColorPointCloud3DSymbol;
    symb->setSingleColor( mSingleColorBtn->color() );
    retSymb = symb;
  }
  else if ( symbolType == QLatin1String( "color-ramp" ) )
  {
    QgsColorRampPointCloud3DSymbol *symb = new QgsColorRampPointCloud3DSymbol;
    symb->setAttribute( mRenderingParameterComboBox->currentText() );
    symb->setColorRampShader( mColorRampShaderWidget->shader() );
    symb->setColorRampShaderMinMax( mColorRampShaderMinEdit->value(), mColorRampShaderMaxEdit->value() );
    retSymb = symb;
  }
  else if ( symbolType == QLatin1String( "rgb" ) )
  {
    QgsRgbPointCloud3DSymbol *symb = new QgsRgbPointCloud3DSymbol;
    symb->setRedAttribute( mRedAttributeComboBox->currentAttribute() );
    symb->setGreenAttribute( mGreenAttributeComboBox->currentAttribute() );
    symb->setBlueAttribute( mBlueAttributeComboBox->currentAttribute() );
    setCustomMinMaxValues( symb );
    retSymb = symb;
  }
  else if ( symbolType == QLatin1String( "classification" ) )
  {
    QgsClassificationPointCloud3DSymbol *symb = new QgsClassificationPointCloud3DSymbol;
    symb->setAttribute( mClassifiedRendererWidget->attribute() );
    symb->setCategoriesList( mClassifiedRendererWidget->categoriesList() );
    retSymb = symb;
  }

  if ( retSymb )
  {
    retSymb->setPointSize( mPointSizeSpinBox->value() );
    retSymb->setRenderAsTriangles( mTriangulateGroupBox->isChecked() );
    retSymb->setHorizontalTriangleFilter( mHorizontalTriangleCheckBox->isChecked() );
    retSymb->setHorizontalFilterThreshold( mHorizontalTriangleThresholdSpinBox->value() );
    retSymb->setVerticalTriangleFilter( mVerticalTriangleCheckBox->isChecked() );
    retSymb->setVerticalFilterThreshold( mVerticalTriangleThresholdSpinBox->value() );
  }

  return retSymb;
}

void QgsPointCloud3DSymbolWidget::setColorRampMinMax( double min, double max )
{
  whileBlocking( mColorRampShaderMinEdit )->setValue( min );
  whileBlocking( mColorRampShaderMaxEdit )->setValue( max );
}

void QgsPointCloud3DSymbolWidget::createValidators()
{
  mRedMinLineEdit->setValidator( new QgsDoubleValidator( mRedMinLineEdit ) );
  mRedMaxLineEdit->setValidator( new QgsDoubleValidator( mRedMinLineEdit ) );
  mGreenMinLineEdit->setValidator( new QgsDoubleValidator( mGreenMinLineEdit ) );
  mGreenMaxLineEdit->setValidator( new QgsDoubleValidator( mGreenMinLineEdit ) );
  mBlueMinLineEdit->setValidator( new QgsDoubleValidator( mBlueMinLineEdit ) );
  mBlueMaxLineEdit->setValidator( new QgsDoubleValidator( mBlueMinLineEdit ) );
}

void QgsPointCloud3DSymbolWidget::setCustomMinMaxValues( QgsRgbPointCloud3DSymbol *symbol ) const
{
  if ( !symbol )
  {
    return;
  }

  if ( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ==
       QgsContrastEnhancement::NoEnhancement )
  {
    symbol->setRedContrastEnhancement( nullptr );
    symbol->setGreenContrastEnhancement( nullptr );
    symbol->setBlueContrastEnhancement( nullptr );
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
  symbol->setRedContrastEnhancement( redEnhancement );
  symbol->setGreenContrastEnhancement( greenEnhancement );
  symbol->setBlueContrastEnhancement( blueEnhancement );
}

void QgsPointCloud3DSymbolWidget::minMaxModified()
{
  if ( !mDisableMinMaxWidgetRefresh )
  {
    if ( ( QgsContrastEnhancement::ContrastEnhancementAlgorithm )( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ) == QgsContrastEnhancement::NoEnhancement )
    {
      mContrastEnhancementAlgorithmComboBox->setCurrentIndex(
        mContrastEnhancementAlgorithmComboBox->findData( ( int ) QgsContrastEnhancement::StretchToMinimumMaximum ) );
    }
    emitChangedSignal();
  }
}

void QgsPointCloud3DSymbolWidget::setMinMaxValue( const QgsContrastEnhancement *ce, QLineEdit *minEdit, QLineEdit *maxEdit )
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

void QgsPointCloud3DSymbolWidget::reloadColorRampShaderMinMax()
{
  const double min = mColorRampShaderMinEdit->value();
  const double max = mColorRampShaderMaxEdit->value();
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
      if ( renderer2d )
      {
        mRedAttributeComboBox->setAttribute( renderer2d->redAttribute() );
        mGreenAttributeComboBox->setAttribute( renderer2d->greenAttribute() );
        mBlueAttributeComboBox->setAttribute( renderer2d->blueAttribute() );

        mDisableMinMaxWidgetRefresh++;
        setMinMaxValue( renderer2d->redContrastEnhancement(), mRedMinLineEdit, mRedMaxLineEdit );
        setMinMaxValue( renderer2d->greenContrastEnhancement(), mGreenMinLineEdit, mGreenMaxLineEdit );
        setMinMaxValue( renderer2d->blueContrastEnhancement(), mBlueMinLineEdit, mBlueMaxLineEdit );
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
        redAttributeChanged();
        greenAttributeChanged();
        blueAttributeChanged();
      }

      ( void )( renderer2d );
      mBlockChangedSignals--;
    }
    else if ( newSymbolType == QLatin1String( "classification" ) )
    {
      const QgsPointCloudClassifiedRenderer *renderer2d = dynamic_cast< const QgsPointCloudClassifiedRenderer * >( mLayer->renderer() );
      mBlockChangedSignals++;
      if ( renderer2d )
      {
        mClassifiedRendererWidget->setFromCategories( renderer2d->categories(), renderer2d->attribute() );
      }
      else
      {
        mClassifiedRendererWidget->setFromCategories( QgsPointCloudClassifiedRenderer::defaultCategories(), QString() );
      }

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

    if ( mRenderingParameterComboBox->currentAttribute() == QLatin1String( "Z" ) )
    {
      const double zScale = static_cast< const QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() )->zScale();
      const double zOffset = static_cast< const QgsPointCloudLayerElevationProperties * >( mLayer->elevationProperties() )->zOffset();
      mProviderMin = mProviderMin * zScale + zOffset;
      mProviderMax = mProviderMax * zScale + zOffset;
    }
  }
  if ( !mBlockSetMinMaxFromLayer )
    setMinMaxFromLayer();
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

void QgsPointCloud3DSymbolWidget::mRedMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloud3DSymbolWidget::mRedMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloud3DSymbolWidget::mGreenMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloud3DSymbolWidget::mGreenMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloud3DSymbolWidget::mBlueMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloud3DSymbolWidget::mBlueMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsPointCloud3DSymbolWidget::redAttributeChanged()
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
      emitChangedSignal();
    }
  }
}

void QgsPointCloud3DSymbolWidget::greenAttributeChanged()
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
      emitChangedSignal();
    }
  }
}

void QgsPointCloud3DSymbolWidget::blueAttributeChanged()
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
      emitChangedSignal();
    }
  }
}

void QgsPointCloud3DSymbolWidget::setMaximumScreenError( double maxScreenError )
{
  whileBlocking( mMaxScreenErrorSpinBox )->setValue( maxScreenError );
}

double QgsPointCloud3DSymbolWidget::maximumScreenError() const
{
  return mMaxScreenErrorSpinBox->value();
}

void QgsPointCloud3DSymbolWidget::setShowBoundingBoxes( bool showBoundingBoxes )
{
  whileBlocking( mShowBoundingBoxesCheckBox )->setChecked( showBoundingBoxes );
}

void QgsPointCloud3DSymbolWidget::setPointBudget( double budget )
{
  whileBlocking( mPointBudgetSpinBox )->setValue( budget );
}

double QgsPointCloud3DSymbolWidget::pointBudget() const
{
  return mPointBudgetSpinBox->value();
}


void QgsPointCloud3DSymbolWidget::setPointCloudSize( int size )
{
  mPointCloudSizeLabel->setText( QStringLiteral( "%1 points" ).arg( size ) );
}

bool QgsPointCloud3DSymbolWidget::showBoundingBoxes() const
{
  return mShowBoundingBoxesCheckBox->isChecked();
}

void QgsPointCloud3DSymbolWidget::connectChildPanels( QgsPanelWidget *parent )
{
  parent->connectChildPanel( mClassifiedRendererWidget );
}
