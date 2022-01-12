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
  : QWidget( parent )
{
  mCanvasWidget = new Qgs3DMapCanvasWidget;

  mDock = new QgsDockWidget( QgisApp::instance() );
  mDock->setWidget( nullptr );
  mDock->setAllowedAreas( Qt::AllDockWidgetAreas );

  mDock->setVisible( false );

  mDialog = new QDialog( QgisApp::instance(), Qt::Window );
  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  mDialog->setLayout( vl );
  mDialog->hide();

  connect( mDock, &QgsDockWidget::closed, this, &Qgs3DMapCanvasDockWidget::closed );
  connect( mDialog, &QDialog::finished, [ = ]()
  {
    emit this->closed();
  } );

  connect( mCanvasWidget, &Qgs3DMapCanvasWidget::toggleDockMode, this, &Qgs3DMapCanvasDockWidget::toggleDockMode );

  mIsDocked = false;
  switchToDockMode();
}

Qgs3DMapCanvasDockWidget::~Qgs3DMapCanvasDockWidget()
{
  delete mCanvasWidget;
  delete mDock;
  delete mDialog;
}

Qgs3DMapCanvasWidget *Qgs3DMapCanvasDockWidget::widget()
{
  return mCanvasWidget;
}

bool Qgs3DMapCanvasDockWidget::isDocked()
{
  return mIsDocked;
}

void Qgs3DMapCanvasDockWidget::toggleDockMode( bool docked )
{
  // TODO: handle window/dock widget sizes and window titles
  if ( docked )
  {
    // going from window -> dock
    switchToDockMode();
  }
  else
  {
    // going from dock -> window
    switchToWindowMode();
  }
}


void Qgs3DMapCanvasDockWidget::switchToWindowMode()
{
  if ( !mIsDocked )
    return;

  mIsDocked = false;

  mDialog->layout()->addWidget( mCanvasWidget );
  mDock->setWidget( nullptr );

  mDialog->show();
  mDock->setVisible( false );

  // TODO: apply resizing in a better way
  mDialog->resize( mDialog->size() + QSize( 1, 1 ) );
  mDialog->resize( mDialog->size() - QSize( 1, 1 ) );

  mCanvasWidget->setDocked( false );
}

void Qgs3DMapCanvasDockWidget::switchToDockMode()
{
  if ( mIsDocked )
    return;

  mIsDocked = true;

  mDialog->layout()->removeWidget( mCanvasWidget );
  mDock->setWidget( mCanvasWidget );

  mDialog->hide();
  mDock->setVisible( true );

  // TODO: apply resizing in a better way
  mDock->resize( mDock->size() + QSize( 1, 1 ) );
  mDock->resize( mDock->size() - QSize( 1, 1 ) );

  mCanvasWidget->setDocked( true );
}

