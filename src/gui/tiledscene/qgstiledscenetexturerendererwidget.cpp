/***************************************************************************
    qgstiledscenetexturerendererwidget.cpp
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgstiledscenetexturerendererwidget.h"
#include "moc_qgstiledscenetexturerendererwidget.cpp"
#include "qgstiledscenelayer.h"
#include "qgstiledscenetexturerenderer.h"
#include "qgsfillsymbol.h"

///@cond PRIVATE

QgsTiledSceneTextureRendererWidget::QgsTiledSceneTextureRendererWidget( QgsTiledSceneLayer *layer, QgsStyle *style )
  : QgsTiledSceneRendererWidget( layer, style )
{
  setupUi( this );

  mFillSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mFillSymbolButton->setSymbol( QgsTiledSceneTextureRenderer::createDefaultFillSymbol() );

  if ( layer )
  {
    setFromRenderer( layer->renderer() );
  }

  connect( mFillSymbolButton, &QgsSymbolButton::changed, this, &QgsTiledSceneTextureRendererWidget::emitWidgetChanged );
}

QgsTiledSceneRendererWidget *QgsTiledSceneTextureRendererWidget::create( QgsTiledSceneLayer *layer, QgsStyle *style, QgsTiledSceneRenderer * )
{
  return new QgsTiledSceneTextureRendererWidget( layer, style );
}

QgsTiledSceneRenderer *QgsTiledSceneTextureRendererWidget::renderer()
{
  std::unique_ptr<QgsTiledSceneTextureRenderer> renderer = std::make_unique<QgsTiledSceneTextureRenderer>();
  renderer->setFillSymbol( mFillSymbolButton->clonedSymbol<QgsFillSymbol>() );

  return renderer.release();
}

void QgsTiledSceneTextureRendererWidget::emitWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit widgetChanged();
}

void QgsTiledSceneTextureRendererWidget::setFromRenderer( const QgsTiledSceneRenderer *renderer )
{
  mBlockChangedSignal = true;

  if ( const QgsTiledSceneTextureRenderer *textureRenderer = dynamic_cast<const QgsTiledSceneTextureRenderer *>( renderer ) )
  {
    mFillSymbolButton->setSymbol( textureRenderer->fillSymbol()->clone() );
  }

  mBlockChangedSignal = false;
}

///@endcond
