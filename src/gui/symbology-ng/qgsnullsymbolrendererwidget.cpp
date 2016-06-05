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

#include "qgsnullsymbolrenderer.h"
#include "qgssymbolv2.h"
#include <QLayout>
#include <QLabel>

QgsRendererV2Widget* QgsNullSymbolRendererWidget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsNullSymbolRendererWidget( layer, style, renderer );
}

QgsNullSymbolRendererWidget::QgsNullSymbolRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
    , mRenderer( nullptr )
{
  if ( renderer )
  {
    mRenderer = QgsNullSymbolRenderer::convertFromRenderer( renderer );
  }
  if ( !mRenderer )
  {
    mRenderer = new QgsNullSymbolRenderer();
  }

  QGridLayout* layout = new QGridLayout( this );
  QLabel* label = new QLabel( tr( "No symbols will be rendered for features in this layer." ) );
  layout->addWidget( label );
}

QgsNullSymbolRendererWidget::~QgsNullSymbolRendererWidget()
{
  delete mRenderer;
}

QgsFeatureRendererV2* QgsNullSymbolRendererWidget::renderer()
{
  return mRenderer;
}
