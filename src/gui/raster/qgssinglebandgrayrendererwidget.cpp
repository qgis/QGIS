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

QgsSingleBandGrayRendererWidget::QgsSingleBandGrayRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent ): QgsRasterRendererWidget( layer, extent )
{
  setupUi( this );

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
    layout()->addWidget( mMinMaxWidget );
    connect( mMinMaxWidget, SIGNAL( load( int, double, double ) ),
             this, SLOT( loadMinMax( int, double, double ) ) );

    //fill available bands into combo box
    int nBands = provider->bandCount();
    for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
    {
      mGrayBandComboBox->addItem( displayBandName( i ), i );
    }

    //contrast enhancement algorithms
    mContrastEnhancementComboBox->addItem( tr( "No enhancement" ), 0 );
    mContrastEnhancementComboBox->addItem( tr( "Stretch to MinMax" ), 1 );
    mContrastEnhancementComboBox->addItem( tr( "Stretch and clip to MinMax" ), 2 );
    mContrastEnhancementComboBox->addItem( tr( "Clip to MinMax" ), 3 );

    setFromRenderer( layer->renderer() );
  }
}

QgsSingleBandGrayRendererWidget::~QgsSingleBandGrayRendererWidget()
{
}

QgsRasterRenderer* QgsSingleBandGrayRendererWidget::renderer()
{
  if ( !mRasterLayer )
  {
    return 0;
  }
  QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return 0;
  }
  int band = mGrayBandComboBox->itemData( mGrayBandComboBox->currentIndex() ).toInt();

  QgsContrastEnhancement* e = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
        provider->dataType( band ) ) );
  e->setMinimumValue( mMinLineEdit->text().toDouble() );
  e->setMaximumValue( mMaxLineEdit->text().toDouble() );
  e->setContrastEnhancementAlgorithm(( QgsContrastEnhancement::ContrastEnhancementAlgorithm )( mContrastEnhancementComboBox->itemData(
                                       mContrastEnhancementComboBox->currentIndex() ).toInt() ) );


  QgsSingleBandGrayRenderer* renderer = new QgsSingleBandGrayRenderer( provider, band );
  renderer->setContrastEnhancement( e );
  return renderer;
}

void QgsSingleBandGrayRendererWidget::loadMinMax( int theBandNo, double theMin, double theMax )
{
  QgsDebugMsg( QString( "theBandNo = %1 theMin = %2 theMax = %3" ).arg( theBandNo ).arg( theMin ).arg( theMax ) );

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
}

void QgsSingleBandGrayRendererWidget::on_mGrayBandComboBox_currentIndexChanged( int index )
{
  QList<int> myBands;
  myBands.append( mGrayBandComboBox->itemData( index ).toInt() );
  mMinMaxWidget->setBands( myBands );
}

void QgsSingleBandGrayRendererWidget::setFromRenderer( const QgsRasterRenderer* r )
{
  const QgsSingleBandGrayRenderer* gr = dynamic_cast<const QgsSingleBandGrayRenderer*>( r );
  if ( gr )
  {
    //band
    mGrayBandComboBox->setCurrentIndex( mGrayBandComboBox->findData( gr->grayBand() ) );
    const QgsContrastEnhancement* ce = gr->contrastEnhancement();
    //minmax
    mMinLineEdit->setText( QString::number( ce->minimumValue() ) );
    mMaxLineEdit->setText( QString::number( ce->maximumValue() ) );
    //contrast enhancement algorithm
    mContrastEnhancementComboBox->setCurrentIndex(
      mContrastEnhancementComboBox->findData(( int )( ce->contrastEnhancementAlgorithm() ) ) );
  }
}
