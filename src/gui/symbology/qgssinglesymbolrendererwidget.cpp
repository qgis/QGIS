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

#include "qgsdatadefinedsizelegend.h"
#include "qgsdatadefinedsizelegendwidget.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include "qgssymbolselectordialog.h"

#include <QMenu>


QgsRendererWidget *QgsSingleSymbolRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsSingleSymbolRendererWidget( layer, style, renderer );
}

QgsSingleSymbolRendererWidget::QgsSingleSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )

{
  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")

  if ( renderer )
  {
    mRenderer = QgsSingleSymbolRenderer::convertFromRenderer( renderer );
  }
  if ( !mRenderer )
  {
    QgsSymbol *symbol = QgsSymbol::defaultSymbol( mLayer->geometryType() );

    if ( symbol )
      mRenderer = new QgsSingleSymbolRenderer( symbol );
  }

  // load symbol from it
  if ( mRenderer )
    mSingleSymbol = mRenderer->symbol()->clone();

  // setup ui
  mSelector = new QgsSymbolSelectorWidget( mSingleSymbol, mStyle, mLayer, nullptr );
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, &QgsSingleSymbolRendererWidget::changeSingleSymbol );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );
  connect( this, &QgsRendererWidget::symbolLevelsChanged, [ = ]()
  {
    delete mSingleSymbol;
    mSingleSymbol = mRenderer->symbol()->clone();
    mSelector->loadSymbol( mSingleSymbol );
  } );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );

  // advanced actions - data defined rendering
  QMenu *advMenu = mSelector->advancedMenu();

  QAction *actionLevels = advMenu->addAction( tr( "Symbol Levels…" ) );
  connect( actionLevels, &QAction::triggered, this, &QgsSingleSymbolRendererWidget::showSymbolLevels );
  if ( mSingleSymbol && mSingleSymbol->type() == QgsSymbol::Marker )
  {
    QAction *actionDdsLegend = advMenu->addAction( tr( "Data-defined Size Legend…" ) );
    // only from Qt 5.6 there is convenience addAction() with new style connection
    connect( actionDdsLegend, &QAction::triggered, this, &QgsSingleSymbolRendererWidget::dataDefinedSizeLegend );
  }
}

QgsSingleSymbolRendererWidget::~QgsSingleSymbolRendererWidget()
{
  delete mSingleSymbol;

  delete mRenderer;

  delete mSelector;
}


QgsFeatureRenderer *QgsSingleSymbolRendererWidget::renderer()
{
  return mRenderer;
}

void QgsSingleSymbolRendererWidget::setContext( const QgsSymbolWidgetContext &context )
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

void QgsSingleSymbolRendererWidget::dataDefinedSizeLegend()
{
  QgsMarkerSymbol *s = static_cast<QgsMarkerSymbol *>( mSingleSymbol ); // this should be only enabled for marker symbols
  QgsDataDefinedSizeLegendWidget *panel = createDataDefinedSizeLegendWidget( s, mRenderer->dataDefinedSizeLegend() );
  if ( panel )
  {
    connect( panel, &QgsPanelWidget::widgetChanged, this, [ = ]
    {
      mRenderer->setDataDefinedSizeLegend( panel->dataDefinedSizeLegend() );
      emit widgetChanged();
    } );
    openPanel( panel );  // takes ownership of the panel
  }
}
