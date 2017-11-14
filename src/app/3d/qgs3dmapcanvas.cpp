/***************************************************************************
  qgs3dmapcanvas.cpp
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

#include "qgs3dmapcanvas.h"

#include <QBoxLayout>
#include <Qt3DExtras/Qt3DWindow>

#include "qgscameracontroller.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmapscene.h"


Qgs3DMapCanvas::Qgs3DMapCanvas( QWidget *parent )
  : QWidget( parent )
{
  mWindow3D = new Qt3DExtras::Qt3DWindow;
  mContainer = QWidget::createWindowContainer( mWindow3D );

  QHBoxLayout *hLayout = new QHBoxLayout( this );
  hLayout->setMargin( 0 );
  hLayout->addWidget( mContainer, 1 );

  mWindow3D->setCursor( Qt::OpenHandCursor );
}

Qgs3DMapCanvas::~Qgs3DMapCanvas()
{
  delete mMap;
}

void Qgs3DMapCanvas::resizeEvent( QResizeEvent *ev )
{
  QWidget::resizeEvent( ev );

  if ( !mScene )
    return;

  QRect viewportRect( QPoint( 0, 0 ), size() );
  mScene->cameraController()->setViewport( viewportRect );
}

void Qgs3DMapCanvas::setMap( Qgs3DMapSettings *map )
{
  // TODO: eventually we want to get rid of this
  Q_ASSERT( !mMap );
  Q_ASSERT( !mScene );

  QRect viewportRect( QPoint( 0, 0 ), size() );
  Qgs3DMapScene *newScene = new Qgs3DMapScene( *map, mWindow3D->defaultFrameGraph(), mWindow3D->renderSettings(), mWindow3D->camera(), viewportRect );

  mWindow3D->setRootEntity( newScene );

  if ( mScene )
    mScene->deleteLater();
  mScene = newScene;

  delete mMap;
  mMap = map;

  resetView();
}

void Qgs3DMapCanvas::resetView()
{
  mScene->viewZoomFull();
}
