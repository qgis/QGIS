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
#include "moc_qgssinglesymbolrendererwidget.cpp"

#include "qgsdatadefinedsizelegend.h"
#include "qgsdatadefinedsizelegendwidget.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include "qgssymbolselectordialog.h"
#include "qgsmarkersymbol.h"

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
    mRenderer.reset( QgsSingleSymbolRenderer::convertFromRenderer( renderer ) );
  }
  if ( !mRenderer )
  {
    QgsSymbol *symbol = QgsSymbol::defaultSymbol( mLayer->geometryType() );

    if ( symbol )
      mRenderer = std::make_unique<QgsSingleSymbolRenderer>( symbol );

    if ( renderer )
      renderer->copyRendererData( mRenderer.get() );
  }

  // load symbol from it
  if ( mRenderer )
    mSingleSymbol.reset( mRenderer->symbol()->clone() );

  // setup ui
  mSelector = new QgsSymbolSelectorWidget( mSingleSymbol.get(), mStyle, mLayer, nullptr );
  connect( mSelector, &QgsSymbolSelectorWidget::symbolModified, this, &QgsSingleSymbolRendererWidget::changeSingleSymbol );
  connect( mSelector, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mSelector );

  // advanced actions - data defined rendering
  QMenu *advMenu = mSelector->advancedMenu();

  mActionLevels = advMenu->addAction( tr( "Symbol Levels…" ) );
  connect( mActionLevels, &QAction::triggered, this, &QgsSingleSymbolRendererWidget::showSymbolLevels );
  if ( mSingleSymbol && mSingleSymbol->type() == Qgis::SymbolType::Marker )
  {
    QAction *actionDdsLegend = advMenu->addAction( tr( "Data-defined Size Legend…" ) );
    connect( actionDdsLegend, &QAction::triggered, this, &QgsSingleSymbolRendererWidget::dataDefinedSizeLegend );
  }
}

QgsSingleSymbolRendererWidget::~QgsSingleSymbolRendererWidget()
{
  mSingleSymbol.reset();
  mRenderer.reset();

  delete mSelector;
}

QgsFeatureRenderer *QgsSingleSymbolRendererWidget::renderer()
{
  return mRenderer.get();
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

void QgsSingleSymbolRendererWidget::disableSymbolLevels()
{
  delete mActionLevels;
  mActionLevels = nullptr;
}

void QgsSingleSymbolRendererWidget::setSymbolLevels( const QList<QgsLegendSymbolItem> &levels, bool enabled )
{
  mSingleSymbol.reset( levels.at( 0 ).symbol()->clone() );
  if ( !enabled )
  {
    // remove the renderer symbol levels flag (if present), as we don't symbol levels automatically re-enabling when other changes
    // are made to the symbol
    mSingleSymbol->setFlags( mSingleSymbol->flags() & ~Qgis::SymbolFlags( Qgis::SymbolFlag::RendererShouldUseSymbolLevels ) );
  }
  mRenderer->setSymbol( mSingleSymbol->clone() );
  mRenderer->setUsingSymbolLevels( enabled );
  mSelector->loadSymbol( mSingleSymbol.get() );
  emit widgetChanged();
}

void QgsSingleSymbolRendererWidget::changeSingleSymbol()
{
  // update symbol from the GUI
  mRenderer->setSymbol( mSingleSymbol->clone() );

  if ( mSingleSymbol->flags() & Qgis::SymbolFlag::RendererShouldUseSymbolLevels )
    mRenderer->setUsingSymbolLevels( true );

  emit widgetChanged();
}

void QgsSingleSymbolRendererWidget::showSymbolLevels()
{
  showSymbolLevelsDialog( mRenderer.get() );
}

void QgsSingleSymbolRendererWidget::dataDefinedSizeLegend()
{
  QgsMarkerSymbol *s = static_cast<QgsMarkerSymbol *>( mSingleSymbol.get() ); // this should be only enabled for marker symbols
  QgsDataDefinedSizeLegendWidget *panel = createDataDefinedSizeLegendWidget( s, mRenderer->dataDefinedSizeLegend() );
  if ( panel )
  {
    connect( panel, &QgsPanelWidget::widgetChanged, this, [=] {
      mRenderer->setDataDefinedSizeLegend( panel->dataDefinedSizeLegend() );
      emit widgetChanged();
    } );
    openPanel( panel ); // takes ownership of the panel
  }
}
