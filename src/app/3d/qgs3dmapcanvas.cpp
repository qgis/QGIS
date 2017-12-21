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
#include <Qt3DRender/QRenderCapture>

#include "qgscameracontroller.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmapscene.h"


Qgs3DMapCanvas::Qgs3DMapCanvas( QWidget *parent )
  : QWidget( parent )
{
  mWindow3D = new Qt3DExtras::Qt3DWindow;

  mCapture = new Qt3DRender::QRenderCapture;
  mWindow3D->activeFrameGraph()->setParent( mCapture );
  mWindow3D->setActiveFrameGraph( mCapture );

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

QgsCameraController *Qgs3DMapCanvas::cameraController()
{
  return mScene ? mScene->cameraController() : nullptr;
}

void Qgs3DMapCanvas::resetView()
{
  mScene->viewZoomFull();
}

void Qgs3DMapCanvas::setViewFromTop( const QgsPointXY &center, float distance, float rotation )
{
  float worldX = center.x() - mMap->origin().x();
  float worldY = center.y() - mMap->origin().y();
  mScene->cameraController()->setViewFromTop( worldX, -worldY, distance, rotation );
}

void Qgs3DMapCanvas::saveAsImage( const QString fileName, const QString fileFormat )
{
  if ( !fileName.isEmpty() )
  {
    Qt3DRender::QRenderCaptureReply *captureReply;
    captureReply = mCapture->requestCapture();
    connect( captureReply, &Qt3DRender::QRenderCaptureReply::completed, this, [ = ]
    {
      captureReply->image().save( fileName, fileFormat.toLocal8Bit().data() );
      emit savedAsImage( fileName );

      captureReply->deleteLater();
    } );
  }
}
