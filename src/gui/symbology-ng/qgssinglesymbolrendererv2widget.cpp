/***************************************************************************
    qgssinglesymbolrendererv2widget.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssinglesymbolrendererv2widget.h"

#include "qgssinglesymbolrendererv2.h"
#include "qgssymbolv2.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include "qgssymbolv2selectordialog.h"

#include <QMenu>

QgsRendererV2Widget* QgsSingleSymbolRendererV2Widget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsSingleSymbolRendererV2Widget( layer, style, renderer );
}

QgsSingleSymbolRendererV2Widget::QgsSingleSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
{
  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( !renderer || renderer->type() != "singleSymbol" )
  {
    // we're not going to use it - so let's delete the renderer
    delete renderer;

    // some default options
    QgsSymbolV2* symbol = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );

    mRenderer = new QgsSingleSymbolRendererV2( symbol );
  }
  else
  {
    mRenderer = static_cast<QgsSingleSymbolRendererV2*>( renderer );
  }

  // load symbol from it
  mSingleSymbol = mRenderer->symbol()->clone();

  // setup ui
  mSelector = new QgsSymbolV2SelectorDialog( mSingleSymbol, mStyle, mLayer, NULL, true );
  connect( mSelector, SIGNAL( symbolModified() ), this, SLOT( changeSingleSymbol() ) );

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget( mSelector );
  setLayout( layout );

  // advanced actions - data defined rendering
  QMenu* advMenu = mSelector->advancedMenu();

  advMenu->addAction( tr( "Symbol levels..." ), this, SLOT( showSymbolLevels() ) );

  mDataDefinedMenus = new QgsRendererV2DataDefinedMenus( advMenu, mLayer->pendingFields(),
    mRenderer->rotationField(), mRenderer->sizeScaleField(), mRenderer->scaleMethod() );
  connect( mDataDefinedMenus, SIGNAL( rotationFieldChanged( QString ) ), this, SLOT( rotationFieldChanged( QString ) ) );
  connect( mDataDefinedMenus, SIGNAL( sizeScaleFieldChanged( QString ) ), this, SLOT( sizeScaleFieldChanged( QString ) ) );
  connect( mDataDefinedMenus, SIGNAL( scaleMethodChanged( QgsSymbolV2::ScaleMethod ) ), this, SLOT( scaleMethodChanged( QgsSymbolV2::ScaleMethod ) ) );
}

QgsSingleSymbolRendererV2Widget::~QgsSingleSymbolRendererV2Widget()
{
  delete mSingleSymbol;

  delete mRenderer;

  delete mSelector;
}


QgsFeatureRendererV2* QgsSingleSymbolRendererV2Widget::renderer()
{
  return mRenderer;
}

void QgsSingleSymbolRendererV2Widget::changeSingleSymbol()
{
  // update symbol from the GUI
  mRenderer->setSymbol( mSingleSymbol->clone() );
}

void QgsSingleSymbolRendererV2Widget::rotationFieldChanged( QString fldName )
{
  mRenderer->setRotationField( fldName );
}

void QgsSingleSymbolRendererV2Widget::sizeScaleFieldChanged( QString fldName )
{
  mRenderer->setSizeScaleField( fldName );
}

void QgsSingleSymbolRendererV2Widget::scaleMethodChanged( QgsSymbolV2::ScaleMethod scaleMethod )
{
  mRenderer->setScaleMethod( scaleMethod );
}

void QgsSingleSymbolRendererV2Widget::showSymbolLevels()
{
  showSymbolLevelsDialog( mRenderer );
}
