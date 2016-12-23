/***************************************************************************
                         qgssinglebandgrayrendererwidget.h
                         ---------------------------------
    begin                : March 2012
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

#include "qgssinglebandgrayrendererwidget.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterminmaxwidget.h"

QgsSingleBandGrayRendererWidget::QgsSingleBandGrayRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent )
    : QgsRasterRendererWidget( layer, extent )
    , mMinMaxWidget( nullptr )
    , mDisableMinMaxWidgetRefresh( false )
{
  setupUi( this );

  mGradientComboBox->insertItem( 0, tr( "Black to white" ), QgsSingleBandGrayRenderer::BlackToWhite );
  mGradientComboBox->insertItem( 1, tr( "White to black" ), QgsSingleBandGrayRenderer::WhiteToBlack );

  mMinLineEdit->setValidator( new QDoubleValidator( mMinLineEdit ) );
  mMaxLineEdit->setValidator( new QDoubleValidator( mMaxLineEdit ) );

  if ( mRasterLayer )
  {
    QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
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
             this, &QgsSingleBandGrayRendererWidget::widgetChanged );

    connect( mMinMaxWidget, &QgsRasterMinMaxWidget::load,
             this, &QgsSingleBandGrayRendererWidget::loadMinMax );

    //fill available bands into combo box
    int nBands = provider->bandCount();
    for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
    {
      mGrayBandComboBox->addItem( displayBandName( i ), i );
    }

    //contrast enhancement algorithms
    mContrastEnhancementComboBox->addItem( tr( "No enhancement" ), QgsContrastEnhancement::NoEnhancement );
    mContrastEnhancementComboBox->addItem( tr( "Stretch to MinMax" ), QgsContrastEnhancement::StretchToMinimumMaximum );
    mContrastEnhancementComboBox->addItem( tr( "Stretch and clip to MinMax" ), QgsContrastEnhancement::StretchAndClipToMinimumMaximum );
    mContrastEnhancementComboBox->addItem( tr( "Clip to MinMax" ), QgsContrastEnhancement::ClipToMinimumMaximum );

    setFromRenderer( layer->renderer() );

    connect( mGradientComboBox, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( widgetChanged() ) );
    connect( mContrastEnhancementComboBox, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( widgetChanged() ) );
  }
}

QgsSingleBandGrayRendererWidget::~QgsSingleBandGrayRendererWidget()
{
}

QgsRasterRenderer* QgsSingleBandGrayRendererWidget::renderer()
{
  if ( !mRasterLayer )
  {
    return nullptr;
  }
  QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return nullptr;
  }
  int band = mGrayBandComboBox->currentData().toInt();

  QgsContrastEnhancement* e = new QgsContrastEnhancement(( Qgis::DataType )(
        provider->dataType( band ) ) );
  e->setMinimumValue( mMinLineEdit->text().toDouble() );
  e->setMaximumValue( mMaxLineEdit->text().toDouble() );
  e->setContrastEnhancementAlgorithm(( QgsContrastEnhancement::ContrastEnhancementAlgorithm )( mContrastEnhancementComboBox->currentData().toInt() ) );

  QgsSingleBandGrayRenderer* renderer = new QgsSingleBandGrayRenderer( provider, band );
  renderer->setContrastEnhancement( e );

  renderer->setGradient(( QgsSingleBandGrayRenderer::Gradient ) mGradientComboBox->currentData().toInt() );
  renderer->setMinMaxOrigin( mMinMaxWidget->minMaxOrigin() );

  return renderer;
}

void QgsSingleBandGrayRendererWidget::doComputations()
{
  mMinMaxWidget->doComputations();
}

void QgsSingleBandGrayRendererWidget::setMapCanvas( QgsMapCanvas* canvas )
{
  QgsRasterRendererWidget::setMapCanvas( canvas );
  mMinMaxWidget->setMapCanvas( canvas );
}

void QgsSingleBandGrayRendererWidget::on_mMinLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsSingleBandGrayRendererWidget::on_mMaxLineEdit_textChanged( const QString & )
{
  minMaxModified();
}

void QgsSingleBandGrayRendererWidget::minMaxModified()
{
  if ( !mDisableMinMaxWidgetRefresh )
  {
    if (( QgsContrastEnhancement::ContrastEnhancementAlgorithm )( mContrastEnhancementComboBox->currentData().toInt() ) == QgsContrastEnhancement::NoEnhancement )
    {
      mContrastEnhancementComboBox->setCurrentIndex(
        mContrastEnhancementComboBox->findData(( int ) QgsContrastEnhancement::StretchToMinimumMaximum ) );
    }
    mMinMaxWidget->userHasSetManualMinMaxValues();
    emit widgetChanged();
  }
}


void QgsSingleBandGrayRendererWidget::loadMinMax( int theBandNo, double theMin, double theMax )
{
  Q_UNUSED( theBandNo );

  QgsDebugMsg( QString( "theBandNo = %1 theMin = %2 theMax = %3" ).arg( theBandNo ).arg( theMin ).arg( theMax ) );

  mDisableMinMaxWidgetRefresh = true;
  if ( qIsNaN( theMin ) )
  {
    mMinLineEdit->clear();
  }
  else
  {
    mMinLineEdit->setText( QString::number( theMin ) );
  }

  if ( qIsNaN( theMax ) )
  {
    mMaxLineEdit->clear();
  }
  else
  {
    mMaxLineEdit->setText( QString::number( theMax ) );
  }
  mDisableMinMaxWidgetRefresh = false;
}

void QgsSingleBandGrayRendererWidget::on_mGrayBandComboBox_currentIndexChanged( int index )
{
  QList<int> myBands;
  myBands.append( mGrayBandComboBox->itemData( index ).toInt() );
  mMinMaxWidget->setBands( myBands );
  emit widgetChanged();
}

void QgsSingleBandGrayRendererWidget::setFromRenderer( const QgsRasterRenderer* r )
{
  const QgsSingleBandGrayRenderer* gr = dynamic_cast<const QgsSingleBandGrayRenderer*>( r );
  if ( gr )
  {
    //band
    mGrayBandComboBox->setCurrentIndex( mGrayBandComboBox->findData( gr->grayBand() ) );
    const QgsContrastEnhancement* ce = gr->contrastEnhancement();

    mGradientComboBox->setCurrentIndex( mGradientComboBox->findData( gr->gradient() ) );
    //minmax
    mDisableMinMaxWidgetRefresh = true;
    mMinLineEdit->setText( QString::number( ce->minimumValue() ) );
    mMaxLineEdit->setText( QString::number( ce->maximumValue() ) );
    mDisableMinMaxWidgetRefresh = false;
    //contrast enhancement algorithm
    mContrastEnhancementComboBox->setCurrentIndex(
      mContrastEnhancementComboBox->findData(( int )( ce->contrastEnhancementAlgorithm() ) ) );

    mMinMaxWidget->setFromMinMaxOrigin( gr->minMaxOrigin() );
  }
}

void QgsSingleBandGrayRendererWidget::setMin( const QString& value, int )
{
  mDisableMinMaxWidgetRefresh = true;
  mMinLineEdit->setText( value );
  mDisableMinMaxWidgetRefresh = false;
}

void QgsSingleBandGrayRendererWidget::setMax( const QString& value, int )
{
  mDisableMinMaxWidgetRefresh = true;
  mMaxLineEdit->setText( value );
  mDisableMinMaxWidgetRefresh = false;
}