/***************************************************************************
  qgsabstract3dengine.cpp
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

#include "qgsabstract3dengine.h"

#include "qgsshadowrenderingframegraph.h"

#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/QRenderSettings>

QgsAbstract3DEngine::QgsAbstract3DEngine( QObject *parent )
  : QObject( parent )
{

}

void QgsAbstract3DEngine::requestCaptureImage()
{
  Qt3DRender::QRenderCaptureReply *captureReply;
  captureReply = mFrameGraph->renderCapture()->requestCapture();

  connect( captureReply, &Qt3DRender::QRenderCaptureReply::completed, this, [ = ]
  {
    emit imageCaptured( captureReply->image() );
    captureReply->deleteLater();
  } );
}

void QgsAbstract3DEngine::requestDepthBufferCapture()
{
  Qt3DRender::QRenderCaptureReply *captureReply;
  captureReply = mFrameGraph->depthRenderCapture()->requestCapture();

  connect( captureReply, &Qt3DRender::QRenderCaptureReply::completed, this, [ = ]
  {
    emit depthBufferCaptured( captureReply->image() );
    captureReply->deleteLater();
  } );
}

void QgsAbstract3DEngine::setRenderCaptureEnabled( bool enabled )
{
  mFrameGraph->setRenderCaptureEnabled( enabled );
}

bool QgsAbstract3DEngine::renderCaptureEnabled() const
{
  return mFrameGraph->renderCaptureEnabled();
}
