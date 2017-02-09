/***************************************************************************
    qgssinglesymbolrendererwidget.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssinglesymbolrendererwidget.h"

#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include "qgssymbolselectordialog.h"

#include <QMenu>

QgsRendererWidget* QgsSingleSymbolRendererWidget::create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer )
{
  return new QgsSingleSymbolRendererWidget( layer, style, renderer );
}

QgsSingleSymbolRendererWidget::QgsSingleSymbolRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer )
    : QgsRendererWidget( layer, style )
    , mRenderer( nullptr )
{
  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")

  if ( renderer )
  {
    mRenderer = QgsSingleSymbolRenderer::convertFromRenderer( renderer );
  }
  if ( !mRenderer )
  {
    QgsSymbol* symbol = QgsSymbol::defaultSymbol( mLayer->geometryType() );

    mRenderer = new QgsSingleSymbolRenderer( symbol );
  }

  // load symbol from it
  mSingleSymbol = mRenderer->symbol()->clone();

  // setup ui
  mSelector = new QgsSymbolSelectorWidget( mSingleSymbol, mStyle, mLayer, nullptr );
  connect( mSelector, SIGNAL( symbolModified() ), this, SLOT( changeSingleSymbol() ) );
  connect( mSelector, SIGNAL( showPanel( QgsPanelWidget* ) ), this, SLOT( openPanel( QgsPanelWidget* ) ) );

  QVBoxLayout* layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );

  // advanced actions - data defined rendering
  QMenu* advMenu = mSelector->advancedMenu();

  advMenu->addAction( tr( "Symbol levels..." ), this, SLOT( showSymbolLevels() ) );
}

QgsSingleSymbolRendererWidget::~QgsSingleSymbolRendererWidget()
{
  delete mSingleSymbol;

  delete mRenderer;

  delete mSelector;
}


QgsFeatureRenderer* QgsSingleSymbolRendererWidget::renderer()
{
  return mRenderer;
}

void QgsSingleSymbolRendererWidget::setContext( const QgsSymbolWidgetContext& context )
{
  QgsRendererWidget::setContext( context );
  if ( mSelector )
    mSelector->setContext( context );
}

void QgsSingleSymbolRendererWidget::setDockMode( bool dockMode )
{
  QgsRendererWidget::setDockMode( dockMode );
  if ( mSelector )
    mSelector->setDockMode( dockMode );
}

void QgsSingleSymbolRendererWidget::changeSingleSymbol()
{
  // update symbol from the GUI
  mRenderer->setSymbol( mSingleSymbol->clone() );
  emit widgetChanged();
}

void QgsSingleSymbolRendererWidget::showSymbolLevels()
{
  showSymbolLevelsDialog( mRenderer );
}
