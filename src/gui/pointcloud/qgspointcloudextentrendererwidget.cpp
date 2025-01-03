/***************************************************************************
                         qgspointcloudextentrendererwidget.cpp
    ---------------------
    begin                : December 2020
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

#include "qgspointcloudextentrendererwidget.h"
#include "moc_qgspointcloudextentrendererwidget.cpp"
#include "qgscontrastenhancement.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudextentrenderer.h"
#include "qgsdoublevalidator.h"
#include "qgsfillsymbol.h"

///@cond PRIVATE

QgsPointCloudExtentRendererWidget::QgsPointCloudExtentRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style )
  : QgsPointCloudRendererWidget( layer, style )
{
  setupUi( this );

  mSymbolButton->setSymbolType( Qgis::SymbolType::Fill );

  if ( layer )
  {
    setFromRenderer( layer->renderer() );
  }
  else
  {
    mSymbolButton->setSymbol( QgsPointCloudExtentRenderer::defaultFillSymbol() );
  }

  connect( mSymbolButton, &QgsSymbolButton::changed, this, &QgsPointCloudExtentRendererWidget::emitWidgetChanged );
}

QgsPointCloudRendererWidget *QgsPointCloudExtentRendererWidget::create( QgsPointCloudLayer *layer, QgsStyle *style, QgsPointCloudRenderer * )
{
  return new QgsPointCloudExtentRendererWidget( layer, style );
}

QgsPointCloudRenderer *QgsPointCloudExtentRendererWidget::renderer()
{
  if ( !mLayer )
  {
    return nullptr;
  }

  std::unique_ptr<QgsPointCloudExtentRenderer> renderer = std::make_unique<QgsPointCloudExtentRenderer>();
  renderer->setFillSymbol( mSymbolButton->clonedSymbol<QgsFillSymbol>() );
  return renderer.release();
}

void QgsPointCloudExtentRendererWidget::emitWidgetChanged()
{
  if ( !mBlockChangedSignal )
    emit widgetChanged();
}

void QgsPointCloudExtentRendererWidget::setFromRenderer( const QgsPointCloudRenderer *r )
{
  mBlockChangedSignal = true;
  if ( const QgsPointCloudExtentRenderer *mbcr = dynamic_cast<const QgsPointCloudExtentRenderer *>( r ) )
  {
    mSymbolButton->setSymbol( mbcr->fillSymbol()->clone() );
  }
  else
  {
    mSymbolButton->setSymbol( QgsPointCloudExtentRenderer::defaultFillSymbol() );
  }
  mBlockChangedSignal = false;
}

///@endcond
