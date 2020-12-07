/***************************************************************************
  qgsrastercontourrendererwidget.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastercontourrendererwidget.h"

#include "qgsrastercontourrenderer.h"
#include "qgsrasterlayer.h"


QgsRasterContourRendererWidget::QgsRasterContourRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
  : QgsRasterRendererWidget( layer, extent )
{
  setupUi( this );

  mContourSymbolButton->setSymbolType( QgsSymbol::Line );
  mIndexContourSymbolButton->setSymbolType( QgsSymbol::Line );

  mInputBandComboBox->setLayer( mRasterLayer );

  if ( !mRasterLayer )
  {
    return;
  }
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return;
  }

  const QgsRasterContourRenderer *rcr = dynamic_cast<const QgsRasterContourRenderer *>( mRasterLayer->renderer() );
  if ( rcr )
  {
    mInputBandComboBox->setBand( rcr->inputBand() );
    mContourIntervalSpinBox->setValue( rcr->contourInterval() );
    mIndexContourIntervalSpinBox->setValue( rcr->contourIndexInterval() );
    mDownscaleSpinBox->setValue( rcr->downscale() );
    if ( rcr->contourSymbol() )
      mContourSymbolButton->setSymbol( rcr->contourSymbol()->clone() );
    if ( rcr->contourIndexSymbol() )
      mIndexContourSymbolButton->setSymbol( rcr->contourIndexSymbol()->clone() );
  }

  connect( mInputBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsRasterRendererWidget::widgetChanged );
  connect( mContourIntervalSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsRasterRendererWidget::widgetChanged );
  connect( mIndexContourIntervalSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsRasterRendererWidget::widgetChanged );
  connect( mDownscaleSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsRasterRendererWidget::widgetChanged );
  connect( mContourSymbolButton, &QgsSymbolButton::changed, this, &QgsRasterRendererWidget::widgetChanged );
  connect( mIndexContourSymbolButton, &QgsSymbolButton::changed, this, &QgsRasterRendererWidget::widgetChanged );
}

QgsRasterRenderer *QgsRasterContourRendererWidget::renderer()
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

  QgsRasterContourRenderer *renderer = new QgsRasterContourRenderer( provider );
  renderer->setInputBand( mInputBandComboBox->currentBand() );
  renderer->setContourInterval( mContourIntervalSpinBox->value() );
  renderer->setContourIndexInterval( mIndexContourIntervalSpinBox->value() );
  renderer->setDownscale( mDownscaleSpinBox->value() );
  renderer->setContourSymbol( mContourSymbolButton->clonedSymbol<QgsLineSymbol>() );
  renderer->setContourIndexSymbol( mIndexContourSymbolButton->clonedSymbol<QgsLineSymbol>() );
  return renderer;
}
