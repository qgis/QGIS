/***************************************************************************
    qgsnullsymbolrendererwidget.cpp
    ---------------------
    begin                : November 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsnullsymbolrendererwidget.h"
#include "moc_qgsnullsymbolrendererwidget.cpp"

#include "qgsnullsymbolrenderer.h"
#include "qgssymbol.h"
#include <QLayout>
#include <QLabel>

QgsRendererWidget *QgsNullSymbolRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsNullSymbolRendererWidget( layer, style, renderer );
}

QgsNullSymbolRendererWidget::QgsNullSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )
{
  if ( renderer )
  {
    mRenderer.reset( QgsNullSymbolRenderer::convertFromRenderer( renderer ) );
  }
  if ( !mRenderer )
  {
    mRenderer = std::make_unique<QgsNullSymbolRenderer>();
    if ( renderer )
      renderer->copyRendererData( mRenderer.get() );
  }

  QGridLayout *layout = new QGridLayout( this );
  QLabel *label = new QLabel( tr( "No symbols will be rendered for features in this layer." ) );
  layout->addWidget( label );
}

QgsNullSymbolRendererWidget::~QgsNullSymbolRendererWidget() = default;

QgsFeatureRenderer *QgsNullSymbolRendererWidget::renderer()
{
  return mRenderer.get();
}
