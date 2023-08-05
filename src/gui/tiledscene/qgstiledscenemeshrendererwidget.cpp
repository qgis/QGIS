/***************************************************************************
    qgstiledscenemeshrendererwidget.cpp
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

#include "qgstiledscenemeshrendererwidget.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenemeshrenderer.h"
#include "qgsfillsymbol.h"

///@cond PRIVATE

QgsTiledSceneMeshRendererWidget::QgsTiledSceneMeshRendererWidget( QgsTiledSceneLayer *layer, QgsStyle *style )
  : QgsTiledSceneRendererWidget( layer, style )
{
  setupUi( this );

  mFillSymbolButton->setSymbolType( Qgis::SymbolType::Fill );

  if ( layer )
  {
    setFromRenderer( layer->renderer() );
  }

  connect( mFillSymbolButton, &QgsSymbolButton::changed, this, &QgsTiledSceneMeshRendererWidget::emitWidgetChanged );
}

QgsTiledSceneRendererWidget *QgsTiledSceneMeshRendererWidget::create( QgsTiledSceneLayer *layer, QgsStyle *style, QgsTiledSceneRenderer * )
{
  return new QgsTiledSceneMeshRendererWidget( layer, style );
}

QgsTiledSceneRenderer *QgsTiledSceneMeshRendererWidget::renderer()
{
  std::unique_ptr< QgsTiledSceneMeshRenderer > renderer = std::make_unique< QgsTiledSceneMeshRenderer >();
  renderer->setFillSymbol( mFillSymbolButton->clonedSymbol< QgsFillSymbol >() );

  return renderer.release();
}

void QgsTiledSceneMeshRendererWidget::emitWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit widgetChanged();
}

void QgsTiledSceneMeshRendererWidget::setFromRenderer( const QgsTiledSceneRenderer *renderer )
{
  mBlockChangedSignal = true;
  if ( const QgsTiledSceneMeshRenderer *meshRenderer = dynamic_cast< const QgsTiledSceneMeshRenderer * >( renderer ) )
  {
    mFillSymbolButton->setSymbol( meshRenderer->fillSymbol()->clone() );
  }
  mBlockChangedSignal = false;
}

///@endcond
