/***************************************************************************
    qgstiledscenewireframerendererwidget.cpp
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

#include "qgstiledscenewireframerendererwidget.h"
#include "moc_qgstiledscenewireframerendererwidget.cpp"
#include "qgstiledscenelayer.h"
#include "qgstiledscenewireframerenderer.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"

///@cond PRIVATE

QgsTiledSceneWireframeRendererWidget::QgsTiledSceneWireframeRendererWidget( QgsTiledSceneLayer *layer, QgsStyle *style )
  : QgsTiledSceneRendererWidget( layer, style )
{
  setupUi( this );

  mFillSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mFillSymbolButton->setSymbol( QgsTiledSceneWireframeRenderer::createDefaultFillSymbol() );
  mLineSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  mLineSymbolButton->setSymbol( QgsTiledSceneWireframeRenderer::createDefaultLineSymbol() );

  if ( layer )
  {
    setFromRenderer( layer->renderer() );
  }

  connect( mFillSymbolButton, &QgsSymbolButton::changed, this, &QgsTiledSceneWireframeRendererWidget::emitWidgetChanged );
  connect( mLineSymbolButton, &QgsSymbolButton::changed, this, &QgsTiledSceneWireframeRendererWidget::emitWidgetChanged );
  connect( mCheckUseTextureColors, &QCheckBox::toggled, this, &QgsTiledSceneWireframeRendererWidget::emitWidgetChanged );
}

QgsTiledSceneRendererWidget *QgsTiledSceneWireframeRendererWidget::create( QgsTiledSceneLayer *layer, QgsStyle *style, QgsTiledSceneRenderer * )
{
  return new QgsTiledSceneWireframeRendererWidget( layer, style );
}

QgsTiledSceneRenderer *QgsTiledSceneWireframeRendererWidget::renderer()
{
  std::unique_ptr<QgsTiledSceneWireframeRenderer> renderer = std::make_unique<QgsTiledSceneWireframeRenderer>();
  renderer->setFillSymbol( mFillSymbolButton->clonedSymbol<QgsFillSymbol>() );
  renderer->setLineSymbol( mLineSymbolButton->clonedSymbol<QgsLineSymbol>() );
  renderer->setUseTextureColors( mCheckUseTextureColors->isChecked() );

  return renderer.release();
}

void QgsTiledSceneWireframeRendererWidget::emitWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit widgetChanged();
}

void QgsTiledSceneWireframeRendererWidget::setFromRenderer( const QgsTiledSceneRenderer *renderer )
{
  mBlockChangedSignal = true;
  if ( const QgsTiledSceneWireframeRenderer *wireframeRenderer = dynamic_cast<const QgsTiledSceneWireframeRenderer *>( renderer ) )
  {
    mFillSymbolButton->setSymbol( wireframeRenderer->fillSymbol()->clone() );
    mLineSymbolButton->setSymbol( wireframeRenderer->lineSymbol()->clone() );
    mCheckUseTextureColors->setChecked( wireframeRenderer->useTextureColors() );
  }

  mBlockChangedSignal = false;
}

///@endcond
