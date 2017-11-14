/***************************************************************************
  qgs3dmapcanvasdockwidget.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmapcanvasdockwidget.h"

#include "qgisapp.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapconfigwidget.h"
#include "qgsmapcanvas.h"

#include "qgs3dmapsettings.h"

#include <QBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QToolBar>

Qgs3DMapCanvasDockWidget::Qgs3DMapCanvasDockWidget( QWidget *parent )
  : QgsDockWidget( parent )
{
  setAttribute( Qt::WA_DeleteOnClose );  // removes the dock widget from main window when

  QWidget *contentsWidget = new QWidget( this );

  QToolBar *toolBar = new QToolBar( contentsWidget );
  toolBar->setIconSize( QgisApp::instance()->iconSize( true ) );
  toolBar->addAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionZoomFullExtent.svg" ) ), "Reset view", this, &Qgs3DMapCanvasDockWidget::resetView );
  toolBar->addAction( QgsApplication::getThemeIcon( QStringLiteral( "mIconProperties.svg" ) ), "Configure", this, &Qgs3DMapCanvasDockWidget::configure );

  mCanvas = new Qgs3DMapCanvas( contentsWidget );
  mCanvas->setMinimumSize( QSize( 200, 200 ) );

  QVBoxLayout *layout = new QVBoxLayout( contentsWidget );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  layout->addWidget( toolBar );
  layout->addWidget( mCanvas, 1 );

  setWidget( contentsWidget );
}

void Qgs3DMapCanvasDockWidget::setMapSettings( Qgs3DMapSettings *map )
{
  mCanvas->setMap( map );
}

void Qgs3DMapCanvasDockWidget::setMainCanvas( QgsMapCanvas *canvas )
{
  mMainCanvas = canvas;

  connect( mMainCanvas, &QgsMapCanvas::layersChanged, this, &Qgs3DMapCanvasDockWidget::onMainCanvasLayersChanged );
  connect( mMainCanvas, &QgsMapCanvas::canvasColorChanged, this, &Qgs3DMapCanvasDockWidget::onMainCanvasColorChanged );
}

void Qgs3DMapCanvasDockWidget::resetView()
{
  mCanvas->resetView();
}

void Qgs3DMapCanvasDockWidget::configure()
{
  QDialog dlg;
  Qgs3DMapConfigWidget *w = new Qgs3DMapConfigWidget( mCanvas->map(), mMainCanvas, &dlg );
  QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg );
  connect( buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
  connect( buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );

  QVBoxLayout *layout = new QVBoxLayout( &dlg );
  layout->addWidget( w, 1 );
  layout->addWidget( buttons );
  if ( !dlg.exec() )
    return;

  // update map
  w->apply();
}

void Qgs3DMapCanvasDockWidget::onMainCanvasLayersChanged()
{
  mCanvas->map()->setLayers( mMainCanvas->layers() );
}

void Qgs3DMapCanvasDockWidget::onMainCanvasColorChanged()
{
  mCanvas->map()->setBackgroundColor( mMainCanvas->canvasColor() );
}
