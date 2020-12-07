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
    mRenderer = QgsNullSymbolRenderer::convertFromRenderer( renderer );
  }
  if ( !mRenderer )
  {
    mRenderer = new QgsNullSymbolRenderer();
  }

  QGridLayout *layout = new QGridLayout( this );
  QLabel *label = new QLabel( tr( "No symbols will be rendered for features in this layer." ) );
  layout->addWidget( label );
}

QgsNullSymbolRendererWidget::~QgsNullSymbolRendererWidget()
{
  delete mRenderer;
}

QgsFeatureRenderer *QgsNullSymbolRendererWidget::renderer()
{
  return mRenderer;
}
