/***************************************************************************
                         qgsmultibandcolorrendererwidget.cpp
                         -----------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmultibandcolorrendererwidget.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterminmaxwidget.h"
#include "qgsdoublevalidator.h"

QgsMultiBandColorRendererWidget::QgsMultiBandColorRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
  : QgsRasterRendererWidget( layer, extent )
  , mDisableMinMaxWidgetRefresh( false )
{
  setupUi( this );
  connect( mRedMinLineEdit, &QLineEdit::textChanged, this, &QgsMultiBandColorRendererWidget::mRedMinLineEdit_textChanged );
  connect( mRedMaxLineEdit, &QLineEdit::textChanged, this, &QgsMultiBandColorRendererWidget::mRedMaxLineEdit_textChanged );
  connect( mGreenMinLineEdit, &QLineEdit::textChanged, this, &QgsMultiBandColorRendererWidget::mGreenMinLineEdit_textChanged );
  connect( mGreenMaxLineEdit, &QLineEdit::textChanged, this, &QgsMultiBandColorRendererWidget::mGreenMaxLineEdit_textChanged );
  connect( mBlueMinLineEdit, &QLineEdit::textChanged, this, &QgsMultiBandColorRendererWidget::mBlueMinLineEdit_textChanged );
  connect( mBlueMaxLineEdit, &QLineEdit::textChanged, this, &QgsMultiBandColorRendererWidget::mBlueMaxLineEdit_textChanged );
  createValidators();

  if ( mRasterLayer )
  {
    QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
    if ( !provider )
    {
      return;
    }

    mMinMaxWidget = new QgsRasterMinMaxWidget( layer, this );
    mMinMaxWidget->setExtent( extent );
    mMinMaxWidget->setMapCanvas( mCanvas );
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins( 0, 0, 0, 0 );
    mMinMaxContainerWidget->setLayout( layout );
    layout->addWidget( mMinMaxWidget );

    connect( mMinMaxWidget, &QgsRasterMinMaxWidget::widgetChanged,
             this, &QgsMultiBandColorRendererWidget::widgetChanged );
    connect( mMinMaxWidget, &QgsRasterMinMaxWidget::load,
             this, &QgsMultiBandColorRendererWidget::loadMinMax );

    connect( mRedBandComboBox, &QgsRasterBandComboBox::bandChanged,
             this, &QgsMultiBandColorRendererWidget::onBandChanged );
    connect( mGreenBandComboBox, &QgsRasterBandComboBox::bandChanged,
             this, &QgsMultiBandColorRendererWidget::onBandChanged );
    connect( mBlueBandComboBox, &QgsRasterBandComboBox::bandChanged,
             this, &QgsMultiBandColorRendererWidget::onBandChanged );

    mRedBandComboBox->setShowNotSetOption( true );
    mGreenBandComboBox->setShowNotSetOption( true );
    mBlueBandComboBox->setShowNotSetOption( true );
    mRedBandComboBox->setLayer( mRasterLayer );
    mGreenBandComboBox->setLayer( mRasterLayer );
    mBlueBandComboBox->setLayer( mRasterLayer );

    //contrast enhancement algorithms
    mContrastEnhancementAlgorithmComboBox->addItem( tr( "No Enhancement" ), QgsContrastEnhancement::NoEnhancement );
    mContrastEnhancementAlgorithmComboBox->addItem( tr( "Stretch to MinMax" ), QgsContrastEnhancement::StretchToMinimumMaximum );
    mContrastEnhancementAlgorithmComboBox->addItem( tr( "Stretch and Clip to MinMax" ), QgsContrastEnhancement::StretchAndClipToMinimumMaximum );
    mContrastEnhancementAlgorithmComboBox->addItem( tr( "Clip to MinMax" ), QgsContrastEnhancement::ClipToMinimumMaximum );

    setFromRenderer( mRasterLayer->renderer() );
    onBandChanged( 0 ); // reset mMinMaxWidget bands

    connect( mContrastEnhancementAlgorithmComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterRendererWidget::widgetChanged );
  }
}

QgsRasterRenderer *QgsMultiBandColorRendererWidget::renderer()
{
  if ( !mRasterLayer )
  {
    return nullptr;
  }
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return nullptr;
  }

  const int redBand = mRedBandComboBox->currentBand();
  const int greenBand = mGreenBandComboBox->currentBand();
  const int blueBand = mBlueBandComboBox->currentBand();

  QgsMultiBandColorRenderer *r = new QgsMultiBandColorRenderer( provider, redBand, greenBand, blueBand );
  setCustomMinMaxValues( r, provider, redBand, greenBand, blueBand );

  r->setMinMaxOrigin( mMinMaxWidget->minMaxOrigin() );

  return r;
}

void QgsMultiBandColorRendererWidget::doComputations()
{
  mMinMaxWidget->doComputations();
}

void QgsMultiBandColorRendererWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  QgsRasterRendererWidget::setMapCanvas( canvas );
  mMinMaxWidget->setMapCanvas( canvas );
}

void QgsMultiBandColorRendererWidget::createValidators()
{
  mRedMinLineEdit->setValidator( new QgsDoubleValidator( mRedMinLineEdit ) );
  mRedMaxLineEdit->setValidator( new QgsDoubleValidator( mRedMinLineEdit ) );
  mGreenMinLineEdit->setValidator( new QgsDoubleValidator( mGreenMinLineEdit ) );
  mGreenMaxLineEdit->setValidator( new QgsDoubleValidator( mGreenMinLineEdit ) );
  mBlueMinLineEdit->setValidator( new QgsDoubleValidator( mBlueMinLineEdit ) );
  mBlueMaxLineEdit->setValidator( new QgsDoubleValidator( mBlueMinLineEdit ) );
}

void QgsMultiBandColorRendererWidget::setCustomMinMaxValues( QgsMultiBandColorRenderer *r,
    const QgsRasterDataProvider *provider,
    int redBand, int greenBand, int blueBand )
{
  if ( !r || !provider )
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
  if ( redMinOk && redMaxOk && redBand != -1 )
  {
    redEnhancement = new QgsContrastEnhancement( ( Qgis::DataType )(
          provider->dataType( redBand ) ) );
    redEnhancement->setMinimumValue( redMin );
    redEnhancement->setMaximumValue( redMax );
  }

  bool greenMinOk, greenMaxOk;
  const double greenMin = QgsDoubleValidator::toDouble( mGreenMinLineEdit->text(), &greenMinOk );
  const double greenMax = QgsDoubleValidator::toDouble( mGreenMaxLineEdit->text(), &greenMaxOk );
  if ( greenMinOk && greenMaxOk && greenBand != -1 )
  {
    greenEnhancement = new QgsContrastEnhancement( ( Qgis::DataType )(
          provider->dataType( greenBand ) ) );
    greenEnhancement->setMinimumValue( greenMin );
    greenEnhancement->setMaximumValue( greenMax );
  }

  bool blueMinOk, blueMaxOk;
  const double blueMin = QgsDoubleValidator::toDouble( mBlueMinLineEdit->text(), &blueMinOk );
  const double blueMax = QgsDoubleValidator::toDouble( mBlueMaxLineEdit->text(), &blueMaxOk );
  if ( blueMinOk && blueMaxOk && blueBand != -1 )
  {
    blueEnhancement = new QgsContrastEnhancement( ( Qgis::DataType )(
          provider->dataType( blueBand ) ) );
    blueEnhancement->setMinimumValue( blueMin );
    blueEnhancement->setMaximumValue( blueMax );
  }

  if ( redEnhancement )
  {
    redEnhancement->setContrastEnhancementAlgorithm( ( QgsContrastEnhancement::ContrastEnhancementAlgorithm )
        ( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ) );
  }
  if ( greenEnhancement )
  {
    greenEnhancement->setContrastEnhancementAlgorithm( ( QgsContrastEnhancement::ContrastEnhancementAlgorithm )
        ( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ) );
  }
  if ( blueEnhancement )
  {
    blueEnhancement->setContrastEnhancementAlgorithm( ( QgsContrastEnhancement::ContrastEnhancementAlgorithm )
        ( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ) );
  }
  r->setRedContrastEnhancement( redEnhancement );
  r->setGreenContrastEnhancement( greenEnhancement );
  r->setBlueContrastEnhancement( blueEnhancement );
}

void QgsMultiBandColorRendererWidget::onBandChanged( int index )
{
  Q_UNUSED( index )

  QList<int> myBands;
  myBands.append( mRedBandComboBox->currentBand() );
  myBands.append( mGreenBandComboBox->currentBand() );
  myBands.append( mBlueBandComboBox->currentBand() );
  mMinMaxWidget->setBands( myBands );
  emit widgetChanged();
}

void QgsMultiBandColorRendererWidget::mRedMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsMultiBandColorRendererWidget::mRedMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsMultiBandColorRendererWidget::mGreenMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsMultiBandColorRendererWidget::mGreenMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsMultiBandColorRendererWidget::mBlueMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsMultiBandColorRendererWidget::mBlueMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsMultiBandColorRendererWidget::minMaxModified()
{
  if ( !mDisableMinMaxWidgetRefresh )
  {
    if ( ( QgsContrastEnhancement::ContrastEnhancementAlgorithm )( mContrastEnhancementAlgorithmComboBox->currentData().toInt() ) == QgsContrastEnhancement::NoEnhancement )
    {
      mContrastEnhancementAlgorithmComboBox->setCurrentIndex(
        mContrastEnhancementAlgorithmComboBox->findData( ( int ) QgsContrastEnhancement::StretchToMinimumMaximum ) );
    }
    mMinMaxWidget->userHasSetManualMinMaxValues();
    emit widgetChanged();
  }
}

void QgsMultiBandColorRendererWidget::loadMinMax( int bandNo, double min, double max )
{
  QgsDebugMsg( QStringLiteral( "theBandNo = %1 min = %2 max = %3" ).arg( bandNo ).arg( min ).arg( max ) );

  QLineEdit *myMinLineEdit, *myMaxLineEdit;

  if ( mRedBandComboBox->currentBand() == bandNo )
  {
    myMinLineEdit = mRedMinLineEdit;
    myMaxLineEdit = mRedMaxLineEdit;
  }
  else if ( mGreenBandComboBox->currentBand() == bandNo )
  {
    myMinLineEdit = mGreenMinLineEdit;
    myMaxLineEdit = mGreenMaxLineEdit;
  }
  else if ( mBlueBandComboBox->currentBand() == bandNo )
  {
    myMinLineEdit = mBlueMinLineEdit;
    myMaxLineEdit = mBlueMaxLineEdit;
  }
  else // should not happen
  {
    QgsDebugMsg( QStringLiteral( "Band not found" ) );
    return;
  }

  mDisableMinMaxWidgetRefresh = true;
  if ( std::isnan( min ) )
  {
    myMinLineEdit->clear();
  }
  else
  {
    myMinLineEdit->setText( QLocale().toString( min ) );
  }

  if ( std::isnan( max ) )
  {
    myMaxLineEdit->clear();
  }
  else
  {
    myMaxLineEdit->setText( QLocale().toString( max ) );
  }
  mDisableMinMaxWidgetRefresh = false;
}

void QgsMultiBandColorRendererWidget::setMinMaxValue( const QgsContrastEnhancement *ce, QLineEdit *minEdit, QLineEdit *maxEdit )
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
        ( int )( ce->contrastEnhancementAlgorithm() ) ) );
}

void QgsMultiBandColorRendererWidget::setFromRenderer( const QgsRasterRenderer *r )
{
  const QgsMultiBandColorRenderer *mbcr = dynamic_cast<const QgsMultiBandColorRenderer *>( r );
  if ( mbcr )
  {
    mRedBandComboBox->setBand( mbcr->redBand() );
    mGreenBandComboBox->setBand( mbcr->greenBand() );
    mBlueBandComboBox->setBand( mbcr->blueBand() );

    mDisableMinMaxWidgetRefresh = true;
    setMinMaxValue( mbcr->redContrastEnhancement(), mRedMinLineEdit, mRedMaxLineEdit );
    setMinMaxValue( mbcr->greenContrastEnhancement(), mGreenMinLineEdit, mGreenMaxLineEdit );
    setMinMaxValue( mbcr->blueContrastEnhancement(), mBlueMinLineEdit, mBlueMaxLineEdit );
    mDisableMinMaxWidgetRefresh = false;

    mMinMaxWidget->setFromMinMaxOrigin( mbcr->minMaxOrigin() );
  }
  else
  {
    if ( mRedBandComboBox->findText( tr( "Red" ) ) > -1 && mRedBandComboBox->findText( tr( "Green" ) ) > -1 &&
         mRedBandComboBox->findText( tr( "Blue" ) ) > -1 )
    {
      mRedBandComboBox->setCurrentIndex( mRedBandComboBox->findText( tr( "Red" ) ) );
      mGreenBandComboBox->setCurrentIndex( mGreenBandComboBox->findText( tr( "Green" ) ) );
      mBlueBandComboBox->setCurrentIndex( mBlueBandComboBox->findText( tr( "Blue" ) ) );
    }
    else
    {
      mRedBandComboBox->setCurrentIndex( mRedBandComboBox->count() > 1 ? 1 : 0 );
      mGreenBandComboBox->setCurrentIndex( mRedBandComboBox->count() > 2 ? 2 : 0 );
      mBlueBandComboBox->setCurrentIndex( mRedBandComboBox->count() > 3 ? 3 : 0 );
    }
  }
}

QString QgsMultiBandColorRendererWidget::min( int index )
{
  switch ( index )
  {
    case 0:
      return mRedMinLineEdit->text();
    case 1:
      return mGreenMinLineEdit->text();
    case 2:
      return mBlueMinLineEdit->text();
    default:
      break;
  }
  return QString();
}

QString QgsMultiBandColorRendererWidget::max( int index )
{
  switch ( index )
  {
    case 0:
      return mRedMaxLineEdit->text();
    case 1:
      return mGreenMaxLineEdit->text();
    case 2:
      return mBlueMaxLineEdit->text();
    default:
      break;
  }
  return QString();
}

void QgsMultiBandColorRendererWidget::setMin( const QString &value, int index )
{
  mDisableMinMaxWidgetRefresh = true;
  switch ( index )
  {
    case 0:
      mRedMinLineEdit->setText( value );
      break;
    case 1:
      mGreenMinLineEdit->setText( value );
      break;
    case 2:
      mBlueMinLineEdit->setText( value );
      break;
    default:
      break;
  }
  mDisableMinMaxWidgetRefresh = false;
}

void QgsMultiBandColorRendererWidget::setMax( const QString &value, int index )
{
  mDisableMinMaxWidgetRefresh = true;
  switch ( index )
  {
    case 0:
      mRedMaxLineEdit->setText( value );
      break;
    case 1:
      mGreenMaxLineEdit->setText( value );
      break;
    case 2:
      mBlueMaxLineEdit->setText( value );
      break;
    default:
      break;
  }
  mDisableMinMaxWidgetRefresh = false;
}

int QgsMultiBandColorRendererWidget::selectedBand( int index )
{
  switch ( index )
  {
    case 0:
      return mRedBandComboBox->currentBand();
    case 1:
      return mGreenBandComboBox->currentBand();
    case 2:
      return mBlueBandComboBox->currentBand();
    default:
      break;
  }
  return -1;
}
