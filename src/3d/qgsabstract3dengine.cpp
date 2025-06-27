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
#include "moc_qgsabstract3dengine.cpp"

#include "qgsframegraph.h"
#include "qgslogger.h"

#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/QRenderSettings>

QgsAbstract3DEngine::QgsAbstract3DEngine( QObject *parent )
  : QObject( parent )
{
}

void QgsAbstract3DEngine::requestCaptureImage()
{
  Qt3DRender::QRenderCaptureReply *captureReply;
  mFrameGraph->setRenderCaptureEnabled( true );
  captureReply = mFrameGraph->renderCapture()->requestCapture();

  connect( captureReply, &Qt3DRender::QRenderCaptureReply::completed, this, [this, captureReply] {
    emit imageCaptured( captureReply->image() );
    captureReply->deleteLater();
    mFrameGraph->setRenderCaptureEnabled( false );
  } );
}

void QgsAbstract3DEngine::requestDepthBufferCapture()
{
  Qt3DRender::QRenderCaptureReply *captureReply;
  captureReply = mFrameGraph->depthRenderCapture()->requestCapture();

  connect( captureReply, &Qt3DRender::QRenderCaptureReply::completed, this, [this, captureReply] {
    emit depthBufferCaptured( captureReply->image() );
    captureReply->deleteLater();
  } );
}

void QgsAbstract3DEngine::dumpFrameGraphToConsole() const
{
  if ( mFrameGraph )
  {
    QgsDebugMsgLevel( QString( "FrameGraph:\n%1" ).arg( mFrameGraph->dumpFrameGraph() ), 1 );
    QgsDebugMsgLevel( QString( "SceneGraph:\n%1" ).arg( mFrameGraph->dumpSceneGraph() ), 1 );
  }
}

QString QgsAbstract3DEngine::dumpFrameGraph() const
{
  if ( mFrameGraph )
  {
    return mFrameGraph->dumpFrameGraph();
  }
  return QString();
}

QString QgsAbstract3DEngine::dumpSceneGraph() const
{
  if ( mFrameGraph )
  {
    return mFrameGraph->dumpSceneGraph();
  }
  return QString();
}
