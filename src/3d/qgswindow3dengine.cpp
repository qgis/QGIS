/***************************************************************************
  qgswindow3dengine.cpp
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswindow3dengine.h"

#include <Qt3DRender/QRenderCapture>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>


QgsWindow3DEngine::QgsWindow3DEngine()
{
  mWindow3D = new Qt3DExtras::Qt3DWindow;

  mCapture = new Qt3DRender::QRenderCapture;
  mWindow3D->activeFrameGraph()->setParent( mCapture );
  mWindow3D->setActiveFrameGraph( mCapture );
}

QWindow *QgsWindow3DEngine::window()
{
  return mWindow3D;
}

void QgsWindow3DEngine::requestCaptureImage()
{
  Qt3DRender::QRenderCaptureReply *captureReply;
  captureReply = mCapture->requestCapture();
  connect( captureReply, &Qt3DRender::QRenderCaptureReply::completed, this, [ = ]
  {
    emit imageCaptured( captureReply->image() );
    captureReply->deleteLater();
  } );
}

void QgsWindow3DEngine::setClearColor( const QColor &color )
{
  mWindow3D->defaultFrameGraph()->setClearColor( color );
}

void QgsWindow3DEngine::setFrustumCullingEnabled( bool enabled )
{
  mWindow3D->defaultFrameGraph()->setFrustumCullingEnabled( enabled );
}

void QgsWindow3DEngine::setRootEntity( Qt3DCore::QEntity *root )
{
  mWindow3D->setRootEntity( root );
}

Qt3DRender::QRenderSettings *QgsWindow3DEngine::renderSettings()
{
  return mWindow3D->renderSettings();
}

Qt3DRender::QCamera *QgsWindow3DEngine::camera()
{
  return mWindow3D->camera();
}

QSize QgsWindow3DEngine::size() const
{
  return mWindow3D->size();
}
