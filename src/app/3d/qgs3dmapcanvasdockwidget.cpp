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
#include "qgs3dmapcanvaswidget.h"
#include "qgsdockwidget.h"

#include <QWidget>

Qgs3DMapCanvasDockWidget::Qgs3DMapCanvasDockWidget( QWidget *parent )
  : QDialog( parent )
{
  mCanvasWidget = new Qgs3DMapCanvasWidget( this );
  mDock = new QgsDockWidget( this );
  mDock->setWidget( mCanvasWidget );

  connect( mCanvasWidget, &Qgs3DMapCanvasWidget::toggleDockMode, this, &Qgs3DMapCanvasDockWidget::toggleDockMode );
}

void Qgs3DMapCanvasDockWidget::setMapSettings( Qgs3DMapSettings *map )
{
  mCanvasWidget->setMapSettings( map );
}

void Qgs3DMapCanvasDockWidget::setMainCanvas( QgsMapCanvas *canvas )
{
  mCanvasWidget->setMainCanvas( canvas );
}

Qgs3DMapCanvas *Qgs3DMapCanvasDockWidget::mapCanvas3D()
{
  return mCanvasWidget->mapCanvas3D();
}

Qgs3DAnimationWidget *Qgs3DMapCanvasDockWidget::animationWidget()
{
  return mCanvasWidget->animationWidget();
}

Qgs3DMapToolMeasureLine *Qgs3DMapCanvasDockWidget::measurementLineTool()
{
  return mCanvasWidget->measurementLineTool();
}

void Qgs3DMapCanvasDockWidget::toggleDockMode( bool docked )
{
  // TODO: handle window/dock widget sizes and window titles
  if ( docked )
  {
    // going from window -> dock
    if ( mDialog )
    {
      mDialog->setLayout( nullptr );
      mDialog->deleteLater();
      mDialog = nullptr;
    }

    mDock = new QgsDockWidget( QString(), QgisApp::instance() );
    mDock->setWidget( mCanvasWidget );
    connect( this, &QObject::destroyed, mDock, &QWidget::close );
    QgisApp::instance()->addTabifiedDockWidget( Qt::BottomDockWidgetArea, mDock, QStringList(), true );
  }
  else
  {
    // going from dock -> window
    mDialog = new QDialog( QgisApp::instance(), Qt::Window );
    mDialog->setAttribute( Qt::WA_DeleteOnClose );

    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->addWidget( mCanvasWidget );
    mDialog->setLayout( vl );

    if ( mDock )
    {
      mDock->setWidget( nullptr );
      disconnect( this, &QObject::destroyed, mDock, &QWidget::close );
      mDock->deleteLater();
      mDock = nullptr;
    }

    mDialog->show();
  }
}
